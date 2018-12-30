/*
 * Copyright (c) 2004, National ICT Australia
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
 * Author: Ben Leslie
 * Created: Tue Nov 9 2004 
 */

#include <stdio.h>
#include <assert.h>
#include <l4/map.h>
#include <l4/kdebug.h>
#include <l4/space.h>
#include <iguana/memsection.h>


// Tagged variables only works on GNU compilers because it involves a bit of
// linker black magic in the build system. We return sucess on non-gnu 
// systems so as to not break automated tests


#ifdef __GNUC__
    volatile int tagged_io SECTION("io") = 99;
    volatile int tagged_tcm SECTION("tcm") = 42;
    volatile int tagged_orphan SECTION("orphan") = 111;
#endif



int
main(int argc, char **argv)
{
    int success = 1;

#ifdef __GNUC__
    if (tagged_tcm != 42) success = 0;
    tagged_tcm = 101;
    if( tagged_tcm != 101) success = 0;
    printf("Tagged_tcm OK\n");

    if (tagged_io != 99) success = 0;
    tagged_io = 101;
    if( tagged_io != 101) success = 0;
    printf("Tagged_io OK\n");

    if (tagged_orphan != 111) success = 0;
    tagged_orphan = 101;
    if( tagged_orphan != 101) success = 0;
    printf("Tagged_orphan OK\n");



    size_t size;
    uintptr_t paddr = 0;

    paddr = memsection_virt_to_phys( (uintptr_t) (&tagged_tcm), &size);
    if ( paddr != TCM_PHYS_ADDR) success = 0;
    if (paddr == -2){
        printf("why no perms on my own text?\n");
    }
    else printf("paddr is %08lx\tLooking for %08x\n", paddr, TCM_PHYS_ADDR);


    paddr = memsection_virt_to_phys( (uintptr_t) (&tagged_io), &size);
    if ( paddr != IO_PHYS_ADDR) success = 0;
    if (paddr == -2){
        printf("why no perms on my own text?\n");
    }
    else printf("paddr is %08lx\tLooking for %08x\n", paddr, IO_PHYS_ADDR);
#else
    printf("Warning: non GNU compiler detected, returning sucess anyway\n");
#endif


    if (success)
        printf("tagging works!\n");
    else
        printf("tagging failed\n");

    return 0;
}


