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

#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <stddef.h>
#include <stdio.h>
#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/caps.h>

static L4_ThreadId_t main_thread;

#define PAGESIZE 0x1000
ALIGNED(PAGESIZE) static char global_stack_area[PAGESIZE] = {1};
#define stack_top ((L4_Word_t)(global_stack_area + PAGESIZE))

/**
 * Utility function for creating address spaces.
 *
 * @param[in] space the space id to use for the new space
 * @param[in] clist_id the clist for the space to use
 * @param[in] utcb_fpage the memory to be used for UTCBs
 */
static L4_Word_t
create_address_space(L4_SpaceId_t space, L4_ClistId_t clist_id, L4_Fpage_t utcb_fpage)
{
    L4_Word_t res;

#ifdef NO_UTCB_RELOCATE
    utcb_fpage = L4_Nilpage;
#endif

    res = L4_SpaceControl(space, L4_SpaceCtrl_new, clist_id,
            utcb_fpage, 0, NULL);
    return res;
}


static L4_Word_t
delete_address_space(L4_SpaceId_t space)
{
    return L4_SpaceControl(space, L4_SpaceCtrl_delete,
            L4_rootclist, L4_Nilpage, 0, NULL);
}

static L4_Word_t
create_thread_in_space(L4_ThreadId_t tid, L4_SpaceId_t space, L4_Word_t utcb)
{
    L4_Word_t result, dummy;

#ifdef NO_UTCB_RELOCATE
        utcb = -1UL;
#endif

    result = L4_ThreadControl(tid, space, default_thread_handler, default_thread_handler,
            L4_nilthread, 0, (void*)utcb);
    if (result != 1) {
        printf("thread create: error no: %"PRI_D_WORD"\n", L4_ErrorCode());
    }

    /* Only let the thread run on the first hardware thread / hardware core. */
    L4_Schedule(tid, -1, 0x00000001, -1, -1, 0, &dummy);

    return result;
}

/*********************************************************************
 *                                                                   *
 *                      CLIST CREATE TESTS                           *
 *                                                                   *
 *********************************************************************/

/*
\begin{test}{CLIST0100}
  \TestDescription{Create a clist, then delete it}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create clists of various sizes
      \item Check the return value 
      \item Delete the clist
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CLIST0100)
{
    L4_Word_t success, ErrorCode;

    L4_Word_t size = 0x1;
    L4_Word_t multiple = 0x3;
    L4_Word_t end_size = 0x80000;

    L4_ClistId_t clist = L4_ClistId(1);

    while (size < end_size) {
        success = L4_CreateClist(clist, size);
        ErrorCode = L4_ErrorCode();
        _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

        success = L4_DeleteClist(clist);
        fail_unless(success, "Delete clist failed");

        size *= multiple;
    }
}
END_TEST

/*
\begin{test}{CLIST0200}
  \TestDescription{Create a clist, create space using clist, then delete them}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create clist
      \item Check the return value 
      \item Invoke \Func{SpaceControl} to create space associated with clist
      \item Check the return value 
      \item Delete the space
      \item Check the return value 
      \item Delete the clist
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CLIST0200)
{
    L4_Word_t success, ErrorCode;

    L4_Word_t size = 0x100;

    L4_ClistId_t clist = L4_ClistId(1);
    L4_SpaceId_t space = L4_SpaceId(8);

    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    success = delete_address_space(space);
    fail_unless(success, "Delete address space failed");

    success = L4_DeleteClist(clist);
    fail_unless(success, "Delete clist failed");
}
END_TEST

/*
  \begin{test}{CLIST1000}
  \TestDescription{Create each valid clist id}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create clist for each valid id
  \item Check the return value of each invocation
  \item Delete each clist
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST1000)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist;
    L4_Word_t size = 0x100;
    int max = 16;
    int id;

    /* 16 clists allowed? */
    for (id = 1; id < max; id++) {
        clist = L4_ClistId(id);
        success = L4_CreateClist(clist, size);
        ErrorCode = L4_ErrorCode();
        _fail_unless(success, __FILE__, __LINE__, "Clist create failed at id %d: ErrorCode=%"PRI_D_WORD"", id, ErrorCode);
    }

    /* Delete everything we just created. */
    for (id = 1; id < max; id++) {
        clist = L4_ClistId(id);
        success = L4_DeleteClist(clist);
        fail_unless(success, "Delete clist failed");
    }

}
END_TEST

/*
  \begin{test}{CLIST1100}
  \TestDescription{Create a clist using an invalid clist id}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} with -1
  \item Check the return value
  \item Invoke \Func{CapControl} with (max)
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST1100)
{
    L4_Word_t success;
    L4_ClistId_t clist;
    L4_Word_t size = 0x100;
    int max = 16; /* 16 clists allowed? */
    
    clist = L4_ClistId(-1);
    success = L4_CreateClist(clist, size);
    fail_unless(!success, "Create clist with id -1 succeeded");
    
    clist = L4_ClistId(max);
    success = L4_CreateClist(clist, size);
    fail_unless(!success, "Create clist with max id succeeded");

}
END_TEST

