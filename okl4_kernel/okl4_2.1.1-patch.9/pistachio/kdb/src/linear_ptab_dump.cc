/*
 * Copyright (c) 2002, 2003-2004, Karlsruhe University
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 2005-2006, National ICT Australia
 */
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
 * Description:   Linear page table dump
 */
#include <l4.h>
#include <debug.h>
#include <kdb/cmd.h>
#include <kdb/kdb.h>
#include <kdb/input.h>
#include <linear_ptab.h>

#include <arch/pgent.h>
#include <tcb.h>

#if defined(CONFIG_KDB_CLI)

void get_ptab_dump_ranges (addr_t * vaddr, word_t * num,
                           pgent_t::pgsize_e * max_size);

#if !defined(CONFIG_ARCH_IA32) && !defined(CONFIG_ARCH_X86_64)
SECTION(SEC_KDEBUG) void
get_ptab_dump_ranges (addr_t * vaddr, word_t * num, pgent_t::pgsize_e * max_size)
{
    *vaddr = (addr_t) 0;
    *num = page_table_size (pgent_t::size_max);
    *max_size = pgent_t::size_max;
}
#endif


/**
 * cmd_dump_ptab: dump page table contents
 */
DECLARE_CMD (cmd_dump_ptab, root, 'p', "ptab", "dump page table");

CMD(cmd_dump_ptab, cg)
{
    static char spaces[] = "                                ";
    char * spcptr = spaces + sizeof (spaces) - 1;
    char * spcpad = spcptr - (word_t)pgent_t::size_max * 2;

    space_t * space;
    addr_t vaddr;
    word_t num;
    pgent_t * pg;
    pgent_t::pgsize_e size, max_size;

    // Arrays to implement recursion
    pgent_t * r_pg[pgent_t::size_max];
    word_t r_num[pgent_t::size_max];

    word_t sub_trees = 0;
    word_t print_tree = 0;

    addr_t sub_addr[pgent_t::size_max];
    pgent_t * sub_pg[pgent_t::size_max];

    // Get dump arguments
    space = get_space ("Space");
    printf("Dumping space 0x%lx:\n", space);
    space = space ? space : get_kernel_space();

    size = pgent_t::size_max;

    get_ptab_dump_ranges (&vaddr, &num, &max_size);
    pg = space->pgent(0)->next(space, size, page_table_index(size, vaddr));

    while (size != max_size)
    {
        ASSERT(DEBUG, pg->is_subtree (space, size));
        pg = pg->subtree (space, size--);
        pg = pg->next (space, size, page_table_index (size, vaddr));
    }

#if defined(CONFIG_ARCH_ARM) && defined(CONFIG_ENABLE_FASS)
    word_t domain = space->get_domain();
#endif
    while (num > 0)
    {
        if (pg->is_valid (space, size))
        {
            if (pg->is_subtree (space, size))
            {
                // Recurse into subtree
#if defined(CONFIG_ARCH_ARM) && defined(CONFIG_ENABLE_FASS)
                if ((space == get_kernel_space() || !space->is_domain_area(vaddr))
                                && size == pgent_t::size_1m)
                    domain = pg->get_domain();
#endif

                print_tree |= 1UL<<sub_trees;
                sub_trees++;

                sub_addr[size-1] = vaddr;
                sub_pg[size-1] = pg;

                size--;
                r_pg[size] = pg->next (space, size+1, 1);
                r_num[size] = num - 1;
                spcptr -= 2;
                spcpad += 2;

                pg = pg->subtree (space, size+1);
                num = page_table_size (size);
                continue;
            }
            else
            {
                // Print subtrees
                for (word_t i = sub_trees; i > 0; i--)
                {
                    word_t s = i + (word_t)size - 1;

                    if (print_tree & (1UL<<(sub_trees-i)))
                    {
                        print_tree &= ~(1UL<<(sub_trees-i));
                        /*lint -e662 We know that this doesn't overflow the array at this
                         * point.
                         */
                        printf ("%p [%p]:%s tree=%p\n", sub_addr[s],
                                sub_pg[s]->raw, spcptr + (i*2),
                                sub_pg[s]->subtree (space, (pgent_t::pgsize_e)s+1));
                    }
                }
                // Print valid mapping
                word_t pgsz = page_size (size);
                word_t rwx = pg->reference_bits (space, size, vaddr);
                printf ("%p [%p]:%s phys=%p pg=%p %s%3d%cB %c%c%c "
                        "(%c%c%c) %s"
                        ,vaddr, pg->raw, spcptr, pg->address (space, size), pg,
                        spcpad, (pgsz >= GB (1) ? pgsz >> 30 :
                                pgsz >= MB (1) ? pgsz >> 20 : pgsz >> 10),
                        pgsz >= GB (1) ? 'G' : pgsz >= MB (1) ? 'M' : 'K',
                        pg->is_readable (space, size)   ? 'r' : '~',
                        pg->is_writable (space, size)   ? 'w' : '~',
                        pg->is_executable (space, size) ? 'x' : '~',
                        rwx & 4 ? 'R' : '~',
                        rwx & 2 ? 'W' : '~',
                        rwx & 1 ? 'X' : '~',
                        pg->is_kernel (space, size) ? "kernel" : "user  ");
                pg->dump_misc (space, size);
#if defined(CONFIG_ARCH_ARM) && defined(CONFIG_ENABLE_FASS)
                if ((space == get_kernel_space() || !space->is_domain_area(vaddr))
                                && size == pgent_t::size_1m)
                    domain = pg->get_domain();
                printf(" domain = %d", domain);
#endif
                printf ("\n");
            }
        }
#if defined(CONFIG_ARM_V5)
        else if (pg->is_window(space, size) == 1)
        {
            printf("%p window -> %S   (%s)\n", vaddr, pg->get_window(space),
                    pg->is_callback() ? "callback" : "faulting");
        }
#endif

        // Goto next ptab entry
        vaddr = addr_offset (vaddr, page_size (size));
        pg = pg->next (space, size, 1);
        num--;

        while (num == 0 && size < max_size)
        {
            // Recurse up from subtree
            pg = r_pg[size];
            num = r_num[size];
            size++;
            spcptr += 2;
            spcpad -= 2;
            sub_trees--;
        }
    }

    return CMD_NOQUIT;
}

#endif
