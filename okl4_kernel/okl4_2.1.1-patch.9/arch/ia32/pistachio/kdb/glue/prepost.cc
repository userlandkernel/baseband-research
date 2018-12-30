/*
 * Copyright (c) 2002-2004, Karlsruhe University
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
 * Description:   IA-32 specific handlers for KDB entry and exit
 */

#include <l4.h>
#include <debug.h>
#include <kdb/kdb.h>
#include <kdb/console.h>
#include <kernel/kdb/names.h>

#include <debug.h>
#include <linear_ptab.h>
#include <tcb.h>

#include <arch/traps.h>      /* for IA32 exception numbers   */
#include <arch/trapgate.h>   /* for ia32_exceptionframe_t    */

#include <plat/nmi.h>

#include <profile.h>

#if defined(CONFIG_KDB_DISAS)
extern "C" int disas(addr_t pc);
INLINE void disas_addr (addr_t ip, char * str = "")
{
    printf ("%p    ", ip);
    disas (ip);
    printf ("\n");
}
#else
INLINE void disas_addr (addr_t ip, char * str = "")
{
    printf ("eip=%p -- %s\n", ip, str);
}
#endif

/*
 * Handle a breakpoint exception
 * This runs on thread stack, not KDB stack!
 *
 * @return: True - handled, False - enter kdebug
 */
bool ia32_handle_break(ia32_exceptionframe_t *f)
{
    space_t * space = get_current_space();
    if (!space) space = get_kernel_space();

    addr_t addr = (addr_t)(f->eip);

    unsigned char c;
    if (! readmem(space, addr, &c) )
        return false;

    if (c == 0x90)
    {
        addr = addr_offset(addr, 1);
        if (! readmem(space, addr, &c) )
            return true;
    }

    /*
     * KDB_Enter() is implemented as:
     *
     *  int     $3
     *  jmp     1f
     *  mov     $2f, %eax
     *   1:
     *
     *  .rodata
     *   2: .ascii      "output text"
     *  .byte   0
     *
     */
    if (c == 0xeb) /* jmp rel */
    {
        return false;
    }

    /*
     * Other kdebug operations are implemented as follows:
     *
     *  int     $3
     *  cmpb    <op>, %al
     *
     */
    else if (c == 0x3c) /* cmpb */
    {
        if (!readmem (space, addr_offset(addr, 1), &c))
            return false;

        switch (c)
        {
#if defined(CONFIG_KDB_CONS)
            case 0x0:
                //
                // KDB_PrintChar()
                //
                putc (f->eax);
                return true;

            case 0x1:
                {
                    //
                    // KDB_PrintString()
                    //
                    addr_t user_addr = (addr_t) f->eax;
                    while (readmem (space, user_addr, &c) && (c != 0))
                    {
                        putc (c);
                        user_addr = addr_offset (user_addr, 1);
                    }
                    if (c != 0)
                        printf ("[string not completely mapped]");
                    return true;
                }

            case 0xd:
                //
                // KDB_ReadChar_Blocked()
                //
                f->eax = getc (true);
                return true;

            case 0x8:
                //
                // KDB_ReadChar()
                //
                f->eax = getc (false);
                return true;
#elif defined(CONFIG_DEBUG)
            case 0x0:
            case 0x1:
                return true;
            case 0xd:
            case 0x8:
                f->eax = 0;
                return true;
#endif
#if defined(CONFIG_THREAD_NAMES) \
        || defined(CONFIG_SPACE_NAMES) \
        || defined(CONFIG_MUTEX_NAMES)
            case 0xf:
                //
                // KDB_Set_ObjectName()
                //
                {
                    char name[MAX_DEBUG_NAME_LENGTH];
                    word_t type = f->ebx;
                    word_t id = f->edi;
                    word_t *name_words = (word_t *)name;

                    /* Copy the debugging name from registers. */
                    name_words[0] = f->eax;
                    name_words[1] = f->esi;
                    name_words[2] = f->ecx;
                    name_words[3] = f->edx;
                    name[MAX_DEBUG_NAME_LENGTH - 1] = '\0';

                    /* Set the object's name. */
                    (void)kdb_set_object_name((object_type_e)type, id, name);
                    return true;
                }
#elif defined(CONFIG_DEBUG)
            case 0xf:
                return true;
#endif

#if defined(CONFIG_L4_PROFILING)
            case 0x7:
                profile_handler(f->eax);
                return true;
#endif

            default:
                printf("kdb: unknown opcode: int3, cmpb %d\n",
                        space->get_from_user(addr_offset(addr, 1)));
        }
        return false;
    }
    return false;
}

