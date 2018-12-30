/*
 * Copyright (c) 2006, National ICT Australia
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
 * Author: Philip O'Sullivan <philip.osullivan@nicta.com.au>
 * Date  : Tue 11 Apr 2006
 */

#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <l4/map.h>
#include <l4/space.h>
#include <l4/thread.h>
#include <l4/config.h>

#include <l4e/map.h>

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include <l4test/l4test.h>
#include <l4test/utility.h>
#include <l4test/arch/constants.h>

/* Defines -----------------------------------------------------------------*/

#define BASE_PAGE_SIZE 0x1000

#if defined(L4_32BIT)
# define PERM_GET_D(perm) (!!((perm) & 0x80000000))
#elif defined(L4_64BIT)
# define PERM_GET_D(perm) (!!((perm) & 0x8000000000000000))
#endif
#define PERM_GET_RWX(perm) (((perm) & 0x70) >> 4)
#define PERM_GET_rwx(perm) ((perm) & 0x7)

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

/* Some utility functions want to check a condition and if the
 * condition fails return an error. */
#define FAIL_UNLESS(cond, desc) fail_unless((cond), (desc)); \
                                if (!(cond)) { return 1; }
#define FAIL_IF(cond, desc) fail_if((cond), (desc)); \
                            if ((cond)) { return 1; }


/* Constants ---------------------------------------------------------------*/

const uintptr_t vbase = 8UL<<20; /* 8MB so that it is aligned for 2MB pages */
const uintptr_t pbase = 0xe0000;

L4_SpaceId_t test_space;


/* Prototypes --------------------------------------------------------------*/

static int
check_mapping(L4_SpaceId_t space, L4_Fpage_t vpage, int exists, unsigned rwx,
              uintptr_t pbase, unsigned attr);

/* Utility functions -------------------------------------------------------*/

/**
 * Check a mappings existence and if it exists that it is valid mapping.
 *
 * @param[in] space The address space to check in
 * @param[in] vpage The mapping to check
 * @param[in] exists Is the mapping expected to exist
 * @param[in] rwx The expected permissions, only valid if exists is true
 * @param[in] attr The expected attributes, only valid if exists is true
 *
 * @return 0 for success and non-zero for failure
 */
static int
check_mapping(L4_SpaceId_t space, L4_Fpage_t vpage, int exists, unsigned rwx,
              uintptr_t pbase, unsigned attr)
{
    L4_PhysDesc_t p;
    L4_MapItem_t m;
    int res;

    if (attr == L4_DefaultMemory) {
        attr = default_attr;
    }

    res = L4_ReadFpage(space, vpage, &p, &m);

    FAIL_UNLESS(res == 1, "L4_MapControl(..., L4_MapCtrl_Query) failed");

    if (exists) {
        /* There should be a valid mapping */
        FAIL_UNLESS(m.X.size != 0, "There should be a valid mapping")
        FAIL_UNLESS(m.X.size == vpage.X.s, "Incorrect mapping size")
        FAIL_UNLESS(m.X.rwx == rwx, "Incorrect permissions read");
        /* FIXME check RWX perms */
        FAIL_UNLESS(p.X.attr == attr, "Incorrect attributes read");
        FAIL_UNLESS(p.X.base == (pbase>>10), "Incorrect phys base read");
    } else {
        FAIL_UNLESS(m.X.size == 0, "Mapping found when no mapping was expected");
    }

    return 0;
}
#define check_no_mapping(space, vpage) check_mapping((space), (vpage), 0, 0, 0UL, 0)

/**
 * Setup a series of mappings.
 *
 * @param[in] page_size the page size to use for all the mappings
 * @param[in] n_pages how many pages to use in the sequence
 * @param[in] v the virtual address to start the sequence of mappings
 * @param[in] p the physical address to start the sequence of mappings
 *
 * @return 0 for success and non-zero on error
 */
static int
map_series(L4_SpaceId_t space, L4_Word_t page_size, uintptr_t n_pages, uintptr_t v, uintptr_t p)
{
    int i, res;
    L4_Fpage_t f;

    for (i = 0; i < n_pages; i++, p += page_size) {
        f = L4_Fpage(v + i*page_size, page_size);
        f.X.rwx = L4_FullyAccessible;
        res = l4e_map_fpage(space, f, p, L4_DefaultMemory);
        FAIL_UNLESS(res == 1, "l4e_map failed");
    }

    return 0;
}

/**
 * Test mapping two equal sized regions of virtual memory with series of
 * fpages of different sizes.  The regions of virtual memory may overlap
 * completely or not at all.
 *
 * @param[in] a page size of the mappings in the first virtual region
 * @param[in] av virtual address of the first virtual region
 * @param[in] b page size of the mappings in the second virtual region
 * @param[in] bv virtual address of the second virtual region
 * @param[in] check_unmap if true check the second mapping after deleting the
 * first
 */
static void
map_region_test(L4_SpaceId_t space, L4_Word_t a, uintptr_t av, L4_Word_t b, uintptr_t bv, int check_unmap)
{
    uintptr_t an = 1, bn = 1;
    int i, error = 0;
    uintptr_t map_pbase;

    map_pbase = (pbase + a-1) & (~(a-1));
    map_pbase = (map_pbase + b-1) & (~(b-1));

    assert(a != b);
    if (a < b) {
        an = b / a;
    } else if (b < a) {
        bn = a / b;
    }

    error |= map_series(space, a, an, av, map_pbase);
    error |= map_series(space, b, bn, bv, map_pbase);
    if (error) { goto cleanup; }

    if (av != bv) {
        for (i = 0; i < an; i++) {
            error |= check_mapping(space, L4_Fpage(av + i*a, a), 1, L4_FullyAccessible, map_pbase + i*a, L4_DefaultMemory);
        }
    }
    for (i = 0; i < bn; i++) {
        error |= check_mapping(space, L4_Fpage(bv + i*b, b), 1, L4_FullyAccessible, map_pbase + i*b, L4_DefaultMemory);
    }
    if (error) { goto cleanup; }

    /* FIXME check that writing to one set of mappings is visible in the other */

    if (check_unmap) {
        /* Check that we can still read the second set of mappings after
         * deleting the first */
        l4e_unmap(space, av, av + a*an);
        for (i = 0; i < bn; i++) {
            check_mapping(space, L4_Fpage(bv + i*b, b), 1, L4_FullyAccessible, map_pbase + i*b, L4_DefaultMemory);
        }
    }

cleanup:
    l4e_unmap(space, av, av + a*an);
    l4e_unmap(space, bv, bv + b*bn);
}

/* Space Ids */

extern L4_Word_t kernel_max_spaces;

/* Invalid SpaceSpecifier tests --------------------------------------------*/

/*
\begin{test}{MAP0100}
  \TestDescription{Verify MapControl checks SpaceSpecifier for L4\_nilspace}
  \TestFunctionalityTested{\Func{SpaceSpecifier}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{MapControl} with \Func{SpaceSpecifier} = \Func{L4\_nilspace} 
      \item Check that \Func{MapControl} returns an error in result
      \item Check the \Func{ErrorCode} [TCR] for invalid space
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0100)
{
    int res;

    res = L4_MapControl(L4_nilspace, L4_MapCtrl_Query);
    fail_unless(res == 0, "MapControl succeded on Nilspace");
    fail_unless(__L4_TCR_ErrorCode() == 3, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST

/*
\begin{test}{MAP0101}
  \TestDescription{Verify MapControl checks SpaceSpecifier for MaxSpaces}
  \TestFunctionalityTested{\Func{SpaceSpecifier}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{MapControl} with \Func{SpaceSpecifier} = \Func{L4\_MaxSpace+1} 
      \item Check that \Func{MapControl} returns an error in result
      \item Check the \Func{ErrorCode} [TCR] for invalid space
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/

START_TEST(MAP0101)
{
    int res;
    L4_SpaceId_t space;

    space.raw = kernel_max_spaces - 1;
    space.space_no ++;

    res = L4_MapControl(space, L4_MapCtrl_Query);
    fail_unless(res == 0, "MapControl succeded on Anythread");
    fail_unless(__L4_TCR_ErrorCode() == 3, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST

/*
\begin{test}{MAP0102}
  \TestDescription{Verify MapControl checks validity of SpaceSpecifier}
  \TestFunctionalityTested{\Func{SpaceSpecifier}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{MapControl} with the \Func{SpaceSpecifier} set to non-existent space id 
      \item Check that \Func{MapControl} returns an error in \Func{result}
      \item Check the \Func{ErrorCode} [TCR] for invalid space
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/

START_TEST(MAP0102)
{
    int res;

    L4_SpaceId_t space;

    space.raw = kernel_max_spaces - 1;
    space.space_no -= 5;

    res = L4_MapControl(space, L4_MapCtrl_Query);
    fail_unless(res == 0, "MapControl succeded on non existent thread");
    fail_unless(__L4_TCR_ErrorCode() == 3, "L4 Error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST


/* Invalid Fpage_t tests ---------------------------------------------------*/

