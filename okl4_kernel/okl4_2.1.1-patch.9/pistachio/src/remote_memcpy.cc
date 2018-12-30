/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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
 * Description:   remote memory copy
 */

#include <l4.h>
#include <debug.h>
#include <config.h>
#include <tcb.h>
#include <thread.h>
#include <schedule.h>
#include <space.h>
#include <syscalls.h>
#include <remote_memcpy.h>
#include <arch/user_access.h>

#include <kdb/tracepoints.h>

DECLARE_TRACEPOINT(SYSCALL_MEMORY_COPY);


/*
 * This should be generic, but currently does not support all architectures.
 */
#if defined(CONFIG_REMOTE_MEMORY_COPY) && (defined(CONFIG_ARCH_IA32) || defined(CONFIG_ARCH_ARM))
/** @todo FIXME: SMT/SMP safety - glee. */
static word_t memcpy_buf[REMOTE_MEMCPY_BUFSIZE/sizeof(word_t)];

static void memcpy_frombounce(tcb_t *tcb, word_t *addr, word_t size);
static void memcpy_tobounce(tcb_t *tcb, word_t *addr, word_t size);


/*
 * memory_copy_recover(): recover from invalid user memory access
 */
static CONTINUATION_FUNCTION(memory_copy_recover)
{
    tcb_t *current;
    word_t fault, addr;
    continuation_t cont;

    /*
     * We have lost all context at this point, but we need to return
     * how much we have actually copied.  To do this before we actually
     * do the copy we flag the starting address.  Then from the pagefault
     * address we can work out how many bytes we copied.
     */
    current = get_current_tcb();
    fault = (word_t)TCB_SYSDATA_USER_ACCESS(current)->fault_address;
    addr = (word_t)TCB_SYSDATA_MEMCPY(current)->copy_start;

    /*
     * At this state we could still be on the other guy's page table.
     * Switch the pagetable back to our own, just in case.
     */
    current->get_space()->activate(current);

    cont = TCB_SYSDATA_MEMCPY(current)->memory_copy_cont;

    /* signal that a mapping was missing */
    current->set_error_code(ENO_MEM);
    /* bye bye ... */
    PROFILE_STOP(sys_remote_memcopy);
    return_memory_copy(0, fault - addr, cont);
}

/*
 * memcpy_frombounce(): make memcpy from bounce buffer
 */
static void memcpy_frombounce(tcb_t *tcb, word_t *addr, word_t size)
{
    tcb_t *current;
    word_t *dest = (word_t*)&memcpy_buf[0];
    word_t i;

    current = get_current_tcb();
    tcb->get_space()->activate(tcb);
    current->set_user_access(memory_copy_recover);

    /* First copy in word size */
    for (i = 0; i < size / sizeof(word_t); i++) {
        user_write_word(addr++, *dest++);
    }

    current->clear_user_access();
}

/*
 * memcpy_tobounce(): make memcpy to bounce buffer
 */
static void memcpy_tobounce(tcb_t *tcb, word_t *addr, word_t size)
{
    tcb_t *current;
    word_t *src = (word_t*)&memcpy_buf[0];
    word_t i;

    current = get_current_tcb();
    tcb->get_space()->activate(tcb);
    current->set_user_access(memory_copy_recover);

    /* First copy in word size */
    for (i = 0; i < size / sizeof(word_t); i++) {
        *src++ = user_read_word(addr++);
    }

    current->clear_user_access();
}