/*
 * Handle a breakpoint exception
 * This runs on thread stack, not KDB stack!
 *
 * @return: True - handled, False - enter kdebug
 */
bool ia32_handle_debug(void)
{
    printf("handle break\n");
    return false;
}

static spinlock_t kdb_lock;

/*
 * KDB entry checks - running on KDB stack
 */
bool SECTION(SEC_KDEBUG) kdb_t::pre()
{
    bool enter_kernel_debugger = true;

    ia32_exceptionframe_t* f = (ia32_exceptionframe_t*) kdb_param;

    switch (f->reason)
    {
    case IA32_EXC_DEBUG:        /* single step, hw breakpoints */
    {
        /* Debug exception */
        if (f->eflags & (1 << 8))
        {
            extern u32_t ia32_last_eip;
#if defined(CONFIG_CPU_IA32_I686) || defined(CONFIG_CPU_IA32_P4)
            extern bool ia32_single_step_on_branches;
            if (ia32_single_step_on_branches)
            {
                addr_t last_branch_ip;
                ia32_wrmsr (IA32_DEBUGCTL, 0);
                ia32_single_step_on_branches = false;
#if defined(CONFIG_CPU_IA32_I686)
                last_branch_ip = (addr_t) (word_t)
                    ia32_rdmsr (IA32_LASTBRANCHFROMIP);
#else
                last_branch_ip = (addr_t) (word_t)
                    (ia32_rdmsr (IA32_LASTBRANCH_0 +
                                 ia32_rdmsr (IA32_LASTBRANCH_TOS)) >> 32);
#endif
                disas_addr (last_branch_ip, "branch to");
                ia32_last_eip = f->eip;
            }
#endif
            if (ia32_last_eip != ~0U)
                disas_addr ((addr_t) ia32_last_eip);
            f->eflags &= ~((1 << 8) + (1 << 16));       /* !RF + !TF */
            ia32_last_eip = ~0U;
        }
        else
        {
            printf("--- Debug Exception ---\n");
        }
        break;
    }

    case IA32_EXC_NMI:          /* well, the name says enough   */
        printf("--- NMI ---\n");
        break;

    case IA32_EXC_BREAKPOINT: /* int3 */
    {
        space_t * space = kdb.kdb_current->get_space();
        if (!space) space = get_kernel_space();

        addr_t addr = (addr_t)(f->eip);

        unsigned char c;
        if (! readmem(space, addr, &c) )
            break;

        if (c == 0x90)
        {
            addr = addr_offset(addr, 1);
            enter_kernel_debugger = false;
        }

        if (! readmem(space, addr, &c) )
            break;

        /*
         * KDB_Enter() is implemented as:
         *
         *      int     $3
         *      jmp     1f
         *      mov     $2f, %eax
         *   1:
         *
         *      .rodata
         *   2: .ascii  "output text"
         *      .byte   0
         *
         */
        if (c == 0xeb) /* jmp rel */
        {
            if (! readmem(space, addr_offset(addr, 2), &c) )
                break;

#if defined(CONFIG_KDB_CONS)
            if (c == 0xb8)
            {
                printf("--- \"");

                /* mov addr, %eax */
                addr_t user_addr;
                if (! readmem (space, addr_offset(addr, 3),
                               (word_t *) (void*)&user_addr))
                    printf("[string address not mapped]");
                else
                {
                    while (readmem(space, user_addr, &c) && (c != 0))
                    {
                        putc(c);
                        user_addr = addr_offset(user_addr, 1);
                    }
                    if (c != 0)
                        printf ("[string not completely mapped]");
                }

                printf("\" ---\n"
                       "--------------------------------- (eip=%p, esp=%p) ---\n",
                       f->eip - 1, f->esp);
            }
#endif
        }
    }
    break;

    default:
        printf("--- KD# unknown reason ---\n");
        break;
    } /* switch */

    if (enter_kernel_debugger)
        kdb_lock.lock();

    return enter_kernel_debugger;
};




void SECTION(SEC_KDEBUG) kdb_t::post() {

    ia32_exceptionframe_t* f = (ia32_exceptionframe_t*) kdb_param;

    switch (f->reason)
    {
    case IA32_EXC_DEBUG:
        /* Set RF in EFLAGS. This will disable breakpoints for one
           instruction. The processor will reset it afterwards. */
        f->eflags |= (1 << 16);
        break;

    case IA32_EXC_NMI:
        nmi_t().unmask();
        break;

    } /* switch */

    kdb_lock.unlock();

    return;
};