/*
\begin{test}{MAP0200}
  \TestDescription{Verify MapControl does not map L4\_Nilpage}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{MapControl} with a \Func{Fpage} in a \Func{MapItem} to be mapped set to \Func{L4\_Nilpage}  
      \item Check that \Func{MapControl} returns successfully
      \item Check that no page at virtual address zero has been mapped
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0200)
{
    L4_Fpage_t f;
    int res;

    res = l4e_map_fpage(test_space, L4_Nilpage, pbase, L4_DefaultMemory);
    fail_unless(res == 1, "l4e_map_fpage failed");

    check_no_mapping(test_space, L4_Nilpage);
    f = L4_Fpage(0UL, BASE_PAGE_SIZE);
    check_no_mapping(test_space, f);
}
END_TEST

/*
\begin{test}{MAP0201}
  \TestDescription{Verify MapControl does not map L4\_CompleteAddressSpace}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{MapControl} with a \Func{Fpage} in a \Func{MapItem} to be mapped set to \Func{L4\_CompleteAddressSpace} 
      \item Check that \Func{MapControl} returns successfully
      \item Check that no page at virtual address zero has been mapped
    \end{enumerate}
  }
  \TestImplementationStatus{Implemeted}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0201)
{
    L4_Fpage_t f;
    int res;

    res = l4e_map_fpage(test_space, L4_CompleteAddressSpace, pbase, L4_DefaultMemory);
    fail_unless(res == 1, "l4e_map_fpage failed");

    check_no_mapping(test_space, L4_CompleteAddressSpace);
    f = L4_Fpage(0UL, BASE_PAGE_SIZE);
    check_no_mapping(test_space, f);
}
END_TEST

/*
\begin{test}{MAP0202}
  \TestDescription{Verify MapControl does not map a Fpage with $1 < s < 10$}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Call \Func{MapControl} with a \Func{MapItem} to be mapped containing a \Func{Fpage} with $s = 8$ 
          \item Check that \Func{MapControl} fails successfully
      \item Check that no page at the virtual address in the \Func{Fpage} has been mapped
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0202)
{
    L4_Fpage_t f;
    int res;

    f = L4_FpageLog2(vbase, 8);
    f.X.rwx = L4_FullyAccessible;
    res = l4e_map_fpage(test_space, f, pbase, L4_DefaultMemory);
    fail_unless(res != 1, "l4e_map_fpage did not fail");

    check_no_mapping(test_space, f);
    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    check_no_mapping(test_space, f);
}
END_TEST

/*
\begin{test}{MAP0203}
  \TestDescription{Verify MapControl handles a Fpage with an address not aligned to its size}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Construct a \Func{Fpage} with an address that is not aligned with the size of the mapping
      \item Call \Func{MapControl} with the \Func{Fpage} in a \Func{MapItem}
      \item Check that \Func{MapControl} fails successfully
      \item Check that a valid mapping does not exists for the \Func{Fpage}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0203)
{
    L4_Fpage_t f;
    int res;

    f.X.b = (vbase + 0x800) >> 10;
    f.X.s = 12;
    f.X.rwx = L4_FullyAccessible;
    res = l4e_map_fpage(test_space, f, pbase, L4_DefaultMemory);
    fail_unless(res != 1, "l4e_map_fpage did not fail");

    check_no_mapping(test_space, f);
}
END_TEST

#if defined(L4_32BIT)
/*
\begin{test}{MAP0204}
  \TestDescription{Verify MapControl handles a Fpage with a size greater than the address space}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestNotes{This test is only valid on 32 bit architectures as the size of a
  \Func{Fpage} can not be greater than the size of a 64 bit address space}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Construct a \Func{Fpage} with $s > 32$
      \item Call \Func{MapControl} with the \Func{Fpage} in a \Func{MapItem} to be mapped
      \item Check that \Func{MapControl} returns successfully
      \item Check that there is no mapping at address 0
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0204)
{
    L4_Fpage_t f;
    L4_Word_t a, max_size;
    int res;

    max_size = 0;

    for (a = 0x400; a < (1UL << (L4_BITS_PER_WORD-1)); a <<= 1) {
        if (!(a & L4_GetPageMask()))
            continue;

        max_size = a;
    }

    f = L4_FpageLog2(0UL, 33);
    f.X.rwx = L4_FullyAccessible;
    res = l4e_map_fpage(test_space, f, 0, L4_DefaultMemory);
    fail_unless(res == 1, "l4e_map_fpage failed");
    check_mapping(test_space, L4_Fpage(0x20000000, max_size), 1, L4_FullyAccessible, 0x20000000, L4_DefaultMemory);
    check_mapping(test_space, L4_Fpage(0x70000000, max_size), 1, L4_FullyAccessible, 0x70000000, L4_DefaultMemory);
    // MIPS32 address space ends at 0x80000000
#if !(defined(L4_ARCH_MIPS) && defined(L4_32BIT))
    check_mapping(test_space, L4_Fpage(0x90000000, max_size), 1, L4_FullyAccessible, 0x90000000, L4_DefaultMemory);
#endif
    l4e_unmap_fpage(test_space, f);

    check_no_mapping(test_space, L4_Fpage(0x20000000, max_size));
    check_no_mapping(test_space, L4_Fpage(0x70000000, max_size));
#if !(defined(L4_ARCH_MIPS) && defined(L4_32BIT))
    check_no_mapping(test_space, L4_Fpage(0x90000000, max_size));
#endif
}
END_TEST
#endif


/* Invalid PhysDesc_t tests ------------------------------------------------*/

/*
\begin{test}{MAP0300}
  \TestDescription{Verify MapControl handles a PhysDesc with an invalid attr}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Construct a \Func{PhysDesc} with an $attr > 7$
      \item Call \Func{MapControl} with the \Func{PhysDesc} in a \Func{MapItem} to be mapped
      \item Check \Func{MapControl} returns successfully
      \item Check that there is no mapping for the \Func{Fpage}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0300)
{
    L4_Fpage_t f;
    int res;

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    res = l4e_map_fpage(test_space, f, pbase, 0x13);
    fail_unless(res == 1, "l4e_map_fpage failed");

    check_no_mapping(test_space, f);
}
END_TEST

/*
\begin{test}{MAP0301}
  \TestDescription{Verify MapControl handles a PhysDesc with a misaligned address}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Construct a \Func{PhysDesc} with an address such that the address is not aligned with the size of the Fpage used
      \item Call \Func{MapControl} with the \Func{PhysDesc} and \Func{Fpage} in a \Func{MapItem} to be mapped
      \item Check that \Func{MapControl} returned successfully
      \item Check that there is no mapping for the given \Func{Fpage}
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0301)
{
    L4_Fpage_t f;
    int res;

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    res = l4e_map_fpage(test_space, f, pbase + 0x1000, L4_DefaultMemory);
    fail_unless(res == 1, "l4e_map_fpage failed");

    check_no_mapping(test_space, f);
}
END_TEST


/* Invalid Number of MapItems test -----------------------------------------*/