/*
  \begin{test}{CLIST1200}
  \TestDescription{Create a clist using a valid, but used, clist id}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} with an id of 1
  \item Check the return value
  \item Invoke \Func{CapControl} again with an id of 1
  \item Check the return value
  \item Delete the clist
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST1200)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist;
    L4_Word_t size = 0x100;
    
    /* Create a clist with id 1 */
    clist = L4_ClistId(1);
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
    
    /* Try and create a clist with the same id */
    success = L4_CreateClist(clist, size);
    fail_unless(!success, "Create clist with same id twice succeeded");

    /* Delete the clist */
    success = L4_DeleteClist(clist);
    fail_unless(success, "Delete clist failed");
}
END_TEST


/*********************************************************************
 *                                                                   *
 *                      CLIST DELETE TESTS                           *
 *                                                                   *
 *********************************************************************/

/*
  \begin{test}{CLIST2000}
  \TestDescription{Delete a clist using a valid but unused clist id}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to delete a clist with an id of 1
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST2000)
{
    L4_Word_t success;
    L4_ClistId_t clist;
    
    /* Delete a clist with id 1.
     * This assumes we haven't created a clist with id 1 - 
     * prior tests should definitely clean up after themselves. */
    clist = L4_ClistId(1);
    success = L4_DeleteClist(clist);
    fail_unless(!success, "Delete valid but unused clist id succeeded");
}
END_TEST

/*
  \begin{test}{CLIST2100}
  \TestDescription{Delete a clist using an invalid clist id}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to delete a clist with an id of -1
  \item Check the return value
  \item Invoke \Func{CapControl} to delete a clist with an id of (max)
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST2100)
{
    L4_Word_t success;
    L4_ClistId_t clist;
    int max = 16; /* 16 clists allowed? */

    /* Delete a clist with id -1 */
    clist = L4_ClistId(-1);
    success = L4_DeleteClist(clist);
    fail_unless(!success, "Delete clist with id -1 succeeded");
    
    /* Delete a clist with id max */
    clist = L4_ClistId(max);
    success = L4_DeleteClist(clist);
    fail_unless(!success, "Delete clist with id max succeeded");
}
END_TEST

/*
  \begin{test}{CLIST2200}
  \TestDescription{Delete a valid, but active, clist}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create a clist with an id of 1
  \item Check the return value
  \item Invoke \Func{SpaceControl} to create a space, associated with the clist
  \item Check the return value
  \item Invoke \Func{CapControl} to attempt to delete the clist
  \item Check the return value; this attempt should fail
  \item Invoke \Func{SpaceControl} to delete the space
  \item Check the return value
  \item Invoke \Func{CapControl} to attempt to delete the clist
  \item Check the return value; this attempt should pass
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST2200)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist;
    L4_Word_t size = 0x100;
    L4_SpaceId_t space = L4_SpaceId(8);
    
    /* Create a clist with id 1 */
    clist = L4_ClistId(1);
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create a space associated with this clist. */
    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Attempt to delete the clist; should fail. */
    success = L4_DeleteClist(clist);
    fail_unless(!success, "Delete active clist succeeded");

    /* Delete the space. */
    success = delete_address_space(space);
    fail_unless(success, "Delete address space failed");

    /* Attempt to delete the clist; should pass. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST

/*
  \begin{test}{CLIST2300}
  \TestDescription{Delete a valid, inactive, clist}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create a clist with an id of 1
  \item Check the return value
  \item Invoke \Func{CapControl} to delete the clist
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST2300)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist;
    L4_Word_t size = 0x100;

    /* Create a clist with id 1 */
    clist = L4_ClistId(1);
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the clist. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST

/*
  \begin{test}{CLIST2400}
  \TestDescription{Delete a clist that contains caps}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create a clist with an id of 1
  \item Check the return value
  \item Invoke \Func{CapControl} to create a cap in the clist
  \item Check the return value
  \item Invoke \Func{CapControl} to attempt to delete the clist
  \item Check the return value; this attempt should fail
  \item Invoke \Func{CapControl} to delete the cap and then the clist
  \item Check the return value; this attempt should pass
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CLIST2400)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist;
    L4_Word_t size = 0x100;
    
    /* Create a clist with id 1 */
    clist = L4_ClistId(1);
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create a cap in this clist. */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(1, 1), clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Attempt to delete the clist; should fail. */
    success = L4_DeleteClist(clist);
    fail_unless(!success, "Delete clist with active caps succeeded");

    /* Delete the cap. */
    success = L4_DeleteCap(clist, L4_GlobalId(1, 1));
    fail_unless(success, "Delete cap failed");

    /* Attempt to delete the clist; should pass. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST

/*********************************************************************
 *                                                                   *
 *                      SPACE CREATION TESTS                         *
 *                                                                   *
 *********************************************************************/

