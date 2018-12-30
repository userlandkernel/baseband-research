/*
 * Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Description:   thread capability list
 */
#include <kernel/l4.h>
#include <kernel/tcb.h>
#include <kernel/clist.h>

#if defined(CONFIG_DEBUG)
/* The amount of time we should spin waiting for references to a
 * variable are finished with before assuming that a reference
 * has been leaked. */
#define REFERENCE_WAIT_SPIN_TIME  0x1000
#endif /* CONFIG_DEBUG */

//#define TRACE_CLIST(x...) printf(x)
#define TRACE_CLIST(x...)

DECLARE_KMEM_GROUP (kmem_root_clist);
clist_t *root_clist = NULL;
GLOBAL_DEF(clist_t *current_clist = NULL)


DEFINE_READ_WRITE_LOCK(cap_reference_t::cap_reference_lock);

void init_root_clist(void)
{
    root_clist =
        (clist_t*) kmem.alloc(kmem_root_clist,
                              (((sizeof(clist_t) +
                                 get_init_data()->max_root_caps * sizeof(cap_t)) / KMEM_CHUNKSIZE) + 1) * KMEM_CHUNKSIZE,
                              true);
    
    cap_reference_t::cap_reference_lock.init();

    root_clist->init(1UL << CONFIG_ROOT_CAP_BITS);

    get_clist_list()->add_clist(clistid(0), root_clist);

    get_kernel_space()->init_security(root_clist);
    GLOBAL(current_clist) = root_clist;
}

tcb_t* clist_t::lookup_thread_cap_locked(threadid_t tid, bool write)
{
    tcb_t* tcb;
    TRACE_CLIST("%s: tid=0x%lx\n", __func__, tid.get_raw());

    if (EXPECT_FALSE(tid.is_nilthread())) {
        return NULL;
    }

    /* Check cap-id range, 0 is a valid cap-id */
    if (EXPECT_FALSE(tid.get_threadno() > max_id))
    {
        TRACE_CLIST(" - invalid tid: 0x%lx\n", tid.get_raw());
        return NULL;
    }

again:
    okl4_atomic_barrier_smp();
    /* Thread ID appears valid. Get the entry. */
    cap_t entry = this->entries[tid.get_threadno()];

    tcb = entry.get_tcb();
    if (!entry.is_thread_cap() || !tcb) {
        TRACE_CLIST(" - not thread cap: 0x%lx\n", tid.get_raw());
        return NULL;
    }

    if (EXPECT_FALSE(write)) {
        if (EXPECT_FALSE(!tcb->try_lock_write())) {
            okl4_atomic_barrier_smp();
            goto again;
        }
    } else {
        if (EXPECT_FALSE(!tcb->try_lock_read())) {
            okl4_atomic_barrier_smp();
            goto again;
        }
    }

    TRACE_CLIST(" - return: 0x%lx\n", tcb);
    return tcb;
}

tcb_t* clist_t::lookup_thread_cap_unlocked(threadid_t tid)
{
    tcb_t* tcb;
    TRACE_CLIST("%s: tid=0x%lx\n", __func__, tid.get_raw());

    if (EXPECT_FALSE(tid.is_nilthread())) {
        return NULL;
    }

    /* Check cap-id range, 0 is a valid cap-id */
    if (EXPECT_FALSE(tid.get_threadno() > max_id))
    {
        TRACE_CLIST(" - invalid tid: 0x%lx\n", tid.get_raw());
        return NULL;
    }

    /* Thread ID appears valid. Get the entry. */
    cap_t entry = this->entries[tid.get_threadno()];

    if (!entry.is_thread_cap()) {
        TRACE_CLIST(" - not thread cap: 0x%lx\n", tid.get_raw());
        return NULL;
    }

    tcb = entry.get_tcb();

    TRACE_CLIST(" - return: 0x%lx\n", tcb);
    return tcb;
}

tcb_t* clist_t::lookup_ipc_cap_locked(threadid_t tid)
{
    tcb_t* tcb;
    TRACE_CLIST("%s: tid=0x%lx\n", __func__, tid.get_raw());

    if (EXPECT_FALSE(tid.is_nilthread())) {
        return NULL;
    }

    /* Check cap-id range, 0 is a valid cap-id */
    if (EXPECT_FALSE((tid.get_threadno() > max_id)))
    {
        TRACE_CLIST(" - invalid tid: 0x%lx\n", tid.get_raw());
        return NULL;
    }

again:
    okl4_atomic_barrier_smp();
    /* Thread ID appears valid. Get the entry. */
    cap_t entry = this->entries[tid.get_threadno()];

    tcb = entry.get_tcb();
    if (!entry.can_ipc_cap() || !tcb) {
        TRACE_CLIST(" - not ipc cap: 0x%lx\n", tid.get_raw());
        return NULL;
    }

    if (EXPECT_FALSE(!tcb->try_lock_read())) {
        okl4_atomic_barrier_smp();
        goto again;
    }

    TRACE_CLIST(" - return: 0x%lx\n", tcb);
    return tcb;
}