#if MAX_MAP_ITEMS < 32
/*
\begin{test}{MAP0400}
  \TestDescription{Verify MapControl handles large $n$}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestNotes{This test is only valid on architectures with less than 64 message registers}
  \TestImplementationProcess{
    \begin{enumerate}
      \item Construct a control word where n = 63
      \item Call \Func{MapControl} with the control word
      \item Check \Func{MapControl} returned with an error in result
      \item Check the \Func{ErrorCode}d [TCR] for invalid parameter
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0400)
{
    int res;

    res = L4_MapControl(test_space, L4_MapCtrl_Query | 0x3f);
    fail_unless(res == 0, "MapControl succeded with n = 63");
    fail_unless(__L4_TCR_ErrorCode() == 5, "L4 error code is incorrect");
    CLEAR_ERROR_CODE;
}
END_TEST
#endif


/* Valid mappings tests ----------------------------------------------------*/

/**
 * Setup the message registers with a sequence of MapItems.
 *
 * @param[in] new_vbase The virtual address to start the sequence of Fpages
 * @param[in] new_pbase The physical address to start the sequence of PhysDesc
 * @param[in] mapitems The number of MapItems in the sequence
 */
static void
setup_mrs(uintptr_t new_vbase, uintptr_t new_pbase, int mapitems)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p;
    int i;

    for (i = 0; i < mapitems; i++) {
        f = L4_Fpage(new_vbase + i*BASE_PAGE_SIZE, BASE_PAGE_SIZE);
        f.X.rwx = L4_FullyAccessible;
        p = L4_PhysDesc(new_pbase + i*BASE_PAGE_SIZE, L4_DefaultMemory);

        L4_LoadMR(2*i, p.raw);
        L4_LoadMR(2*i + 1, f.raw);
    }
}

/**
 * Test framework for checking mapping of valid MapItems.
 *
 * @param[in] space The space in which MapControl will be tested
 * @param[in] mapitems The number of mapitems to be used
 * @param[in] rm_flag The read/modify flag
 *
 * @return 0 for success and non-zero for failure
 */
static int
general_mapping_test(L4_SpaceId_t space, int mapitems, L4_Word_t rm_flag)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p;
    L4_MapItem_t m;
    int i, res;

    //printf("%s: space=%lx n=%d rm_flag=%"PRIxPTR"\n", __func__, space.raw, n, rm_flag);
    /* If READ: setup the first set of mappings to read */
    if (rm_flag & L4_MapCtrl_Query) {
        setup_mrs(vbase, pbase, mapitems);
        res = L4_MapControl(space, L4_MapCtrl_Modify | (mapitems - 1));
        fail_unless(res == 1, "MapControl failed");
        if (res == 0) {
            printf("MapControl FAILED: error=%"PRI_D_WORD"\n", __L4_TCR_ErrorCode());
            return 1;
        }
    }

    /* Setup the mappings to be tested */
    setup_mrs(vbase, 2*pbase, mapitems);

    res = L4_MapControl(space, rm_flag | (mapitems - 1));
    fail_unless(res == 1, "MapControl failed");
    if (res == 0) {
        return 2;
    }

    /* If READ: check the QueryItems */
    if (rm_flag & L4_MapCtrl_Query) {
        for (i = 0; i < mapitems; i++) {
            L4_StoreMR(2*i, &p.raw);
            L4_StoreMR(2*i+1, &m.raw);

            fail_unless(m.X.size != 0, "QueryItem is invalid");
            if (m.X.size == 0) {
                return 3;
            }
            /* FIXME check the permissions */
            fail_unless(p.X.base << 10 == pbase + i*BASE_PAGE_SIZE, "incorrect base in QueryItem");
        }
    }

    /* If MODIFY: check the mappings */
    if (rm_flag & L4_MapCtrl_Modify) {
        for (i = 0; i < mapitems; i++) {
            f = L4_Fpage(vbase + i*BASE_PAGE_SIZE, BASE_PAGE_SIZE);
            check_mapping(space, f, 1, L4_FullyAccessible, 2*pbase + i*BASE_PAGE_SIZE, L4_DefaultMemory);
        }
    }

    l4e_unmap(space, vbase, vbase + mapitems*BASE_PAGE_SIZE);

    return 0;
}

/**
 * Run the general_mapping tester with the given Read/Modify flag.
 *
 * @param[in] rm_flag flag for read/modify using L4_MapCtrl_Query and L4_MapCtrl_Modify
 */
static void
rm_mapping_tester(L4_Word_t rm_flag)
{
    L4_SpaceId_t space, spaces[2];
    int mapitems, n_mapitems[] = { 1, 2, MAX_MAP_ITEMS - 1, MAX_MAP_ITEMS };
    int res, i, j;

    spaces[0] = L4_rootspace;
    spaces[1] = test_space;

    for (i = 0; i < ARRAY_SIZE(spaces); i++) {
        space = spaces[i];
        for (j = 0; j < ARRAY_SIZE(n_mapitems); j++) {
            mapitems = n_mapitems[j];
            res = general_mapping_test(space, mapitems, rm_flag);
            fail_unless(res == 0, "general_mapping_test failed");
            if (res != 0) {
                printf("%s: FAIL: res=%d space=%"PRI_X_WORD" mapitems=%d rm_flag=%"PRI_X_WORD"\n",
                       __func__, res, space.raw, mapitems, rm_flag);
            }
        }
    }
}

/*
\begin{test}{MAP0500}
  \TestDescription{Verify MapControl reads valid mappings correctly}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item For all combinations of:
        \begin{enumerate}
                \item $SpaceSpecifier$ values:
                \begin{enumerate}
                        \item \Func{L4\_Myself()}
                        \item space different from the roottask
                \end{enumerate}
                \item $n$ values:
                \begin{enumerate}
                        \item 0
                        \item 1
                        \item \Func{MAX\_MAP\_ITEMS} - 2
                        \item \Func{MAX\_MAP\_ITEMS} - 1
                \end{enumerate}
                \item $r,m$ values:
                \begin{enumerate}
                        \item \Func{L4\_MapCtrl\_Read}
                        \item \Func{L4\_MapCtrl\_Modify}
                        \item \Func{L4\_MapCtrl\_Read} $|$ \Func{L4\_MapCtrl\_Modify}
                \end{enumerate}
        \end{enumerate}
        \item Setup message registers with a set of valid \Func{MapItems} and use \Func{MapControl} to map them
        \item Setup message registers with the same set of \Func{MapItems} and use \Func{MapControl} to read them
        \item Check \Func{QueryItems} returned have valid mappings and that addresses, permissions and attributes are correct
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0500)
{
    rm_mapping_tester(L4_MapCtrl_Query);
}
END_TEST

/*
\begin{test}{MAP0501}
  \TestDescription{Verify MapControl maps valid mappings correctly}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item For all combinations of:
        \begin{enumerate}
                \item $SpaceSpecifier$ values:
                \begin{enumerate}
                        \item \Func{L4\_Myself()}
                        \item space different from the roottask
                \end{enumerate}
                \item $n$ values:
                \begin{enumerate}
                        \item 0
                        \item 1
                        \item \Func{MAX\_MAP\_ITEMS} - 2
                        \item \Func{MAX\_MAP\_ITEMS} - 1
                \end{enumerate}
                \item $r,m$ values:
                \begin{enumerate}
						\item \Func{L4\_MapCtrl\_Read}
                        \item \Func{L4\_MapCtrl\_Modify}
						\item \Func{L4\_MapCtrl\_Read} $|$ \Func{L4\_MapCtrl\_Modify}
                \end{enumerate}
        \end{enumerate}
        \item Setup the message registers with a set of valid \Func{MapItems} and use
          \Func{MapControl} to map them
        \item Check the mappings have been set correctly, by checking the addresses,
          permissions and attributes are correct
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0501)
{
    rm_mapping_tester(L4_MapCtrl_Modify);
}
END_TEST

/*
\begin{test}{MAP0502}
  \TestDescription{Verify MapControl reads and maps valid mappings correctly}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
	    For all combinations of:
			\begin{enumerate}
                \item $SpaceSpecifier$ values:
                \begin{enumerate}
                        \item \Func{L4\_Myself()}
                        \item space different from the roottask
                \end{enumerate}
                \item $n$ values:
                \begin{enumerate}
                        \item 0
                        \item 1
                        \item \Func{MAX\_MAP\_ITEMS} - 2
                        \item \Func{MAX\_MAP\_ITEMS} - 1
                \end{enumerate}
                \item $r, m$ values:
                \begin{enumerate}
                        \item \Func{L4\_MapCtrl\_Read}
                        \item \Func{L4\_MapCtrl\_Modify}
                        \item \Func{L4\_MapCtrl\_Read} $|$ \Func{L4\_MapCtrl\_Modify}
                \end{enumerate}
        \item Setup the message registers with a set of valid \Func{MapItems} and use
          \Func{MapControl} to map them
        \item Setup the message registers with a set of valid \Func{MapItems}, with the
          same virtual address but different physical addresses, and use \Func{MapControl}
          to map and read them
        \item Check the \Func{QueryItems} returned have valid mappings and that the
          addresses, permissions and attributes are correct
        \item Check the mappings have been set correctly, by checking the addresses,
          permissions and attributes are correct
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0502)
{
    rm_mapping_tester(L4_MapCtrl_Query | L4_MapCtrl_Modify);
}
END_TEST

/*
\begin{test}{MAP0503}
  \TestDescription{Verify MapControl correctly maps each valid hardware page size}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
   For each valid page size listed in the \Func{page\_size\_mask} in the KIP:
        \begin{enumerate}
                \item Construct valid MapItem and use \Func{MapControl} to map it
                \item Check the mapping is valid
        \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0503)
{
    L4_Word_t a;
    L4_Fpage_t f;
    L4_Word_t map_vbase, map_pbase;


    for (a = 0x400; a < ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)); a <<= 1) {
        if (!(a & L4_GetPageMask()))
            continue;

        map_vbase = (vbase + a) & (~(a-1));
        map_pbase = (pbase + a) & (~(a-1));

        f = L4_Fpage(map_vbase, a);
        f.X.rwx = L4_FullyAccessible;
        l4e_map_fpage(test_space, f, map_pbase, L4_DefaultMemory);
        check_mapping(test_space, f, 1, L4_FullyAccessible, map_pbase, L4_DefaultMemory);
        l4e_unmap_fpage(test_space, f);
    }
}
END_TEST


/* Dual mapping tests -------------------------------------------------------*/