/*
  \begin{test}{CAPSPACE1000}
  \TestDescription{Create a space with a valid, but non-existent, clist id}
  \TestFunctionalityTested{\Func{SpaceControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{SpaceControl} to create a space with a clist id of 1
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPSPACE1000)
{
    L4_Word_t success;
    L4_SpaceId_t space = L4_SpaceId(8);

    /* Create a space associated with clist with id 1.
     * This obviously assumes that clist 1 hasn't been created. */
    success = create_address_space(space, L4_ClistId(1), L4_Fpage(0xb10000, 0x1000));
    fail_unless(!success, "Space create with valid but non-existent clist id succeeded");
}
END_TEST

/*
  \begin{test}{CAPSPACE1100}
  \TestDescription{Create a space with an invalid clist id}
  \TestFunctionalityTested{\Func{SpaceControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{SpaceControl} to create a space with a clist id of -1
  \item Check the return value
  \item Invoke \Func{SpaceControl} to create a space with a clist id of max
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPSPACE1100)
{
    L4_Word_t success;
    L4_SpaceId_t space = L4_SpaceId(8);
    int max = 16;

    /* Create a space associated with clist with id -1. */
    success = create_address_space(space, L4_ClistId(-1), L4_Fpage(0xb10000, 0x1000));
    fail_unless(!success, "Space create with invalid clist id of -1 succeeded");

    /* Create a space associated with clist with id max. */
    success = create_address_space(space, L4_ClistId(max), L4_Fpage(0xb10000, 0x1000));
    fail_unless(!success, "Space create with invalid clist id of max succeeded");
}
END_TEST

/*
  \begin{test}{CAPSPACE1200}
  \TestDescription{Create a space with an valid, existent clist id}
  \TestFunctionalityTested{\Func{SpaceControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create a clist with a clist id of 1
  \item Check the return value
  \item Invoke \Func{SpaceControl} to create a space with the new clist
  \item Check the return value
  \item Invoke \Func{SpaceControl} to delete the space
  \item Check the return value
  \item Invoke \Func{CapControl} to delete the clist
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPSPACE1200)
{
    L4_Word_t success, ErrorCode;
    L4_ClistId_t clist;
    L4_SpaceId_t space = L4_SpaceId(8);
    L4_Word_t size = 0x100;
    
    /* Create a clist with id 1 */
    clist = L4_ClistId(1);
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create a space associated with this clist. */
    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the space. */
    success = delete_address_space(space);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the clist. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST

/*********************************************************************
 *                                                                   *
 *                        CAP CREATION TESTS                         *
 *                                                                   *
 *********************************************************************/

/*
\begin{test}{CAP0100}
  \TestDescription{Create an IPC CAP, then delete it}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create ipc-cap at various slots
      \item Check the return value 
      \item Delete the cap
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CAP0100)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t slot = 0;
    L4_ClistId_t clist = L4_ClistId(1);

    success = L4_CreateClist(clist, 128);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    while (slot < 128) {
        success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(slot, 1), clist);
        ErrorCode = L4_ErrorCode();
        _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

        success = L4_DeleteCap(clist, L4_GlobalId(slot, 1));
        fail_unless(success, "Delete cap failed");

        slot++;
    }

    success = L4_DeleteClist(clist);
    fail_unless(success, "Delete clist failed");
}
END_TEST


/*
\begin{test}{CAP0110}
  \TestDescription{Create n IPC CAPs, then delete them}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create ipc-caps at various slots
      \item Check the return value 
      \item Delete the caps
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CAP0110)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t slot = 0;
    L4_ClistId_t clist = L4_ClistId(1);

    success = L4_CreateClist(clist, 128);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    while (slot < 128) {
        success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(slot, 1), clist);
        ErrorCode = L4_ErrorCode();
        _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

        slot++;
    }

    slot = 0;
    while (slot < 128) {
        success = L4_DeleteCap(clist, L4_GlobalId(slot, 1));
        fail_unless(success, "Delete cap failed");

        slot++;
    }

    success = L4_DeleteClist(clist);
    fail_unless(success, "Delete clist failed");
}
END_TEST


/*
  \begin{test}{CAP1000}
  \TestDescription{Create an IPC Cap in an invalid slot}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap in slot -1
  \item Check the return value 
  \item Invoke \Func{CapControl} to create an ipc cap in slot max
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP1000)
{
    L4_Word_t success;
    int max = 1024;
    L4_ClistId_t clist = L4_ClistId(1);

    success = L4_CreateClist(clist, max);
    fail_unless(success, "Create clist failed");

    /* Attempt to create at slot -1 */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(-1, 1), clist);
    fail_unless(!success, "Create IPC Cap at slot -1 succeeded");

    /* Attempt to create at slot max */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(max, 1), clist);
    fail_unless(!success, "Create IPC Cap at max slot succeeded");

    success = L4_DeleteClist(clist);
    fail_unless(success, "Delete clist failed");
}
END_TEST

