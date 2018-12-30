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
 * Description: Initialise iguana, start init task and jump to the server
 * Authors: Ben Leslie, Alex Webster 
 * Created: Sat Jul 7 2004 
 */

#include <l4/types.h>
#include <l4/kdebug.h>
#include <l4e/misc.h>
#include <l4e/map.h>

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <bootinfo/bootinfo.h>

#include "objtable.h"
#include "pd.h"
#include "thread.h"
#include "bootinfo.h"
#include "util.h"
#include "space.h"
#include "extensions.h"
#include "mutex.h"

/*
 * Obligatory promotion 
 */

#if defined(IG_DEBUG_PRINT)
static char banner[] =
    "\n"
    " *****************************************\n"
    " *  Iguana system, Copyright 1996-2006   *\n"
    " *  ERTOS - National ICT Australia       *\n"
    " *****************************************\n\n";
#endif

/*
 * Forward declaration of our server loop 
 */
extern void iguana_server_loop(void);

/* Address of the bootinfo buffer. */
extern void* __okl4_bootinfo;

int
main(void)
{
    int r;

    DEBUG_PRINT(banner);
#if defined(ARCH_ARM) && ARCH_VER <= 5
    /* Place iguana in L4 ARM vspace #1 */
    (void)L4_SpaceControl(L4_rootspace, L4_SpaceCtrl_resources, L4_rootclist,
                          L4_Nilpage, (1 << 16), NULL);
    /* Cache Flush */
    (void)L4_CacheFlushAll();
#endif
    INIT_PRINT("Processing Boot Info: %p\n", __okl4_bootinfo);

    utcb_init();

    mutex_init();
    space_init();
    pd_init();
    objtable_init();
    thread_init();

    r = bi_execute(__okl4_bootinfo);
    if (r != 0) {
#if defined(IGUANA_DEBUG)
        L4_KDB_Enter("PANIC: Bootinfo did not initialise correctly");
#endif
        while (1);
    }

    extensions_init();

    /*
     * TODO: We could reclaim memory here is we need to 
     */

    /* Now that we are ready to roll, lets start the server */
    INIT_PRINT("iguana_server\n");

    iguana_server_loop();

    assert(!"Should never reach here");

    return 0;
}