/*
\begin{test}{MAP0600}
  \TestDescription{Verify MapControl can map same physical address to multiple virtual addresses}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
        For each valid pair of page sizes, with sizes taken from \Func{page\_size\_mask} in the KIP:
        \begin{enumerate}
                \item For the smaller page size map a sequence of virtual pages that has the same size as the larger mapping
                \item Map a single larger page with the same physical address at a different virtual address 
                \item Check that the mappings are valid
                \item Check that writes to one mapping are visible on the other mapping
        \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0600)
{
    L4_Word_t a, b, an=1;
    L4_Word_t map_base;

    for (a = 0x400; a < ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)); a <<= 1) {
        if (!(a & L4_GetPageMask()))
            continue;

        for (b = 0x400; b < ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)); b <<= 1) {
            if (!(b & L4_GetPageMask()))
                continue;
            if (a == b)
                continue;
            if (a < b)
                an = b / a;
            else
                an = 1;

            /* correctly align up bases */
            map_base = (vbase + a-1) & (~(a-1));
            map_base = (map_base + b-1) & (~(b-1));

            map_region_test(test_space, a, map_base, b, map_base+(a*an), 1);
        }
    }
}
END_TEST


#define BITS_WORD 32
inline L4_Word_t msb(L4_Word_t w);
inline 
L4_Word_t msb(L4_Word_t w)
{
    L4_Word_t bit = BITS_WORD - 1;
    L4_Word_t test = 1ul << (BITS_WORD-1);

    while (!(w & test)) {
        w <<= 1;
        bit --;
    }
    return bit;
}

extern L4_Word_t kernel_phys_base;
extern L4_Word_t kernel_phys_end;

#define MAX_PAGES 128
/*
\begin{test}{MAP0601}
  \TestDescription{Verify Unmapping flushes the TLB}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
        Find the biggest free physical memory block.
        For each valid page size, with size taken from \Func{page\_size\_mask} in the KIP:
        \begin{enumerate}
                \item Map as many pages as possible given the size of the free physical memory
                \item Write data to each page
                \item Unmap all the pages
                \item Map same number of pages with different physical addressses at the same virtual addresses
                \item Check data are different from data written during the first mapping
                \item Write different data to each page
                \item Unmap all the pages
                \item Map original pages at the same virtual addresses
                \item Check for original data
        \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0601)
{
    L4_Word_t page_size, pm_size;
    L4_Word_t nb_pages;
    L4_Fpage_t fpages[MAX_PAGES];
    L4_PhysDesc_t physmems[MAX_PAGES];
    L4_Word_t v_base, other_pm_base;
    L4_Word_t pm_base;
    L4_Word_t res;
    L4_Word_t page_size_mask;
    volatile L4_Word_t *w, *r;
    int i;
    
    page_size_mask = L4_GetPageMask();
    pm_size = kernel_phys_end - kernel_phys_base;
    pm_base = kernel_phys_base;
    /* only use biggest page size supported by free memory */
    while((2*(1UL<<msb(page_size_mask))) > pm_size)
        page_size_mask &= ~(1UL << msb(page_size_mask));
    pm_base = ROUND_UP(pm_base, 1UL << msb(page_size_mask));
    v_base = ROUND_UP(vbase, 1UL << msb(page_size_mask));
    //printf("mask %lx, msb %lx, 1<<msb %lx\n", page_size_mask, msb(page_size_mask), 1UL<<msb(page_size_mask));
    //printf("Free pm base/size: 0x%lx/%luMo ; v_base: 0x%lx\n", pm_base, pm_size/1048576, (long unsigned int)v_base);
    for (page_size = 0x400; page_size < ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)); page_size <<= 1) {
        if (!(page_size & page_size_mask))
            continue;
        for (nb_pages = 1; (nb_pages <= MAX_PAGES) && (2 * nb_pages * page_size <= pm_size); nb_pages <<= 2) {
            //printf("Mapping 2 * %lu pages of %luKB: ", nb_pages, page_size/1024);
            for (i = 0; i < nb_pages; i++) {
                fpages[i] = L4_Fpage(v_base + i*page_size, page_size);
                fpages[i].X.rwx = L4_FullyAccessible;
                physmems[i] = L4_PhysDesc(pm_base + i*page_size, L4_DefaultMemory);
                res = L4_MapFpage(L4_rootspace, fpages[i], physmems[i]);
                fail_unless(res == 1, "First L4_MapFpage failed");
                w = (L4_Word_t *)(v_base + i*page_size);
                *w = (L4_Word_t)(i + 1);
                //printf("FP: %d(%lu), ", i, *w);
                //printf("va: %lx, pa: %lx\n", v_base + i*page_size, pm_base + i*page_size);
            }
            other_pm_base = pm_base + i*page_size;
            for (i = 0; i < nb_pages; i++) {
                L4_UnmapFpage(L4_rootspace, fpages[i]);
            }
            //printf("// ");
            for (i = 0; i < nb_pages; i++) {
                fpages[i] = L4_Fpage(v_base + i*page_size, page_size);
                fpages[i].X.rwx = L4_FullyAccessible;
                physmems[i] = L4_PhysDesc(other_pm_base + i*page_size, L4_DefaultMemory);
                res = L4_MapFpage(L4_rootspace, fpages[i], physmems[i]);
                fail_unless(res == 1, "Second L4_MapFpage failed");
                w = (L4_Word_t *)(v_base + i*page_size);
                //fail_unless(*w != (L4_Word_t)(i + 1), "Data are the same as in previous mapping");
                if (*w == (L4_Word_t)(i + 1)) {
                    printf("!!Data %d is the same as in previous mapping!!\n", i);
                }
                *w = (L4_Word_t)(i + 1 + nb_pages);
                //printf("FP: %d(%lu), ", i, *w);
                //printf("va: %lx, pa: %lx\n", v_base + i*page_size, other_pm_base + i*page_size);
            }
            //printf("\n");
            for (i = 0; i < nb_pages; i++) {
                L4_UnmapFpage(L4_rootspace, fpages[i]);
            }
            for (i = 0; i < nb_pages; i++) {
                fpages[i] = L4_Fpage(v_base + i*page_size, page_size);
                fpages[i].X.rwx = L4_FullyAccessible;
                physmems[i] = L4_PhysDesc(pm_base + i*page_size, L4_DefaultMemory);
                res = L4_MapFpage(L4_rootspace, fpages[i], physmems[i]);
                fail_unless(res == 1, "L4_MapFpage failed");
                r = (L4_Word_t *)(v_base + i*page_size);
                //printf("%lu ", *r);
                fail_unless(*r == (L4_Word_t)(i + 1), "Original data corrupted");
            }
            //printf("\n");
            for (i = 0; i < nb_pages; i++) {
                L4_UnmapFpage(L4_rootspace, fpages[i]);
            }
        }
        //printf("\n");
    }
}
END_TEST