/*
  \begin{test}{CAP1100}
  \TestDescription{Create an IPC Cap in a used slot}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap in a slot
  \item Check the return value 
  \item Invoke \Func{CapControl} to create an ipc cap in the same slot
  \item Check the return value; this should fail.
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP1100)
{
    L4_Word_t success;

    /* Create cap at slot 8 (just to avoid conflicts)  */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(8, 1), L4_rootclist);
    fail_unless(success, "Create IPC Cap at slot 1 failed");

    /* Attempt to create at slot 8 again */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(8, 1), L4_rootclist);
    fail_unless(!success, "Create IPC Cap at in used slot succeeded");

    /* Delete the cap */
    success = L4_DeleteCap(L4_rootclist, L4_GlobalId(8, 1));
    fail_unless(success, "Delete cap failed");

}
END_TEST

/*
  \begin{test}{CAP1200}
  \TestDescription{Create an IPC Cap, source cap invalid (-1)}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap from an invalid source cap
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP1200)
{
    L4_Word_t success;

    /* Create cap at slot 8 (just to avoid conflicts).  */
    success = L4_CreateIpcCap(L4_GlobalId(-1,1), L4_rootclist, L4_GlobalId(8, 1), L4_rootclist);
    fail_unless(!success, "Create IPC Cap from invalid source cap succeeded");
}
END_TEST

/*
  \begin{test}{CAP1300}
  \TestDescription{Create an IPC Cap, source cap valid but non-existent}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap from a non-existent source cap
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP1300)
{
    L4_Word_t success;

    /* Create cap at slot 8 (just to avoid conflicts).
     * Assumes slot 16 in source is non-existent. */
    success = L4_CreateIpcCap(L4_GlobalId(16,1), L4_rootclist, L4_GlobalId(8, 1), L4_rootclist);
    fail_unless(!success, "Create IPC Cap from non-existent source cap succeeded");
}
END_TEST

/*
  \begin{test}{CAP1400}
  \TestDescription{Create an IPC Cap, source clist non-existent}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap from an invalid source clist
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP1400)
{
    L4_Word_t success;

    /* Create cap at slot 8 (just to avoid conflicts).
     * Assumes clist with id 15 is non-existent. */
    L4_ClistId_t clist = L4_ClistId(15);
    success = L4_CreateIpcCap(main_thread, clist, L4_GlobalId(8, 1), L4_rootclist);
    fail_unless(!success, "Create IPC Cap from non-existent source clist succeeded");
}
END_TEST

/*
  \begin{test}{CAP1500}
  \TestDescription{Create an IPC Cap, dest clist non-existent}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap to an invalid dest clist
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP1500)
{
    L4_Word_t success;

    /* Create cap at slot 8 (just to avoid conflicts).
     * Assumes clist with id 15 is non-existent. */
    L4_ClistId_t clist = L4_ClistId(15);
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(8, 1), clist);
    fail_unless(!success, "Create IPC Cap to invalid non-existent source clist succeeded");
}
END_TEST


/*********************************************************************
 *                                                                   *
 *                        CAP DELETION TESTS                         *
 *                                                                   *
 *********************************************************************/

/*
  \begin{test}{CAP2000}
  \TestDescription{Delete an IPC Cap, slot invalid}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to delete an ipc cap in an invalid slot
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP2000)
{
    L4_Word_t success;

    /* Delete cap at slot -1 */
    success = L4_DeleteCap(L4_rootclist, L4_GlobalId(-1, 1));
    fail_unless(!success, "Delete cap at slot -1 succeeded");

    /* Delete cap at slot max */
    success = L4_DeleteCap(L4_rootclist, L4_GlobalId(1024, 1));
    fail_unless(!success, "Delete cap at slot max succeeded");
}
END_TEST

/*
  \begin{test}{CAP2100}
  \TestDescription{Delete an IPC Cap, slot unused}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to delete an ipc cap in an unused slot
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP2100)
{
    L4_Word_t success;

    /* Delete cap at slot 200.
     * Obviously assumes slot 200 is unused. */
    success = L4_DeleteCap(L4_rootclist, L4_GlobalId(200, 1));
    fail_unless(!success, "Delete cap at unused slot 200 succeeded");

}
END_TEST

/*
  \begin{test}{CAP2200}
  \TestDescription{Delete an IPC Cap, invalid clist}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to delete an ipc cap from an invalid clist
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP2200)
{
    L4_Word_t success;

    /* Delete cap from non-existent clist
     * Assumes clist with id 15 is non-existent. */
    L4_ClistId_t clist = L4_ClistId(15);
    success = L4_DeleteCap(clist, L4_GlobalId(1, 1));
    fail_unless(!success, "Delete cap from unused clist succeeded");
}
END_TEST