static CONTINUATION_FUNCTION(memory_copy_loop)
{
    tcb_t *current, *from_tcb, *to_tcb;
    addr_t src, dest, orig_src, orig_dest;
    word_t size, remote_size, orig_size;
    word_t copy_size;
    continuation_t cont;

    current = get_current_tcb();

    from_tcb =  TCB_SYSDATA_MEMCPY(current)->from_tcb;
    to_tcb =    TCB_SYSDATA_MEMCPY(current)->to_tcb;
    src =       TCB_SYSDATA_MEMCPY(current)->src;
    dest =      TCB_SYSDATA_MEMCPY(current)->dest;
    size =      TCB_SYSDATA_MEMCPY(current)->size;
    remote_size = TCB_SYSDATA_MEMCPY(current)->remote_size;
    orig_src =  TCB_SYSDATA_MEMCPY(current)->orig_src;
    orig_dest = TCB_SYSDATA_MEMCPY(current)->orig_dest;

    //printf("memcpy: from %p(%p, %lx), to %p(%p, %lx)\n", src, from_tcb, remote_size, dest, to_tcb, size);
    for (; remote_size > 0 && size > 0; /*null*/) {
        copy_size = size > remote_size ? remote_size : size;
        copy_size = REMOTE_MEMCPY_BUFSIZE > copy_size ? copy_size :
            REMOTE_MEMCPY_BUFSIZE;

        TCB_SYSDATA_MEMCPY(current)->copy_start = orig_src;
        memcpy_tobounce(from_tcb, (word_t *)src, copy_size);
        TCB_SYSDATA_MEMCPY(current)->copy_start = orig_dest;
        memcpy_frombounce(to_tcb, (word_t *)dest, copy_size);

        /*
         * Put these adjustments within the loop so we can turn 
         * the preemption point on easily.
         */
        src = addr_offset(src, copy_size);
        dest = addr_offset(dest, copy_size);
        remote_size -= copy_size;
        size -= copy_size;
        /*
         * After a full copy give a chance for interrupts to flow through.
         * Stash the necessary data somewhere and then run the 
         * continuation.
         */
        TCB_SYSDATA_MEMCPY(current)->src = src;
        TCB_SYSDATA_MEMCPY(current)->dest = dest;
        TCB_SYSDATA_MEMCPY(current)->size = size;
        TCB_SYSDATA_MEMCPY(current)->remote_size = remote_size;

        preempt_enable(memory_copy_loop);
        preempt_disable();
    }
    /* switch back to current address space */
    if (to_tcb != current) {
        current->get_space()->activate(current);
    }

    cont = TCB_SYSDATA_MEMCPY(current)->memory_copy_cont;

    orig_size = TCB_SYSDATA_MEMCPY(current)->orig_size;
    /*
     * If there is still size remaining it means the copy has overflowed
     * the remote buffer.
     */
    if (size) {
        current->set_error_code(ENO_PRIVILEGE);
        PROFILE_STOP(sys_remote_memcopy);
        return_memory_copy(0, orig_size - size, cont);
    }

    PROFILE_STOP(sys_remote_memcopy);
    return_memory_copy(1, orig_size, cont);
}

/*
 * sys_memory_copy() system call
 */