/* Dependent MapItems tests ------------------------------------------------*/

/*
\begin{test}{MAP0700}
  \TestDescription{Verify {MapItems} referencing the same mapping can be mapped correctly}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Setup a sequence of \Func{MapItems} in MRs with the same virtual address and different physical addresses
        \item Use \Func{MapControl} to map the \Func{MapItems} using \Func{L4\_MapCtrl\_Read} and \Func{L4\_MapCtrl\_Modify}
        \item Check that query items returned are valid, that is, each \Func{MapItem} has modified mapping of the previous \Func{MapItem}
        \item Check that there is a valid mapping in the address space
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0700)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p, pread;
    L4_MapItem_t m;
    int i, res;

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    f.X.rwx = L4_FullyAccessible;
    for (i = 0; i < MAX_MAP_ITEMS; i++) {
        p = L4_PhysDesc(pbase + i*BASE_PAGE_SIZE, L4_DefaultMemory);
        L4_LoadMR(2*i, p.raw);
        L4_LoadMR(2*i+1, f.raw);
    }

    res = L4_MapControl(test_space, L4_MapCtrl_Modify | L4_MapCtrl_Query | (MAX_MAP_ITEMS - 1));
    fail_unless(res == 1, "MapControl failed");

    for (i = 1; i < MAX_MAP_ITEMS; i++) {
        L4_StoreMR(2*i, &pread.raw);
        L4_StoreMR(2*i+1, &m.raw);
        fail_unless(m.X.size != 0, "Non-existent mapping");
        fail_unless((pread.X.base << 10) == (pbase + (i-1)*BASE_PAGE_SIZE), "Wrong physical address read");
    }

    check_mapping(test_space, L4_Fpage(vbase, BASE_PAGE_SIZE), 1, L4_FullyAccessible, pbase + (MAX_MAP_ITEMS-1)*BASE_PAGE_SIZE, L4_DefaultMemory);

    l4e_unmap_fpage(test_space, f);
}
END_TEST


/* Unmap tests -------------------------------------------------------------*/

/*
\begin{test}{MAP0800}
  \TestDescription{Verify MapControl unmaps a simple set of mappings}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Map a sequence of simple valid mappings
        \item Use \Func{MapControl} to unmap that range
        \item Check that the mapping has been unmapped
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0800)
{
    int res;
    uintptr_t p;

    res = l4e_map(test_space, vbase, vbase + 16*BASE_PAGE_SIZE, pbase,
                  L4_FullyAccessible, L4_DefaultMemory);
    fail_unless(res == 1, "l4e_map failed");

    l4e_unmap(test_space, vbase, vbase + 16*BASE_PAGE_SIZE);
    for (p = vbase; p < vbase + 16*BASE_PAGE_SIZE; p += BASE_PAGE_SIZE) {
        check_no_mapping(test_space, L4_Fpage(p, BASE_PAGE_SIZE));
    }
}
END_TEST

/**
 * Test that checks unmapping a range of pages.
 *
 * @param[in] npages Total number of valid pages to map
 * @param[in] start First page of range to unmap
 * @param[in] end Last page of range to unamp
 */
static void
unmap_range_test(L4_SpaceId_t space, int npages, int start, int end)
{
    int res, i;
    uintptr_t v, p, vstart, pstart, vend;

    for (i = 0; i < npages; i++) {
        vstart = vbase + i*BASE_PAGE_SIZE;
        pstart = pbase + i*BASE_PAGE_SIZE;
        vend = vstart + BASE_PAGE_SIZE;
        res = l4e_map(space, vstart, vend, pstart, L4_FullyAccessible, L4_DefaultMemory);
        fail_unless(res == 1, "l4e_map failed");
    }

    l4e_unmap(space, vbase + start*BASE_PAGE_SIZE, vbase + end*BASE_PAGE_SIZE);
    for (v = vbase, p = pbase; v < vbase + start*BASE_PAGE_SIZE; v += BASE_PAGE_SIZE, p+= BASE_PAGE_SIZE) {
        check_mapping(space, L4_Fpage(v, BASE_PAGE_SIZE), 1, L4_FullyAccessible, p, L4_DefaultMemory);
    }
    for (v = vbase + start*BASE_PAGE_SIZE; v < vbase + end*BASE_PAGE_SIZE; v += BASE_PAGE_SIZE) {
        check_no_mapping(space, L4_Fpage(v, BASE_PAGE_SIZE));
    }
    for (v = vbase + end*BASE_PAGE_SIZE, p = pbase + end*BASE_PAGE_SIZE; v < vbase + npages*BASE_PAGE_SIZE; v += BASE_PAGE_SIZE, p += BASE_PAGE_SIZE) {
        check_mapping(space, L4_Fpage(v, BASE_PAGE_SIZE), 1, L4_FullyAccessible, p, L4_DefaultMemory);
    }
    l4e_unmap(space, vbase, vbase + npages*BASE_PAGE_SIZE);
}

/*
\begin{test}{MAP0801}
  \TestDescription{Verify MapControl correctly unmaps the start of a sequence of mappings}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Set up a sequence of valid mappings
        \item Unmap the first half of the sequence
        \item Check that the first half has been unmapped correctly
        \item Check that the second half is still mapped correctly
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0801)
{
    unmap_range_test(test_space, 16, 0, 8);
}
END_TEST

/*
\begin{test}{MAP0802}
  \TestDescription{Verify MapControl correctly unmaps the end of a sequence of mappings}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Set up a sequence of valid mappings
        \item Unmap the second half of that sequence
        \item Check that the second half has been unmapped correctly
        \item Check that the first half is still mapped correctly
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0802)
{
    unmap_range_test(test_space, 16, 8, 16);
}
END_TEST

/*
\begin{test}{MAP0803}
  \TestDescription{Verify MapControl correctly unmaps the middle of a sequence of mappings}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Set up a sequence of valid mappings
        \item Unmap the half of that sequence starting from a quarter of the way
        \item Check that the first quarter is still mapped correctly
        \item Check that the middle has been unmapped correctly
        \item Check that the last quarter is still mapped correctly
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0803)
{
    unmap_range_test(test_space, 16, 4, 12);
}
END_TEST


/* Over writing tests ------------------------------------------------------*/