/*
  \begin{test}{CAP2300}
  \TestDescription{Delete a master/threadcap}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create a thread and obtain its thread id as a cap
  \item Invoke \Func{CapControl} to delete the cap
  \item Check the return value 
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP2300)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t size = 0x100;
    L4_ThreadId_t thrd;

    L4_ClistId_t clist = L4_ClistId(1);
    L4_SpaceId_t space = L4_SpaceId(8);

    /* Create clist */
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create address space */
    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create the thread */
    thrd = allocateThread(space);
    success = create_thread_in_space(thrd, space, 0xb10000);
    fail_unless(success, "Create thread failed");

    /* Delete the thread cap */
    success = L4_DeleteCap(L4_rootclist, thrd);
    fail_unless(!success, "Delete thread cap from root clist succeeded");

    /* Delete the thread */
    deleteThread(thrd);

    /* Delete the space */
    success = delete_address_space(space);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the clist. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST


/*********************************************************************
 *                                                                   *
 *                    CAP THREAD DELETION TESTS                      *
 *                                                                   *
 *********************************************************************/

/*
  \begin{test}{CAP3000}
  \TestDescription{Delete thread, verify all IPC caps removed}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create a thread
  \item Invoke \Func{CapControl} create a number of IPC caps from the thread
  \item Delete the thread
  \item Attempt to create caps in the same slots in the same clist
  \item Check the return value; should succeed
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP3000)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t size = 0x100;
    L4_ThreadId_t thrd;
    int cap;

    L4_ClistId_t clist = L4_ClistId(1);
    L4_SpaceId_t space = L4_SpaceId(8);

    /* Create clist */
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create address space */
    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create the thread */
    thrd = allocateThread(space);
    success = create_thread_in_space(thrd, space, 0xb10000);
    fail_unless(success, "Create thread failed");

    /* Create caps at slots 20, 30, and 40, from this thread */
    for (cap = 20; cap < 50; cap += 10) {
        success = L4_CreateIpcCap(thrd, L4_rootclist, L4_GlobalId(cap, 1), clist);
        ErrorCode = L4_ErrorCode();
        _fail_unless(success, __FILE__, __LINE__, "Cap create failed at slot %d: ErrorCode=%"PRI_D_WORD"", cap, ErrorCode);
    }

    /* Delete the thread */
    deleteThread(thrd);

    /* Create caps again at slots 20, 30, and 40, from master thread */
    for (cap = 20; cap < 50; cap += 10) {
        success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(cap, 1), clist);
        ErrorCode = L4_ErrorCode();
        _fail_unless(success, __FILE__, __LINE__, "Cap create failed at previously used, but deleted, slot %d: ErrorCode=%"PRI_D_WORD"", cap, ErrorCode);
    }

    /* Delete the caps */
    for (cap = 20; cap < 50; cap += 10) {
        success = L4_DeleteCap(clist, L4_GlobalId(cap, 1));
        ErrorCode = L4_ErrorCode();
        fail_unless(success, "Delete cap failed");
    }

    /* Delete the space */
    success = delete_address_space(space);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the clist. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST

/*
  \begin{test}{CAP3100}
  \TestDescription{Delete thread, attempt to create new cap from the thread}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create a thread and obtain its thread id as a cap
  \item Delete the thread
  \item Invoke \Func{CapControl} to create an IPC cap from the deleted thread cap
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP3100)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t size = 0x100;
    L4_ThreadId_t thrd;

    L4_ClistId_t clist = L4_ClistId(1);
    L4_SpaceId_t space = L4_SpaceId(8);

    /* Create clist */
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create address space */
    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create the thread */
    thrd = allocateThread(space);
    success = create_thread_in_space(thrd, space, 0xb10000);
    fail_unless(success, "Create thread failed");

    /* Delete the thread */
    deleteThread(thrd);

    /* Create cap at slot 8 (just to avoid conflicts). */
    success = L4_CreateIpcCap(thrd, L4_rootclist, L4_GlobalId(8, 1), clist);
    fail_unless(!success, "Create cap from deleted thread cap succeeded");

    /* Delete the space */
    success = delete_address_space(space);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the clist. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

}
END_TEST


/*********************************************************************
 *                                                                   *
 *                       CAP PRIVILEGE TESTS                         *
 *                                                                   *
 *********************************************************************/

/* Used by all below tests */
L4_ThreadId_t main_alias;

/* Thread will attempt to call CapControl */
static void test_cap4000_thrd(void)
{
    //L4_KDB_Enter("here");
    L4_Word_t success, ErrorCode;
    L4_MsgTag_t tag;
    L4_Word_t size = 0x100;
    L4_ClistId_t clist = L4_ClistId(15);

    /* Create clist.
     * Assumes clist with id 15 doesn't already exist. */
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    fail_unless(!success, "Create clist from deprivileged thread succeeded");
    
    /* Alert the main thread that we're done.
     * We need to use the main_alias cap. */
    L4_LoadMR(0, 0);
    tag = L4_Send(main_thread);

    /* Main thread will kill us */
    while (1);
}