SYS_MEMORY_COPY(threadid_t remote, word_t local, word_t size,
                word_t direction)
{
    tcb_t *remote_tcb = NULL, *from_tcb, *to_tcb, *current;
    continuation_t cont = ASM_CONTINUATION;
    word_t remote_dir, remote_addr, remote_size, descidx;
    word_t src, dest;

    PROFILE_START(sys_remote_memcopy);

    TRACEPOINT (SYSCALL_MEMORY_COPY,
                printf ("SYSCALL_MEMORY_COPY: dest=%t, local addr=0x%lx "
                        "size=0x%lx dir=0x%lx\n",
                        TID (remote), local, size, direction));

    current = get_current_tcb();

    /*
     * Step 1: make sure that the cap is valid
     */
    if (EXPECT_TRUE(remote.is_threadhandle())) {

        remote_tcb = lookup_tcb_by_handle_locked(remote.get_raw());

        if (EXPECT_FALSE(remote_tcb == NULL)) {
            current->set_error_code(EINVALID_CAP);
            goto error_out;
        }

        if (EXPECT_FALSE(!remote_tcb->get_state().is_waiting() ||
                         !remote_tcb->is_partner_valid() ||
                         remote_tcb->get_partner() != current)) {
            remote_tcb->unlock_read();
            current->set_error_code(EINVALID_PARAM);
            goto error_out;
        }
    } else {
        current->set_error_code(EINVALID_CAP);
        goto error_out;
    }
    /*
     * XXX 
     *
     * the lock should be held for the whole system call but the 
     * preemption isn't very well-defined yet.  This needs to be fixed
     * for SMT with preemption.
     */
    remote_tcb->unlock_read();

    /*
     * Step 2: make sure that the direction specified in the memory
     * descriptor allows copying in the specified direction
     */
    if (EXPECT_TRUE(!remote_tcb->get_tag().get_memcpy())) {
        current->set_error_code(ENO_PRIVILEGE);
        goto error_out;
    }

    descidx = remote_tcb->get_tag().get_untyped() + 1/*tag*/;
    /* Check for message overflow */
    if (EXPECT_FALSE((descidx + 2) >= IPC_NUM_MR)) {
        current->set_error_code(EINVALID_PARAM);
        goto error_out;
    }

    remote_addr = remote_tcb->get_mr(descidx);
    remote_size = remote_tcb->get_mr(descidx + 1);
    remote_dir = remote_tcb->get_mr(descidx + 2);

    if (EXPECT_FALSE(remote_dir == direction)) {
        current->set_error_code(ENO_PRIVILEGE);
        goto error_out;
    }

#define WORD_MASK (sizeof(word_t) - 1)
    if ((local & WORD_MASK) || (size & WORD_MASK) ||
        (remote_addr & WORD_MASK) || (remote_size & WORD_MASK)) {
        current->set_error_code(EINVALID_PARAM);
        goto error_out;
    }
#undef WORD_MASK

    /* 
     * check the start and end of the address to see if within
     * user area (check both to catch overflow bugs
     */
    if (!current->get_space()->is_user_area((addr_t)local) ||
        !current->get_space()->is_user_area((addr_t)(local + size - 1)) ||
        !remote_tcb->get_space()->is_user_area((addr_t)remote_addr) ||
        !remote_tcb->get_space()->is_user_area((addr_t)(remote_addr + 
        remote_size - 1))) {
        current->set_error_code(EINVALID_PARAM);
        goto error_out;
    }

    /*
     * Step 3: enable the kernel user memory access feature and 
     * do the copy, while doing check to see if it will overflow
     * the bounds of the memory descriptor.  If it does then return
     * a short copy.
     */

    if (direction == (word_t) memcpy_dir_to) {
        src = remote_addr;
        dest = local;
        from_tcb = remote_tcb;
        to_tcb = current;
    } else if (direction == (word_t) memcpy_dir_from) {
        src = local;
        dest = remote_addr;
        from_tcb = current;
        to_tcb = remote_tcb;
    } else {
        current->set_error_code(EINVALID_PARAM);
        goto error_out;
    }

    /*
     * Stash the data into the tcb of the thread so that
     * we can pull it out in case we need to do a preemption.
     */
    current->sys_data.set_action(tcb_syscall_data_t::action_remote_memcpy);
    TCB_SYSDATA_MEMCPY(current)->to_tcb = to_tcb;
    TCB_SYSDATA_MEMCPY(current)->from_tcb = from_tcb;
    TCB_SYSDATA_MEMCPY(current)->src = (addr_t)src;
    TCB_SYSDATA_MEMCPY(current)->dest = (addr_t)dest;
    TCB_SYSDATA_MEMCPY(current)->orig_src = (addr_t)src;
    TCB_SYSDATA_MEMCPY(current)->orig_dest = (addr_t)dest;
    TCB_SYSDATA_MEMCPY(current)->size = size;
    TCB_SYSDATA_MEMCPY(current)->orig_size = size;
    TCB_SYSDATA_MEMCPY(current)->remote_size = remote_size;
    TCB_SYSDATA_MEMCPY(current)->memory_copy_cont = cont;

    ACTIVATE_CONTINUATION(memory_copy_loop);
    /*NOTREACHED*/

error_out:
    PROFILE_STOP(sys_remote_memcopy);
    return_memory_copy(0, 0, cont);
}
#else
/*
 * sys_memory_copy() system call
 */
SYS_MEMORY_COPY(threadid_t remote, word_t local, word_t size,
                word_t direction)
{
    continuation_t cont = ASM_CONTINUATION;

    PROFILE_START(sys_remote_memcopy);

    TRACEPOINT (SYSCALL_MEMORY_COPY,
                printf ("SYSCALL_MEMORY_COPY: dest=%t, local addr=0x%lx "
                        "size=0x%lx dir=0x%lx\n",
                        TID (remote), local, size, direction));

    get_current_tcb()->set_error_code(ENOT_IMPLEMENTED);
    PROFILE_STOP(sys_remote_memcopy);
    return_memory_copy(0, 0, cont);
}
#endif    /* !CONFIG_REMOTE_MEMORY_COPY */