/*
\begin{test}{MAP0900}
  \TestDescription{Verify MapControl can modify the physical address of a mapping}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Construct a valid \Func{MapItem} and map using \Func{MapControl}
        \item Change the physical address of the \Func{MapItem} and use \Func{MapControl} to remap it 
        \item Check the mapping is valid and that the new physical adddress is used
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0900)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p, pread;
    L4_MapItem_t m;
    uintptr_t new_pbase = 2*pbase;
    int res;

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    f.X.rwx = L4_FullyAccessible;
    p = L4_PhysDesc(pbase, L4_DefaultMemory);

    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Modify);
    fail_unless(res == 1, "MapControl failed");

    p = L4_PhysDesc(new_pbase, L4_DefaultMemory);
    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Modify);
    fail_unless(res == 1, "MapControl failed");

    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Query);
    fail_unless(res == 1, "MapControl failed");

    L4_StoreMR(0, &pread.raw);
    L4_StoreMR(1, &m.raw);
    fail_unless(m.X.size != 0, "no valid mapping found");
    fail_unless(pread.X.base << 10 == new_pbase, "physical address not updated");

    l4e_unmap(test_space, vbase, vbase + BASE_PAGE_SIZE);
}
END_TEST

/*
\begin{test}{MAP0901}
  \TestDescription{Verify MapControl can modify permissions of a mapping}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Construct a valid \Func{MapItem} and map using \Func{MapControl}
        \item Change the permission of the \Func{MapItem} and use \Func{MapControl} to remap it 
        \item Check the mapping is valid and that the new permission is used
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0901)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p, pread;
    L4_MapItem_t m;
    int res;

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    f.X.rwx = L4_FullyAccessible;
    p = L4_PhysDesc(pbase, L4_DefaultMemory);

    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Modify);
    fail_unless(res == 1, "MapControl failed");

    f.X.rwx = L4_ReadeXecOnly;
    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Modify);
    fail_unless(res == 1, "MapControl failed");

    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Query);
    fail_unless(res == 1, "MapControl failed");

    L4_StoreMR(0, &pread.raw);
    L4_StoreMR(1, &m.raw);
    fail_unless(m.X.size != 0, "no valid mapping found");
    fail_unless(m.X.rwx == L4_ReadeXecOnly, "failed to change permission");

    l4e_unmap(test_space, vbase, vbase + BASE_PAGE_SIZE);
}
END_TEST


#if defined(MACHINE_IPAQ_H3800) || defined(MACHINE_PLEB2) || defined(MACHINE_LN2410SBC)
static const int new_attr = L4_UncachedMemory;
# define RUN_MAP0902
#endif

#if defined(RUN_MAP0902)
/*
\begin{test}{MAP0902}
  \TestDescription{Verify MapControl can modify the physical memory attribute of a mapping}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Construct a valid \Func{MapItem} and map using \Func{MapControl}
        \item Change the $attr$ of the \Func{MapItem} and use \Func{MapControl} to remap it 
        \item Check the mapping is valid and that the new attribute is used
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP0902)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p, pread;
    L4_Word_t perm;
    int res;

    assert(default_attr != new_attr);
    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    f.X.rwx = L4_FullyAccessible;
    p = L4_PhysDesc(pbase, default_attr);

    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Modify);
    fail_unless(res == 1, "MapControl failed");

    p = L4_PhysDesc(pbase, new_attr);
    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Modify);
    fail_unless(res == 1, "MapControl failed");

    L4_LoadMR(1, f.raw);
    res = L4_MapControl(test_space, L4_MapCtrl_Query);
    fail_unless(res == 1, "MapControl failed");

    L4_StoreMR(0, &pread.raw);
    L4_StoreMR(1, &perm);
    fail_unless(PERM_GET_D(perm), "no valid mapping found");
    fail_unless(pread.X.attr == new_attr, "failed to change attribute");

    l4e_unmap(test_space, vbase, vbase + BASE_PAGE_SIZE);
}
END_TEST
#endif


/* MapControl privileged test ----------------------------------------------*/

static L4_ThreadId_t main_thread;
static void unpriv_thread(void)
{

    L4_Fpage_t f;
    L4_PhysDesc_t p;
    L4_Word_t res;
    L4_MsgTag_t tag;
    L4_Msg_t msg;
    L4_ThreadId_t dummy;

    CLEAR_ERROR_CODE;

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    f.X.rwx = L4_FullyAccessible;
    p = L4_PhysDesc(pbase, default_attr);
    L4_LoadMR(0, p.raw);
    L4_LoadMR(1, f.raw);
    f = L4_Fpage(vbase + BASE_PAGE_SIZE, BASE_PAGE_SIZE);
    f.X.rwx = L4_FullyAccessible;
    p = L4_PhysDesc(pbase + BASE_PAGE_SIZE, default_attr);
    L4_LoadMR(2, p.raw);
    L4_LoadMR(3, f.raw);

    res = L4_MapControl(test_space, L4_MapCtrl_Modify | 0x1);

    fail_unless(res == 0, "MapControl from unpriv did not error");
    _fail_unless(L4_ErrorCode() == 1, __FILE__, __LINE__, "ErrorCode=%d, should have been 1", L4_ErrorCode());

    CLEAR_ERROR_CODE;

    L4_MsgClear(&msg);
    tag.raw = 0;
    L4_Set_SendBlock(&tag);
    L4_Set_ReceiveBlock(&tag);
    L4_Set_MsgMsgTag(&msg, tag);
    L4_Set_MsgLabel(&msg, 0xdead);
    L4_MsgLoad(&msg);
    tag = L4_Ipc(main_thread, main_thread, L4_MsgMsgTag(&msg), &dummy);
    if (L4_IpcFailed(tag)) {
        printf("ERROR CODE %"PRI_D_WORD"\n", L4_ErrorCode());
        assert(!"should not get here");
    }
}

/*
\begin{test}{MAP1000}
  \TestDescription{Verify MapControl cannot be called from an unpriveledged thread}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Create an unpriviledged thread
        \item Check that the call to \Func{MapControl} by the thread fails with the appropriate error
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP1000)
{
    L4_ThreadId_t unpriv;
    L4_MsgTag_t tag;

    unpriv = createThreadInSpace(test_space, unpriv_thread);
    tag = L4_Receive(unpriv);
    fail_unless(!L4_IpcFailed(tag), "unpriv thread did not complete");
    deleteThread(unpriv);
}
END_TEST


/* UTCB tests ------------------------------------------------------*/

/*
\begin{test}{MAP1102}
  \TestDescription{Verify MapControl does not alter UTCB mapping}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Try to map the virtual address of the UTCB
        \item Check that \Func{MyGlobalId} in the UTCB has not changed
        \item Check that \Func{ThreadLocalStorage} in the UTCB has not changed
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
static L4_Word_t *test_utcb;

static void utcb_test_thread(void)
{

    L4_MsgTag_t tag;
    L4_Word_t myid, tls;

    test_utcb = L4_GetUtcbBase();

    myid = __L4_TCR_MyGlobalId();
    tls = __L4_TCR_ThreadLocalStorage();

    tag.raw = 0;
    L4_Set_MsgTag(tag);
    L4_Call(main_thread);

    fail_unless(myid ==  __L4_TCR_MyGlobalId(), "MyGlobalId is invalid");
    fail_unless(tls ==  __L4_TCR_ThreadLocalStorage(), "MyGlobalId is invalid");

    tag.raw = 0;
    L4_Set_MsgTag(tag);
    L4_Call(main_thread);
}

START_TEST(MAP1102)
{
    int res;
    L4_Msg_t msg;
    L4_MsgTag_t tag;

    L4_ThreadId_t thread;

    thread = createThreadInSpace(test_space, utcb_test_thread);
    L4_ThreadSwitch(thread);

    tag = L4_Niltag;
    L4_Set_MsgTag(tag);
    // Wait for thread
    tag = L4_Receive(thread);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could get message from thread.  Error: %ld",
                             (int) L4_ErrorCode ());

    res = l4e_map(test_space, (uintptr_t)test_utcb, (uintptr_t)test_utcb + BASE_PAGE_SIZE,
                  pbase, L4_FullyAccessible, L4_DefaultMemory);
    fail_unless(res == 1, "l4e_map failed");

    L4_MsgClear (&msg);
    L4_MsgLoad (&msg);
    // Wait for thread
    tag = L4_Call(thread);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could not send message to thread.  Error: %ld",
                             (int) L4_ErrorCode ());

    deleteThread(thread);
}
END_TEST

/*
\begin{test}{MAP1103}
  \TestDescription{Verify MapControl does cannot unmap UTCB}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Try to unmap the virtual address of the UTCB
        \item Check that \Func{MyGlobalId} in the UTCB has not changed
        \item Check that \Func{ThreadLocalStorage} in the UTCB has not changed
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP1103)
{
    L4_Msg_t msg;
    L4_MsgTag_t tag;

    L4_ThreadId_t thread;

    thread = createThreadInSpace(test_space, utcb_test_thread);
    L4_ThreadSwitch(thread);

    tag = L4_Niltag;
    L4_Set_MsgTag(tag);
    // Wait for thread
    tag = L4_Receive(thread);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could get message from thread.  Error: %ld",
                             (int) L4_ErrorCode ());

    l4e_unmap(test_space, (uintptr_t)test_utcb, (uintptr_t)test_utcb + BASE_PAGE_SIZE);

    L4_MsgClear (&msg);
    L4_MsgLoad (&msg);
    // Wait for thread
    tag = L4_Call(thread);

    _fail_unless (L4_IpcSucceeded (tag), __FILE__, __LINE__, "Could not send message to thread.  Error: %ld",
                             (int) L4_ErrorCode ());

    deleteThread(thread);
}
END_TEST


/* Derived Function Tests --------------------------------------------------*/