/*
  \begin{test}{CAP4000}
  \TestDescription{Ensure privileged threads only can call CapControl}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create a thread
  \item Have the thread call \Func{CapControl}
  \item Check the return value
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAP4000)
{
    L4_ThreadId_t tid;
    L4_SpaceId_t space;

    tid = createThreadInSpace(L4_nilspace, test_cap4000_thrd);
    space = lookupSpace(tid);

    L4_Receive(tid);

    deleteThread(tid);
    deleteSpace(space);
}
END_TEST

/*********************************************************************
 *                                                                   *
 *                          CAP IPC TESTS                            *
 *                                                                   *
 *********************************************************************/

static void test_capipc1000_thrd(void)
{
    L4_MsgTag_t tag;
    L4_ThreadId_t from;

    L4_LoadMR(0, 0);
    //printf("t - send startup\n");
    tag = L4_Send(main_thread);

    //printf("t - wait ipc\n");
    L4_Wait(&from);

    //printf("t - got somthing\n");
    tag = L4_Send(main_thread);
    while (1);
}

/*
\begin{test}{CAPIPC1000}
  \TestDescription{Create an IPC CAP, and try send IPC to it}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create ipc-cap
      \item Check the return value 
      \item Send IPC via the ipc cap
      \item Receiver does open wait
      \item Check the ipc succeeded
      \item Delete the cap
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CAPIPC1000)
{
    L4_ThreadId_t thrd, alias;
    L4_MsgTag_t tag;
    L4_Word_t slot = 8, success, ErrorCode;

    thrd = createThreadInSpace(L4_rootspace, test_capipc1000_thrd);
    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not start");

    alias =  L4_GlobalId(slot, 1);
    success = L4_CreateIpcCap(thrd, L4_rootclist, alias, L4_rootclist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

    L4_LoadMR(0, 0);
    tag = L4_Send(alias);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "IPC to alias failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not complete");

    deleteThread(thrd);
}
END_TEST

static void test_capipc1010_thrd(void)
{
    L4_MsgTag_t tag;

    L4_LoadMR(0, 0);
    //printf("t - send startup\n");
    tag = L4_Send(main_thread);

    //printf("t - wait ipc\n");
    L4_Receive(main_thread);

    //printf("t - got somthing\n");
    tag = L4_Send(main_thread);
    while (1);
}

/*
\begin{test}{CAPIPC1010}
  \TestDescription{Create an IPC CAP, and try send IPC to it}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create ipc-cap
      \item Check the return value 
      \item Send IPC via the ipc cap
      \item Check the ipc succeeded
      \item Receiver does closed wait
      \item Delete the cap
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CAPIPC1010)
{
    L4_ThreadId_t thrd, alias;
    L4_MsgTag_t tag;
    L4_Word_t slot = 8, success, ErrorCode;

    thrd = createThreadInSpace(L4_rootspace, test_capipc1010_thrd);
    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not start");

    alias =  L4_GlobalId(slot, 1);
    success = L4_CreateIpcCap(thrd, L4_rootclist, alias, L4_rootclist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

    L4_LoadMR(0, 0);
    tag = L4_Send(alias);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "IPC to alias failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not complete");

    deleteThread(thrd);
}
END_TEST

static void test_capipc1020_thrd(void)
{
    L4_MsgTag_t tag;

    L4_LoadMR(0, 0);
    //printf("t - send startup\n");
    tag = L4_Send(main_thread);

    //printf("t - wait ipc\n");
    L4_Receive(main_alias);

    //printf("t - send ipc\n");
    tag = L4_Send(main_thread);
    while (1);
}

/*
\begin{test}{CAPIPC1020}
  \TestDescription{Create an IPC CAP, and try receive IPC from it}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create ipc-cap at various slots
      \item Check the return value 
      \item Receive IPC via the ipc cap
      \item Check the ipc succeeded
      \item Delete the cap
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CAPIPC1020)
{
    L4_ThreadId_t thrd;
    L4_MsgTag_t tag;
    L4_Word_t slot = 8, success, ErrorCode;

    thrd = createThreadInSpace(L4_rootspace, test_capipc1020_thrd);
    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not start");

    main_alias =  L4_GlobalId(slot, 1);
    success = L4_CreateIpcCap(main_thread, L4_rootclist, main_alias, L4_rootclist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

    L4_LoadMR(0, 0);
    tag = L4_Send(thrd);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "IPC to alias failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not complete");

    deleteThread(thrd);
}
END_TEST


static void test_capipc1100_thrd(void)
{
    ARCH_THREAD_INIT

    L4_MsgTag_t tag;
    L4_Word_t ErrorCode;

    L4_LoadMR(0, 0);
    //printf("t - send startup\n");
    tag = L4_Send(main_alias);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "1 IPC to main failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    //printf("t - wait ipc\n");
    L4_Receive(main_alias);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "2 IPC to main failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    //printf("t - send ipc\n");
    tag = L4_Send(main_alias);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "3 IPC to main failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
    while (1);
}

/*
\begin{test}{CAPIPC1100}
  \TestDescription{IPC Send and Receive to thread in different clist}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Invoke \Func{CapControl} to create clist
      \item Check the return value 
      \item Invoke \Func{SpaceControl} to create space associated with clist
      \item Check the return value 
      \item Invoke \Func{CapControl} to create cap to main thread in new clist
      \item Check the return value 
      \item Invoke \Func{ThreadControl} to create thread in the space
      \item Check the return value 
      \item Send and Receive IPC messages via the new CAP
      \item Delete the thread
      \item Check the return value 
      \item Delete the cap
      \item Check the return value 
      \item Delete the space
      \item Check the return value 
      \item Delete the clist
      \item Check the return value 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(CAPIPC1100)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t size = 0x100;
    L4_ThreadId_t thrd;
    L4_MsgTag_t tag;

    L4_ClistId_t clist = L4_ClistId(1);
    L4_SpaceId_t space = L4_SpaceId(8);

    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    main_alias =  L4_GlobalId(12, 1);
    success = L4_CreateIpcCap(main_thread, L4_rootclist, main_alias, clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Create cap failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);

    L4_CreateIpcCap(L4_GlobalId(2,1), L4_rootclist, L4_GlobalId(2,1), clist);

    thrd = allocateThread(space);
    success = create_thread_in_space(thrd, space, 0xb10000);
    fail_unless(success, "Create thread failed");
    L4_Start_SpIp(thrd, stack_top, (L4_Word_t)test_capipc1100_thrd);
    //printf("M - wait startup\n");
    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not start");

    //printf("M - send IPC\n");
    L4_LoadMR(0, 0);
    tag = L4_Send(thrd);
    ErrorCode = L4_ErrorCode();
    _fail_unless(!L4_IpcFailed(tag), __FILE__, __LINE__, "IPC to thread failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    //printf("M - wait IPC\n");
    tag = L4_Receive(thrd);
    fail_unless(!L4_IpcFailed(tag), "thread did not complete");

    deleteThread(thrd);

    success = delete_address_space(space);
    fail_unless(success, "Delete space failed");

    success = L4_DeleteCap(clist, main_alias);
    fail_unless(success, "Delete IPC cap failed");

    success = L4_DeleteCap(clist, L4_GlobalId(2,1));
    fail_unless(success, "Delete IPC cap failed");

    success = L4_DeleteClist(clist);
    fail_unless(success, "Delete clist failed");
}
END_TEST

/*********************************************************************
 *                                                                   *
 *                      CAP ASYNC IPC TESTS                          *
 *                                                                   *
 *********************************************************************/

