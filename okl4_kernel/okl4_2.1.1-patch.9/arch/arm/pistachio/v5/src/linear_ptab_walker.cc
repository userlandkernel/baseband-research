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
 * Description:   Linear page table manipulation - ARM specifics
 */
#include <l4.h>
#include <linear_ptab.h>
#include <space.h>
#include <tcb.h>
#include <arch/pgent.h>

bool space_t::domain_unlink(generic_space_t *source)
{
    addr_t vaddr = (addr_t)0;

    word_t num;
    pgent_t *t_pgs[pgent_t::size_max] = { NULL };
    word_t r_num[pgent_t::size_max];

    pgent_t *t_pg = this->pgent(0);

    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    num = page_table_size(pgsize);

    while (num)
    {
        if (!is_user_area(vaddr)) {
            break;
        }

        //printf("try remove window: @ %p\n", vaddr);

        if (t_pg->is_valid(this, pgsize) &&
                (t_pg->is_window_level(this, pgsize) == 2))
        {
            //printf("traverse subtree %p @ %p\n", this, vaddr);

            // Recurse into subtree
            if ((word_t)pgsize == 0)
                return false;

            t_pgs[pgsize-1] = t_pg;
            r_num[pgsize-1] = num;

            t_pg = t_pg->subtree(this, pgsize);
            pgsize--;

            num = page_table_size(pgsize);
            continue;
        }

        if (t_pg->is_window(this, pgsize) == 1)
        {
            if (t_pg->get_window(this) == source)
            {
                //printf("unlink - clear %p : @ %p\n", t_pg, vaddr);
                t_pg->clear_window(this);
            }
        }

        num --;
        t_pg = t_pg->next(this, pgsize, 1);
        vaddr = addr_offset(vaddr, page_size(pgsize));
        //printf(" -- next: %p\n", vaddr);

        while ((num == 0) && (pgsize < pgent_t::size_max)) {
            //printf("recurse up\n");
            t_pg = t_pgs[pgsize];
            num = r_num[pgsize];
            pgsize++;

            num--;
            t_pg = t_pg->next(this, pgsize, 1);
        }
    }

    return true;
}

bool space_t::window_share_fpage(space_t *space, fpage_t fpage)
{
    addr_t vaddr = (addr_t)0;

    word_t num;
    pgent_t *t_pgs[pgent_t::size_max] = { NULL };
    word_t r_num[pgent_t::size_max];

    pgent_t *t_pg = this->pgent(0);

    pgent_t::pgsize_e pgsize = pgent_t::size_max;
    num = page_table_size(pgsize);

    while (num)
    {
        if (!is_user_area(vaddr)) {
            break;
        }
        if (!fpage.is_range_overlapping(vaddr, addr_offset(vaddr, page_size(pgsize)))) {
            goto next;
        }

        //printf("try tag address: @ %p\n", vaddr);

        if (!t_pg->is_valid(this, pgsize) &&
                (t_pg->is_window_level(this, pgsize) == 2))
        {
            if (!t_pg->make_subtree(this, pgsize, false)) {
                printf("make t_subtree failed\n");
            }
            //printf("make t_subtree %p @ %p\n", this, vaddr);
        }

        if (t_pg->is_valid(this, pgsize))
        {
            if (t_pg->is_subtree(this, pgsize))
            {
                if (t_pg->is_window_level(this, pgsize) < 2)
                {
                    printf("conflicting subtree - cannot add window %p @ %p\n",
                            this, vaddr);
                    get_current_tcb ()->set_error_code(EDOMAIN_CONFLICT);
                    goto error_out;
                }

                // Recurse into subtree
                if ((word_t)pgsize == 0)
                    return false;

                t_pgs[pgsize-1] = t_pg;
                r_num[pgsize-1] = num;

                t_pg = t_pg->subtree(this, pgsize);
                pgsize--;

                num = page_table_size(pgsize);
                continue;
            }
            else
            {
                printf("conflicting mapping - cannot add window %p @ %p\n",
                        this, vaddr);
                get_current_tcb ()->set_error_code(EDOMAIN_CONFLICT);
                goto error_out;
            }
        }
        else
        {
            switch (t_pg->is_window(this, pgsize)) {
            case 1:
                if (fpage.get_rwx() == 0) {
                    // printf("rm window: %p   ent = %p\n", vaddr, t_pg);
                    t_pg->clear_window(this);
                } else {
                    printf("window in the way?\n");
                    get_current_tcb ()->set_error_code(EDOMAIN_CONFLICT);
                    goto error_out;
                }
                break;
            case 0:
                if (fpage.get_rwx()) {
                    if (fpage.is_meta() == 0) {
                        // printf("add flt window: %p   ent = %p\n", vaddr, t_pg);
                        t_pg->set_window_faulting(this, space);
                    } else {
                        // printf("add cbk window: %p   ent = %p\n", vaddr, t_pg);
                        t_pg->set_window_callback(this, space);
                    }
                } else {
                    printf("no window?\n");
                }
                //printf(" -- : %p / %lx\n", t_pg, t_pg->l1.raw);
                break;
            case 2:
            default:
                panic("bad addresspace state\n");
                break;
            }
        }

next:
        num --;
        t_pg = t_pg->next(this, pgsize, 1);
        vaddr = addr_offset(vaddr, page_size(pgsize));
        //printf(" -- next: %p\n", vaddr);

        while ((num == 0) && (pgsize < pgent_t::size_max)) {
            //printf("recurse up\n");
            t_pg = t_pgs[pgsize];
            num = r_num[pgsize];
            pgsize++;

            num--;
            t_pg = t_pg->next(this, pgsize, 1);
        }
    }

    return true;

error_out:
    return false;
}