/*
\begin{test}{MAP1200}
  \TestDescription{Verify the derived functions L4\_MapFpage and L4\_UnmapFpage}
  \TestFunctionalityTested{\Func{L4\_MapFpage}, \Func{L4\_UnmapFpage}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Setup a valid \Func{Fpage\_t} and \Func{PhysDesc\_t}
        \item Map them using \Func{L4\_MapFpage}
        \item Check that the mapping is correct
        \item Unmap them using \Func{L4\_UnmapFpage}
        \item Check that the mapping has been correctly unmapped 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP1200)
{
    L4_Fpage_t f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    L4_PhysDesc_t p = L4_PhysDesc(pbase, L4_DefaultMemory);
    int res;

    f.X.rwx = L4_FullyAccessible;

    res = L4_MapFpage(test_space, f, p);
    fail_unless(res == 1, "L4_MapFpage failed");
    check_mapping(test_space, f, 1, L4_FullyAccessible, pbase, L4_DefaultMemory);

    L4_UnmapFpage(test_space, f);
    check_no_mapping(test_space, f);
}
END_TEST


/*
\begin{test}{MAP1201}
  \TestDescription{Verify the derived functions L4\_MapFpages and L4\_UnmapFpages}
  \TestFunctionalityTested{\Func{L4\_MapFpages}, \Func{L4\_UnmapFpages}}
  \TestImplementationProcess{
    \begin{enumerate}
        \item Setup a series of valid \Func{Fpage\_t}'s and \Func{PhysDesc\_t}'s
        \item Map the series using \Func{L4\_MapFpages}
        \item Check that the mappings are correct
        \item Unmap the series using \Func{L4\_UnmapFpages}
        \item Check that the mappings have been correctly unmapped 
    \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP1201)
{
    L4_Fpage_t fs[MAX_MAP_ITEMS];
    L4_PhysDesc_t ps[MAX_MAP_ITEMS];
    int i, res;

    for (i = 0; i < MAX_MAP_ITEMS; i++) {
        fs[i] = L4_Fpage(vbase + i*BASE_PAGE_SIZE, BASE_PAGE_SIZE);
        fs[i].X.rwx = L4_FullyAccessible;
        ps[i] = L4_PhysDesc(pbase + i*BASE_PAGE_SIZE, L4_DefaultMemory);
    }

    res = L4_MapFpages(test_space, MAX_MAP_ITEMS, fs, ps);
    fail_unless(res == 1, "L4_MapFpages failed");
    for (i = 0; i < MAX_MAP_ITEMS; i++) {
        check_mapping(test_space, fs[i], 1, L4_FullyAccessible, pbase + i*BASE_PAGE_SIZE, L4_DefaultMemory);
    }

    L4_UnmapFpages(test_space, MAX_MAP_ITEMS, fs);
    for (i = 0; i < MAX_MAP_ITEMS; i++) {
        check_no_mapping(test_space, fs[i]);
    }
}
END_TEST


/* Overmapping Tests -------------------------------------------------------*/

/*
\begin{test}{MAP1300}
  \TestDescription{Verify overmapping of sequences of mappings}
  \TestFunctionalityTested{\Func{MapControl}}
  \TestImplementationProcess{
        For each valid pair of page sizes, ${a, b}$, where the sizes are taken from \Func{page\_size\_mask} in the KIP:
        \begin{enumerate}
                \item $min = \min(a, b)$
                \item $max = \max(a, b)$
                \item $n = max/min$
                \item If $a < b$ map $a$ $n$ times, otherwise map $a$ once
                \item If $b < a$ map $b$ $n$ times, otherwise map $b$ once using the same base virtual address as $a$
                \item Check that the mapping for $b$ is complete and valid
        \end{enumerate}
  }
  \TestImplementationStatus{Implemented}
  \TestRegressionStatus{In regression test suite}
  \TestIsFullyAutomated{Yes}
\end{test}
*/
START_TEST(MAP1300)
{
    L4_Word_t a, b, page_size_mask;
    L4_Word_t map_base;

    page_size_mask = L4_GetPageMask();
    for (a = 0x400; a < ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)); a <<= 1) {
        if (!(a & page_size_mask))
            continue;
        for (b = 0x400; b < ((L4_Word_t)1 << (L4_BITS_PER_WORD-1)); b <<= 1) {
            if (!(b & page_size_mask))
                continue;
            if (a == b)
                continue;

            /* correctly align up bases */
            map_base = (vbase + a-1) & (~(a-1));
            map_base = (map_base + b-1) & (~(b-1));

            map_region_test(test_space, a, map_base, b, map_base, 0);
        }
    }
}
END_TEST


/* Permissions Tests -------------------------------------------------------*/

/*
\begin{test}{MAP1400}
   \TestDescription{Verify page permissions work as specified in the KIP (r/w/x only mapped where architecture supports it)}
   \TestFunctionalityTested{\Func{MapControl}}
   \TestImplementationProcess{
      \begin{enumerate}
         \item For each combination of permissions, map a page with that permission
         \item Read the permissions from the page and verify that the permissions were set as per the PMask.
      \end{enumerate}
   }
   \TestImplementationStatus{Implemented}
   \TestRegressionStatus{In regression test suite}
   \TestIsFullyAutomated{Yes}
\end{test}
*/

START_TEST(MAP1400)
{
    L4_Fpage_t f;
    L4_PhysDesc_t p, q;
    L4_MapItem_t m;
    int pmask, i, res, perm;


    pmask = L4_GetPagePerms();

    f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    p = L4_PhysDesc(pbase, L4_DefaultMemory); 

    for(i=0;i<=7;i++) {
        f.X.rwx = i;
        L4_LoadMR(0, p.raw);
        L4_LoadMR(1, f.raw);
        res = L4_MapControl(test_space, L4_MapCtrl_Modify);
        fail_unless(res == 1, "L4_MapControl(..., L4_MapCtrl_Modify) failed");

        /* Calculate expected permission */ 
        if(i==0)
            perm = 0;
        else
            perm = (~pmask | i) & 7;

        res = L4_ReadFpage(test_space, f, &q, &m);
        fail_unless(res == 1, "L4_MapControl(..., L4_MapCtrl_Query) failed");
        fail_unless(m.X.rwx == perm, "Incorrect permissions read");

        l4e_unmap_fpage(test_space, f);
    }
}
END_TEST


/* Deprivileged Mapping Tests ------------------------------------------------*/

// Try to use mapcontrol, map and unmap should both fail
static void
depriv_map_fail_all(void)
{
    L4_MsgTag_t tag;
    L4_SpaceId_t target;
    L4_Fpage_t f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    L4_PhysDesc_t p = L4_PhysDesc(pbase, L4_DefaultMemory);
    int res;

    tag = L4_Receive(main_thread);
    L4_StoreMR(0, &target.raw);

    f.X.rwx = L4_FullyAccessible;
    res = L4_MapFpage(target, f, p);
    fail_unless(res == 0, "L4_MapFpage succeeded");

    res = L4_UnmapFpage(target, f);
    fail_unless(res == 0, "L4_UnmapFpage succeeded");

    while (1)
        L4_Call(main_thread);
}

// Try to use mapcontrol, should fail on map only
static void
depriv_map_fail_map(void)
{
    L4_MsgTag_t tag;
    L4_SpaceId_t target;
    L4_Fpage_t f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    L4_PhysDesc_t p = L4_PhysDesc(pbase, L4_DefaultMemory);
    int res;

    tag = L4_Receive(main_thread);
    L4_StoreMR(0, &target.raw);

    f.X.rwx = L4_FullyAccessible;
    res = L4_MapFpage(target, f, p);
    fail_unless(res == 0, "L4_MapFpage succeeded");

    res = L4_UnmapFpage(target, f);
    fail_unless(res == 1, "L4_UnmapFpage failed");

    while (1)
        L4_Call(main_thread);
}