/* Just accepts AIPC and spins */
static void test_capaipc_thrd(void)
{
    ARCH_THREAD_INIT

    L4_Accept(L4_NotifyMsgAcceptor);
    while(1) {
        L4_ThreadSwitch(main_thread);
    }
}

/*
  \begin{test}{CAPIPC2000}
  \TestDescription{Async IPC to thread in same clist}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create a new thread in the rootspace
  \item Have the thread spin while accepting AIPCs
  \item Notify the thread
  \item Check the return value 
  \item Delete the thread
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPIPC2000)
{
    L4_ThreadId_t thrd;
    L4_MsgTag_t tag;

    /* Create thread in rootspace and allow it 10 timeslices to start */
    thrd = createThreadInSpace(L4_rootspace, test_capaipc_thrd);
    waitSending(thrd);

    /* Notify */
    tag = L4_Notify(thrd, 0x1);
    fail_unless(tag.raw == 0, "Incorrect result from notify");

    /* Clean up the thread */
    deleteThread(thrd);
}
END_TEST

/* Just accepts AIPC and spins */
static void test_capaipc_clist_thrd(void)
{
    ARCH_THREAD_INIT

    L4_Accept(L4_NotifyMsgAcceptor);
    while(1) {
        L4_ThreadSwitch(main_alias);
    }
}

/*
  \begin{test}{CAPIPC2100}
  \TestDescription{Async IPC to thread in different clist}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Create a new clist, space, and thread
  \item Have the thread spin while accepting AIPCs
  \item Notify the thread
  \item Check the return value 
  \item Delete the thread, space, and clist
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPIPC2100)
{
    L4_Word_t success, ErrorCode;
    L4_Word_t size = 0x100;
    L4_ThreadId_t thrd;
    L4_MsgTag_t tag;

    L4_ClistId_t clist = L4_ClistId(1);
    L4_SpaceId_t space = L4_SpaceId(8);

    main_alias =  L4_GlobalId(10, 1);

    /* Create clist */
    success = L4_CreateClist(clist, size);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Clist create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create an ipc cap to the main thread in this clist.
     * Create in slot 10; this will be picked up by the created thread. */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, main_alias, clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Create IPC cap failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create address space */
    success = create_address_space(space, clist, L4_Fpage(0xb10000, 0x1000));
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space create failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Create a cap to the thread's pager.
     * Without this the thread won't start. */
    L4_CreateIpcCap(L4_GlobalId(2,1), L4_rootclist, L4_GlobalId(2,1), clist);

    /* Create and start the thread.
     * Wait 10 timeslices for it to start. */
    thrd = allocateThread(space);
    success = create_thread_in_space(thrd, space, 0xb10000);
    fail_unless(success, "Create thread failed");
    L4_Start_SpIp(thrd, stack_top, (L4_Word_t)test_capaipc_clist_thrd);
    waitSending(thrd);

    /* Notify */
    tag = L4_Notify(thrd, 0x1);
    fail_unless(tag.raw == 0, "Incorrect result from notify");

    /* Delete the thread */
    deleteThread(thrd);

    /* Delete the caps */
    success = L4_DeleteCap(clist, main_alias);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Cap delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);
    success = L4_DeleteCap(clist, L4_GlobalId(2, 1));
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Cap delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the space */
    success = delete_address_space(space);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Space delete failed: ErrorCode=%"PRI_D_WORD"", ErrorCode);

    /* Delete the clist. */
    success = L4_DeleteClist(clist);
    ErrorCode = L4_ErrorCode();
    _fail_unless(success, __FILE__, __LINE__, "Delete clist failed : ErrorCode=%"PRI_D_WORD"", ErrorCode);
}
END_TEST