cap_t* clist_t::lookup_cap(threadid_t tid)
{
    TRACE_CLIST("%s: tid=0x%lx\n", __func__, tid.get_raw());

    if (EXPECT_FALSE(tid.is_nilthread())) {
        return NULL;
    }

    /* Check cap-id range, 0 is a valid cap-id */
    if (EXPECT_FALSE((tid.get_threadno() > max_id)))
    {
        TRACE_CLIST(" - invalid tid: 0x%lx\n", tid.get_raw());
        return NULL;
    }

    /* Thread ID appears valid. Get the entry. */
    cap_t *entry = &this->entries[tid.get_threadno()];

    if (!entry->get_tcb()) {
        TRACE_CLIST(" - not valid cap: 0x%lx\n", tid.get_raw());
        return NULL;
    }

    TRACE_CLIST(" - return: 0x%lx\n", entry);
    return entry;
}

bool clist_t::add_thread_cap(threadid_t tid, tcb_t *tcb)
{
    ASSERT(DEBUG, tcb);
    ASSERT(DEBUG, !tid.is_nilthread());
    ASSERT(DEBUG, is_valid(tid));

    list_lock.lock();
    cap_t *entry = &this->entries[tid.get_threadno()];

    if (EXPECT_FALSE(entry->get_tcb() != NULL)) {
        list_lock.unlock();
        return false;
    }

    entry->set_thread_cap(tcb);
    list_lock.unlock();

    cap_reference_t::add_reference(tcb, entry);
    return true;
}

bool clist_t::add_ipc_cap(threadid_t tid, tcb_t *tcb)
{
    ASSERT(DEBUG, tcb);
    ASSERT(DEBUG, !tid.is_nilthread());
    ASSERT(DEBUG, is_valid(tid));

    list_lock.lock();
    cap_t *entry = &this->entries[tid.get_threadno()];

    if (EXPECT_FALSE(entry->get_tcb() != NULL)) {
        list_lock.unlock();
        return false;
    }

    entry->set_ipc_cap(tcb);
    list_lock.unlock();

    cap_reference_t::add_reference(tcb, entry);
    return true;
}

bool clist_t::remove_thread_cap(threadid_t tid)
{
    ASSERT(DEBUG, !tid.is_nilthread());
    ASSERT(DEBUG, is_valid(tid));

    list_lock.lock();
    /* Remove the entry. */
    cap_t entry = this->entries[tid.get_threadno()];
    tcb_t *tcb = entry.get_tcb();

    if (EXPECT_FALSE(!(entry.is_thread_cap() && tcb))) {
        list_lock.unlock();
        return false;
    }
    cap_reference_t::invalidate_reference(tcb);

    list_lock.unlock();
    return true;
}

bool clist_t::remove_ipc_cap(threadid_t tid)
{
    ASSERT(DEBUG, !tid.is_nilthread());
    ASSERT(DEBUG, is_valid(tid));

    list_lock.lock();
again:
    okl4_atomic_barrier_smp();
    /* Remove the entry. */
    cap_t entry = this->entries[tid.get_threadno()];
    tcb_t *tcb = entry.get_tcb();

    if (EXPECT_FALSE(!(entry.is_ipc_cap() && tcb))) {
        list_lock.unlock();
        return false;
    }
    if (EXPECT_FALSE(!tcb->try_lock_write())) {
        okl4_atomic_barrier_smp();
        goto again;
    }
    cap_reference_t::remove_reference(tcb, &this->entries[tid.get_threadno()]);
    tcb->unlock_write();

    list_lock.unlock();
    return true;
}

bool clist_t::is_empty(void)
{
    bool found = false;
    list_lock.lock();

    for (word_t i=0; i <= this->max_id; i++) {
        cap_t *entry = &this->entries[i];
        tcb_t * tcb = entry->get_tcb();

        if (tcb) {
            found = true;
            break;
        }
    }

    list_lock.unlock();
    return (found == false);
}

// XXX Locking needs to be redone


void cap_reference_t::add_reference(cap_t *master_cap, cap_t *cap)
{
    ASSERT(DEBUG, master_cap->get_tcb() == cap->get_tcb());
    cap_reference_lock.lock_write();
    cap->next = master_cap->next;
    master_cap->next = cap;
    cap_reference_lock.unlock_write();
}

void cap_reference_t::remove_reference(cap_t *master_cap, cap_t *cap)
{
    ASSERT(DEBUG, master_cap->get_tcb() == cap->get_tcb());
    cap_t *prev = master_cap;
    cap_reference_lock.lock_write();
    while (prev != NULL)
    {
        if (prev->next == cap) break;
        prev = prev->next;
    }
    if (prev != NULL)
    {
        prev->next = cap->next;
        cap->init();
    }
    cap_reference_lock.unlock_write();
}

void cap_reference_t::invalidate_reference(cap_t *master_cap)
{
    cap_t *temp = master_cap;
    cap_t *prev;
    cap_reference_lock.lock_write();
    while (temp != NULL)
    {
        temp->clear();
        prev = temp;
        temp = temp->next;
        prev->next = NULL;
    }
    cap_reference_lock.unlock_write();
}


void cap_reference_t::add_reference(tcb_t *tcb, cap_t *cap)
{
    if (tcb->master_cap == NULL)
    {
        tcb->master_cap = cap;
        ASSERT(DEBUG, cap->next == NULL);
    }
    else
    {
        add_reference(tcb->master_cap, cap);
    }
}

void cap_reference_t::remove_reference(tcb_t *tcb, cap_t *cap)
{
    remove_reference(tcb->master_cap, cap);
}

void cap_reference_t::invalidate_reference(tcb_t *tcb)
{
    invalidate_reference(tcb->master_cap);
    tcb->master_cap = NULL;
}