// This map control call should succeed    
static void
depriv_map_pass(void)
{
    L4_MsgTag_t tag;
    L4_SpaceId_t target;
    L4_Fpage_t f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    L4_PhysDesc_t p = L4_PhysDesc(pbase, L4_DefaultMemory);
    int res;

    tag = L4_Receive(main_thread);
    L4_StoreMR(0, &target.raw);

    f.X.rwx = L4_FullyAccessible;
    res = L4_MapFpage(target, f, p);
    fail_unless(res == 1, "L4_MapFpage failed");

    check_mapping(target, f, 1, L4_FullyAccessible, pbase, L4_DefaultMemory);

    res = L4_UnmapFpage(target, f);
    fail_unless(res == 1, "L4_UnmapFpage failed");

    check_mapping(target, f, 0, L4_FullyAccessible, pbase, L4_DefaultMemory);

    while (1)
        L4_Call(main_thread);
}

// This map control call should fail for mapping outside the allocated range
static void
depriv_map_fail_range(void)
{
    L4_MsgTag_t tag;
    L4_SpaceId_t target;
    L4_Fpage_t f = L4_Fpage(vbase, BASE_PAGE_SIZE);
    L4_PhysDesc_t p = L4_PhysDesc(pbase - 0x10, L4_DefaultMemory);
    int res;

    tag = L4_Receive(main_thread);
    L4_StoreMR(0, &target.raw);

    f.X.rwx = L4_FullyAccessible;
    res = L4_MapFpage(target, f, p);
    fail_unless(res == 0, "L4_MapFpage succeded");

    check_mapping(target, f, 0, L4_FullyAccessible, pbase - 0x10, L4_DefaultMemory);

    /* We should be able to unmap any address we are the pager of and we have
     * some user mapping settings
     */
    res = L4_UnmapFpage(target, f);
    fail_unless(res == 1, "L4_UnmapFpage failed");

    check_mapping(target, f, 0, L4_FullyAccessible, pbase - 0x10, L4_DefaultMemory);

    p = L4_PhysDesc(pbase + BASE_PAGE_SIZE + 0x10, L4_DefaultMemory);

    res = L4_MapFpage(target, f, p);
    fail_unless(res == 0, "L4_MapFpage succeded");

    check_mapping(target, f, 0, L4_FullyAccessible, pbase + BASE_PAGE_SIZE + 0x10, L4_DefaultMemory);

    res = L4_UnmapFpage(target, f);
    fail_unless(res == 1, "L4_UnmapFpage failed");

    check_mapping(target, f, 0, L4_FullyAccessible, pbase, L4_DefaultMemory);

    while (1)
        L4_Call(main_thread);
}

// Check that a deprivileged map call fails with a new space
START_TEST(MAP1500)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    thread = createThreadInSpace(space, depriv_map_fail_all);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_fail thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

// Check that only having user mapping defined causes a call to fail
START_TEST(MAP1501)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    L4_AllowUserMapping(space, pbase, BASE_PAGE_SIZE);
    thread = createThreadInSpace(space, depriv_map_fail_all);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_fail thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

// Check that just being the space pager fails
START_TEST(MAP1502)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    L4_LoadMR(0, space.raw);
    L4_SpaceControl(target, L4_SpaceCtrl_space_pager, L4_rootclist, L4_Nilpage, 0, NULL);
    thread = createThreadInSpace(space, depriv_map_fail_map);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_fail thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

// Check that being the space pager and having user mapping defined works
START_TEST(MAP1503)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    L4_LoadMR(0, space.raw);
    L4_SpaceControl(target, L4_SpaceCtrl_space_pager, L4_rootclist, L4_Nilpage, 0, NULL);
    L4_AllowUserMapping(space, pbase, BASE_PAGE_SIZE);
    thread = createThreadInSpace(space, depriv_map_pass);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_pass thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

// Check that mapping outside the phys range doesn't work
START_TEST(MAP1504)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    L4_LoadMR(0, space.raw);
    L4_SpaceControl(target, L4_SpaceCtrl_space_pager, L4_rootclist, L4_Nilpage, 0, NULL);
    L4_AllowUserMapping(space, pbase, BASE_PAGE_SIZE);
    thread = createThreadInSpace(space, depriv_map_fail_range);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_pass thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

// Check that restricting user mapping actually does prevent map calls
START_TEST(MAP1505)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    L4_LoadMR(0, space.raw);
    L4_SpaceControl(target, L4_SpaceCtrl_space_pager, L4_rootclist, L4_Nilpage, 0, NULL);
    L4_AllowUserMapping(space, pbase, BASE_PAGE_SIZE);

    L4_RestrictUserMapping(space, pbase, BASE_PAGE_SIZE);
    thread = createThreadInSpace(space, depriv_map_fail_map);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_fail thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

// Check that restrcting the space pager causes map calls to fail
START_TEST(MAP1506)
{
    L4_ThreadId_t thread;
    L4_MsgTag_t tag;
    L4_SpaceId_t space, target;

    space = createSpace();
    target = createSpace();

    L4_LoadMR(0, space.raw);
    L4_SpaceControl(target, L4_SpaceCtrl_space_pager, L4_rootclist, L4_Nilpage, 0, NULL);
    L4_AllowUserMapping(space, pbase, BASE_PAGE_SIZE);

    L4_LoadMR(0, 0);
    L4_SpaceControl(target, L4_SpaceCtrl_space_pager, L4_rootclist, L4_Nilpage, 0, NULL);
    thread = createThreadInSpace(space, depriv_map_fail_all);

    L4_LoadMR(0, target.raw);
    tag = L4_Send(thread);
    tag = L4_Receive(thread);
    fail_unless(!L4_IpcFailed(tag), "depriv_map_fail thread did not complete");

    deleteThread(thread);
    deleteSpace(target);
    deleteSpace(space);
}
END_TEST

/* -------------------------------------------------------------------------*/
extern L4_ThreadId_t test_tid;

static void test_setup(void)
{
    initThreads(0);
    test_space = createSpace();
    main_thread = test_tid;
}

static void test_teardown(void)
{
    deleteSpace(test_space);
}

/* -------------------------------------------------------------------------*/

TCase *
make_mapcontrol_tcase(void)
{
    TCase *tc;

    tc = tcase_create("MapControl Interface");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);

    tcase_add_test(tc, MAP0100);
    tcase_add_test(tc, MAP0101);
    tcase_add_test(tc, MAP0102);

    tcase_add_test(tc, MAP0200);
    tcase_add_test(tc, MAP0201);
    tcase_add_test(tc, MAP0202);
    tcase_add_test(tc, MAP0203);
#if defined(L4_32BIT)
    tcase_add_test(tc, MAP0204);
#endif

    tcase_add_test(tc, MAP0300);
    tcase_add_test(tc, MAP0301);

#if MAX_MAP_ITEMS < 32
    tcase_add_test(tc, MAP0400);
#endif

    tcase_add_test(tc, MAP0500);
    tcase_add_test(tc, MAP0501);
    tcase_add_test(tc, MAP0502);
    tcase_add_test(tc, MAP0503);

    tcase_add_test(tc, MAP0600);
    tcase_add_test(tc, MAP0601);

    tcase_add_test(tc, MAP0700);

    tcase_add_test(tc, MAP0800);
    tcase_add_test(tc, MAP0801);
    tcase_add_test(tc, MAP0802);
    tcase_add_test(tc, MAP0803);

    tcase_add_test(tc, MAP0900);
    tcase_add_test(tc, MAP0901);
#if defined(RUN_MAP0902)
    tcase_add_test(tc, MAP0902);
#endif

    tcase_add_test(tc, MAP1000);

    tcase_add_test(tc, MAP1102);
    tcase_add_test(tc, MAP1103);

    tcase_add_test(tc, MAP1200);
    tcase_add_test(tc, MAP1201);

    tcase_add_test(tc, MAP1300);
    tcase_add_test(tc, MAP1400);

    tcase_add_test(tc, MAP1500);
    tcase_add_test(tc, MAP1501);
    tcase_add_test(tc, MAP1502);
    tcase_add_test(tc, MAP1503);
    tcase_add_test(tc, MAP1504);
    tcase_add_test(tc, MAP1505);
    tcase_add_test(tc, MAP1506);

    return tc;
}