/*********************************************************************
 *                                                                   *
 *                      CAP THREAD SWITCH TESTS                      *
 *                                                                   *
 *********************************************************************/

/*
  \begin{test}{CAPTS0100}
  \TestDescription{Create an IPC Cap of main thread tid in a valid slot and ThreadSwitch to the IPC CAP}
  \TestFunctionalityTested{\Func{CapControl}}
  \TestImplementationProcess{
  \begin{enumerate}
  \item Invoke \Func{CapControl} to create an ipc cap of main thread in a slot
  \item Check the return value 
  \item Switch to myself by calling \Func{L4\_Schedule} with the new ipc cap
  \item Switch to myself by calling \Func{L4\_Schedule} with main_thread tid
  \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{Not yet in regression test suite}
  \TestIsFullyAutomated{Yes}
  \end{test}
*/
START_TEST(CAPTS0100)
{
    L4_Word_t success;
    int i;

    /* Create at slot 8 */
    success = L4_CreateIpcCap(main_thread, L4_rootclist, L4_GlobalId(8, 1), L4_rootclist);
    fail_unless(success, "Create IPC Cap at slot 8 failed");

    for (i = 0; i < 20; i++) {
        L4_ThreadSwitch(L4_GlobalId(8, 1));
    }

    for (i = 0; i < 20; i++) {
        L4_ThreadSwitch(main_thread);
    }

    /* Delete the cap */
    success = L4_DeleteCap(L4_rootclist, L4_GlobalId(8, 1));
    fail_unless(success, "Delete cap failed");
}
END_TEST


/*********************************************************************
 *                                                                   *
 *                               MAIN                                *
 *                                                                   *
 *********************************************************************/

extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    main_thread = test_tid;
}


static void test_teardown(void)
{

}

TCase *
make_caps_tcase(void)
{
    TCase *tc;

    initThreads(0);

    tc = tcase_create("Capability Tests");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    /* Clist create tests */
    tcase_add_test(tc, CLIST0100);
    tcase_add_test(tc, CLIST0200);
    tcase_add_test(tc, CLIST1000);
    tcase_add_test(tc, CLIST1100);
    tcase_add_test(tc, CLIST1200);

    /* Clist delete tests */
    tcase_add_test(tc, CLIST2000);
    tcase_add_test(tc, CLIST2100);
    tcase_add_test(tc, CLIST2200);
    tcase_add_test(tc, CLIST2300);
    tcase_add_test(tc, CLIST2400);

    /* Space create tests */
    tcase_add_test(tc, CAPSPACE1000);
    tcase_add_test(tc, CAPSPACE1100);
    tcase_add_test(tc, CAPSPACE1200);

    /* Cap create tests */
    tcase_add_test(tc, CAP0100);
    tcase_add_test(tc, CAP0110);
    tcase_add_test(tc, CAP1000);
    tcase_add_test(tc, CAP1100);
    tcase_add_test(tc, CAP1200);
    tcase_add_test(tc, CAP1300);
    tcase_add_test(tc, CAP1400);
    tcase_add_test(tc, CAP1500);

    /* Cap delete tests */
    tcase_add_test(tc, CAP2000);
    tcase_add_test(tc, CAP2100);
    tcase_add_test(tc, CAP2200);
    tcase_add_test(tc, CAP2300);

    /* Cap thread delete tests */
    tcase_add_test(tc, CAP3000);
    tcase_add_test(tc, CAP3100);

    /* CapControl tests */
    tcase_add_test(tc, CAP4000);

    /* Ipc tests */
    tcase_add_test(tc, CAPIPC1000);
    tcase_add_test(tc, CAPIPC1010);
    tcase_add_test(tc, CAPIPC1020);
    tcase_add_test(tc, CAPIPC1100);

    /* Async Ipc tests */
    tcase_add_test(tc, CAPIPC2000);
    tcase_add_test(tc, CAPIPC2100);

    /* Cap thread switch tests */
    tcase_add_test(tc, CAPTS0100);

    return tc;
}
