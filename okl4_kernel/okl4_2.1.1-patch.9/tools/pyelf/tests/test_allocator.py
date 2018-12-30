##############################################################################
# Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
# All rights reserved.
# 
# 1. Redistribution and use of OKL4 (Software) in source and binary
# forms, with or without modification, are permitted provided that the
# following conditions are met:
# 
#     (a) Redistributions of source code must retain this clause 1
#         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
#         (Licence Terms) and the above copyright notice.
# 
#     (b) Redistributions in binary form must reproduce the above
#         copyright notice and the Licence Terms in the documentation and/or
#         other materials provided with the distribution.
# 
#     (c) Redistributions in any form must be accompanied by information on
#         how to obtain complete source code for:
#        (i) the Software; and
#        (ii) all accompanying software that uses (or is intended to
#        use) the Software whether directly or indirectly.  Such source
#        code must:
#        (iii) either be included in the distribution or be available
#        for no more than the cost of distribution plus a nominal fee;
#        and
#        (iv) be licensed by each relevant holder of copyright under
#        either the Licence Terms (with an appropriate copyright notice)
#        or the terms of a licence which is approved by the Open Source
#        Initative.  For an executable file, "complete source code"
#        means the source code for all modules it contains and includes
#        associated build and other files reasonably required to produce
#        the executable.
# 
# 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
# LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
# IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
# EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
# THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
# BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
# PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
# THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
# 
# 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import unittest
import weaver.allocator

modules_under_test = [weaver.allocator]

# Standard page size used by the tests.
PAGE_SIZE = 4096
MEGABYTE = 1024L * 1024L

class TestAllocator(unittest.TestCase):

    def test_creation(self):
        """The allocator requires that the size be a power of two."""
        self.assertRaises(AssertionError, weaver.allocator.Allocator,
                           PAGE_SIZE - 1)
        self.assertRaises(AssertionError, weaver.allocator.Allocator,
                          PAGE_SIZE + 1)
        self.assertRaises(AssertionError, weaver.allocator.Allocator,
                          PAGE_SIZE * 3)

        self.assertRaises(AssertionError, weaver.allocator.Allocator,
                          8095)
        self.assertRaises(AssertionError, weaver.allocator.Allocator,
                          8191)
        self.assertRaises(AssertionError, weaver.allocator.Allocator,
                          8192 * 3)

        # Test with various powers of two.
        self.failUnless(weaver.allocator.Allocator(PAGE_SIZE))
        self.failUnless(weaver.allocator.Allocator(PAGE_SIZE * 2))
        self.failUnless(weaver.allocator.Allocator(MEGABYTE))
        self.failUnless(weaver.allocator.Allocator(MEGABYTE * 8))
        

    def test_add_memory(self):
        """
        Test various combinations of adding memory to a fresh pool.
        """
        alloc = weaver.allocator.Allocator(PAGE_SIZE)

        # Add a single range to the free list.
        alloc.add_memory(0, PAGE_SIZE, None)

        free = alloc.get_freelist()
        full = alloc.fulllist

        self.assertEqual(len(free), 1)
        self.assertEqual(free[0][0], 0)
        self.assertEqual(free[0][1], PAGE_SIZE - 1)

        self.assertEqual(len(full), 1)
        self.assertEqual(full[0][0], 0)
        self.assertEqual(full[0][1], PAGE_SIZE - 1)

        # Add more memory but in the 64-bit range to test cross
        # platform support.
        alloc.add_memory(5L * MEGABYTE, MEGABYTE, None)

        free = alloc.get_freelist()
        full = alloc.fulllist

        self.assertEqual(len(free), 2)
        self.assertEqual(free[1][0], 5L * MEGABYTE)
        self.assertEqual(free[1][1], 6L * MEGABYTE - 1)

        self.assertEqual(len(full), 2)
        self.assertEqual(full[1][0], 5L * MEGABYTE)
        self.assertEqual(full[1][1], 6L * MEGABYTE - 1)

        # Check that the freelist is sorted.

        alloc.add_memory(4 * PAGE_SIZE, PAGE_SIZE, None)

        free = alloc.get_freelist()
        full = alloc.fulllist

        self.assertEqual(len(free), 3)
        self.assertEqual(free[1][0], 4 * PAGE_SIZE)
        self.assertEqual(free[1][1], 5 * PAGE_SIZE - 1)

        self.assertEqual(len(full), 3)
        self.assertEqual(full[1][0], 4 * PAGE_SIZE)
        self.assertEqual(full[1][1], 5 * PAGE_SIZE - 1)

    def test_add_bad_memory(self):
        """Test that adding invalid memory regions raises exceptions."""
        alloc = weaver.allocator.Allocator(PAGE_SIZE)

        # Add unaligned and non-page sized regions.
        self.assertRaises(AssertionError, alloc.add_memory,
                          1, PAGE_SIZE, None)
        self.assertRaises(AssertionError, alloc.add_memory,
                          0, PAGE_SIZE - 1, None)

        self.assertRaises(AssertionError, alloc.add_memory,
                          PAGE_SIZE * 2 - 1, PAGE_SIZE, None)
        self.assertRaises(AssertionError, alloc.add_memory,
                          PAGE_SIZE * 2, PAGE_SIZE * 2 - 1, None)

        # Test that overlapping memory regions fail.
        alloc.add_memory(0, PAGE_SIZE * 10, None)
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.add_memory, 0, PAGE_SIZE, None)
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.add_memory, 9 * PAGE_SIZE, PAGE_SIZE, None)

        # Adjacent memory regions should not produce an error, but
        # instead be merge together.
        alloc.add_memory(PAGE_SIZE * 10, PAGE_SIZE, None)

        # Verify that only the successful additions are in the system
        self.assertEqual(len(alloc.fulllist), 1)
        self.assertEqual(alloc.freelist[0][0], 0)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 11) - 1)

    def test_next_avail(self):
        """Tests for the next_avail() method."""

        alloc = weaver.allocator.Allocator(PAGE_SIZE)

        # An empty freelist should fail.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.next_avail)

        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 100, None)

        # With one entry, the base of the block should be returned.
        self.assertEqual(alloc.next_avail(), PAGE_SIZE)

        # Add a smaller block, but the first should still be
        # returned.
        alloc.add_memory(PAGE_SIZE * 200, PAGE_SIZE * 50, None)
        self.assertEqual(alloc.next_avail(), PAGE_SIZE)

        # Add a block of the same size.  In this case the one with the
        # largest base address should be returned.
        alloc.add_memory(PAGE_SIZE * 500, PAGE_SIZE * 100, None)
        self.assertEqual(alloc.next_avail(), PAGE_SIZE * 500)

        # Add a larger block.  This should be returned.
        alloc.add_memory(PAGE_SIZE * 1000, PAGE_SIZE * 1000, None)
        self.assertEqual(alloc.next_avail(), PAGE_SIZE * 1000)

    def test_mark_window(self):
        """Tests for the mark_window() method."""

        alloc = weaver.allocator.Allocator(PAGE_SIZE)

        # Prime the freelist.
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 3 * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 4 * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 5 * 1024, PAGE_SIZE * 1024, None)

        # Cannot allocate negative sizes
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, 1024, -20, None)
        

        # Cannot allocate windows that fall outside memory passed to
        # add_memory()

        # Off the bottom of the list.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, 0, PAGE_SIZE, None)
        # Across the bottom of the first free range.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, 0, PAGE_SIZE * 1024, None)
        # Across the top of the first free range.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, PAGE_SIZE * 1000,
                          PAGE_SIZE * 200, None)
        # Across the bottom of the second free range.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, PAGE_SIZE * (1024 * 2 - 100),
                          PAGE_SIZE * 200, None)
        # The cap between two ranges.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, PAGE_SIZE * 2 * 1024,
                          PAGE_SIZE * 1024, None)
        # Across the last three ranges and the off the end.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, PAGE_SIZE * 3 * 1024,
                          PAGE_SIZE * 1024 * 4, None)
        # Off the top of the last free range.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window, PAGE_SIZE * 10000,
                          PAGE_SIZE, None)

        
        # Check that holes are in the proposed window.

        # Off the bottom of the window.
        self.assertRaises(AssertionError,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(0, 1024)])

        # Across the bottom of the window
        self.assertRaises(AssertionError,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * (1024 - 1), PAGE_SIZE * 2)])

        # Across the top of the window.
        self.assertRaises(AssertionError,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * (1024 * 2 - 1), PAGE_SIZE * 2)])

        # Off the top of the window.
        self.assertRaises(AssertionError,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * (1024 * 2), PAGE_SIZE * 2)])


        # Reset the allocater to test some actual allocations.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)

        # The window covers the entire freelist.  These should be
        # nothing left.
        alloc.mark_window(PAGE_SIZE * 1024, PAGE_SIZE * 1024, [])
        self.assertEqual(len(alloc.get_freelist()), 0)

        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)

        # Claim that there is a hole, but there isn't one.
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * 1024, PAGE_SIZE)])
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * 1025, PAGE_SIZE)])
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * (1024 * 2 - 1), PAGE_SIZE)])

        # Claim a window across three freelist records but doesn't use
        # all of the memory.
        alloc.add_memory(PAGE_SIZE * 2 * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 3 * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 + 500), PAGE_SIZE), True)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 3 + 500), PAGE_SIZE), True)
        alloc.mark_window(PAGE_SIZE * 1024,
                          PAGE_SIZE * (1024 * 3 - 1),
                          [(PAGE_SIZE * (1024 + 500), PAGE_SIZE),
                           (PAGE_SIZE * (1024 * 3 + 500), PAGE_SIZE)])
        self.assertEqual(len(alloc.get_freelist()), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * (1024 * 4 - 1))
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1024 * 4) - 1)

        # A one byte window should remove a page from the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.mark_window(PAGE_SIZE * 1024, 1, [])
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1025)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1024 * 2) - 1)

        # Mark window without a hole, but there is one.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 + 500), PAGE_SIZE), True)
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024, [])

        # But declaring the hole should work, and leave an empty
        # freelist.
        alloc.mark_window(PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                           [(PAGE_SIZE * (1024 + 500), PAGE_SIZE)])
        self.assertEqual(len(alloc.get_freelist()), 0)

        # Mark window with a hole that is too big and runs into the
        # next freelist entry.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 + 500), PAGE_SIZE), True)
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * (1024 + 499), PAGE_SIZE * 2)])
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark_window,
                          PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                          [(PAGE_SIZE * (1024 + 500), PAGE_SIZE * 2)])

        # Alloc the middle free list record.  The remaining records
        # should be untouched.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(0, PAGE_SIZE * 1023, None)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 1024 * 3, PAGE_SIZE * 1024, None)
        alloc.mark_window(PAGE_SIZE * 1024, PAGE_SIZE * 1024, [])
        self.assertEqual(len(alloc.get_freelist()), 2)
        self.assertEqual(alloc.freelist[0][0], 0)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1023) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 1024 * 3)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1024 * 4) - 1)

        # Allocate a region that is all hole.
        # Firstly with only one initial freelist record
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 1024, PAGE_SIZE * 1024), True)
        alloc.mark_window(PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                           [(PAGE_SIZE * 1024, PAGE_SIZE * 1024)])
        self.assertEqual(len(alloc.get_freelist()), 0)

        # And now with free records around it.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(0, PAGE_SIZE * 1023, None)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 1024 * 3, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 1024, PAGE_SIZE * 1024), True)
        alloc.mark_window(PAGE_SIZE * 1024, PAGE_SIZE * 1024,
                           [(PAGE_SIZE * 1024, PAGE_SIZE * 1024)])
        self.assertEqual(len(alloc.get_freelist()), 2)
        self.assertEqual(alloc.freelist[0][0], 0)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1023) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 1024 * 3)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1024 * 4) - 1)

        # Put the window in the middle of a freelist record, splitting
        # the record.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.mark_window(PAGE_SIZE * (1024 + 500), PAGE_SIZE, [])
        self.assertEqual(len(alloc.get_freelist()), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1024)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * (1024 + 500) - 1))
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * (1024 + 501))
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * (1024 * 2) - 1))

    def test_mark(self):
        """Tests for the mark() method."""

        alloc = weaver.allocator.Allocator(PAGE_SIZE)

        # Cannot mark negative sizes
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.mark, 1024, -20)

        # Marking in an empty list is OK.
        self.assertEqual(alloc.mark(0, PAGE_SIZE), False)
        self.assertEqual(alloc.mark(PAGE_SIZE * 2, PAGE_SIZE * 10), False)
        self.assertEqual(alloc.mark(100, 50), False)

        # Marking everything should empty the free list.
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 1024, PAGE_SIZE * 1024), True)
        self.assertEqual(len(alloc.freelist), 0)

        # Marking one byte should remove a page from the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 1024, 1), True)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1025)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1024 * 2) - 1)

        # Mark the last page of the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 2 - 1), PAGE_SIZE), True)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1024)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * (1024 * 2 - 1)) - 1)

        # Mark the last page of the first entry in the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 1024 * 2, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 2 - 1), PAGE_SIZE), True)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1024)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * (1024 * 2 - 1)) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 1024 * 2)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1024 * 3) - 1)

        # Mark part of the empty region between two free ranges.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 1024 * 3, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 2 + 10), PAGE_SIZE), False)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1024)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * (1024 * 2)) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 1024 * 3)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1024 * 4) - 1)

        # Mark the ranges that overlap the edges of the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 1024 * 3, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 - 1), PAGE_SIZE * 2), True)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 2 - 1), PAGE_SIZE * 2), True)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 3 - 1), PAGE_SIZE * 2), True)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 * 4 - 1), PAGE_SIZE * 2), True)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * (1024 + 1))
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * (1024 * 2 - 1)) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * (1024 * 3 + 1))
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * (1024 * 4 - 1)) - 1)

        # Mark in the middle of a free range, splitting the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * (1024 + 500), PAGE_SIZE), True)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 1024)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * (1024 + 500)) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * (1024 + 501))
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * (1024 * 2)) - 1)

        # Mark across freelist ranges and a gap.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * 1024 * 3, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 1024, PAGE_SIZE * 1024 * 3), True)
        self.assertEqual(len(alloc.freelist), 0)

    def test_alloc_single(self):
        """
        Tests for the alloc() method allocating memory for a single
        object.
        """
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)

        # Try to allocate using an instance of the abstract
        # AllocatorItem class.  This should assert.
        item = weaver.allocator.AllocatorItem()
        group = weaver.allocator.AllocatorGroup(0, [item])
        self.assertRaises(NotImplementedError, alloc.alloc, group)

        # Test that abstract interfaces fail.  This is chiefly here
        # so that all of the code is executed for the coverage tests.
        self.assertRaises(NotImplementedError, item.get_name)
        self.assertRaises(NotImplementedError, item.get_size)
        self.assertRaises(NotImplementedError, item.get_alignment)
        self.assertEqual(item.get_offset(), 0)
        self.assertRaises(NotImplementedError, item.get_addr)
        self.assertRaises(NotImplementedError, item.set_addr, 0)
        

        # Allocate an empty group.  The freelist should be unchanged.
        group = weaver.allocator.AllocatorGroup(0, None)
        alloc.alloc(group)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Allocate a simple item.  This should be placed at the bottom
        # of the freelist.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 2)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Call allocate with the same item again.  This nothing should
        # change.  You can call alloc() with items that already have
        # addresses because a group can contain a mixed collection of
        # marked and to-be-allocated items.
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 2)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Allocate a simple item with a non-zero offset.  This should
        # be allocated at the bottom of the freelist, but the address
        # will not be page aligned.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE,
                                                    offset = 500)
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE * 2 + 500)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 4)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Allocate a page with a large alignment.  This will split the
        # freelist
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE,
                                                    alignment = 10 * PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE * 10)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 4)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 10) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 11)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1025) - 1)

        # Allocate an item so large that it has to come from the
        # second freelist record.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE * 10)
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE * 11)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 4)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 10) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 21)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1025) - 1)

        # Now something small again, so that it will come from the
        # first freelist record.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE * 4)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 5)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 10) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 21)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1025) - 1)

        # Alloc the remains of the first freelist record.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE * 5)
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE * 5)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 21)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Alloc the rest and empty the freelist.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE * (1024 - 20))
        group = weaver.allocator.AllocatorGroup(0, [item])
        alloc.alloc(group)
        self.assertEqual(item.get_addr(), PAGE_SIZE * 21)
        self.assertEqual(len(alloc.freelist), 0)

        # Alloc on an empty list.  Throws an exception.
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item])
        self.assertRaises(weaver.allocator.AllocatorException, alloc.alloc, group)

        # Ask for more memory than is in any one block in the freelist.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE * 1024 * 0, PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * ((1024 * 1) + 1), PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * ((1024 * 2) + 2), PAGE_SIZE * 1024, None)
        alloc.add_memory(PAGE_SIZE * ((1024 * 3) + 3), PAGE_SIZE * 1024, None)
        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE * 1025)
        group = weaver.allocator.AllocatorGroup(0, [item],
                                                error_message="Custom Error Message")
        self.assertRaises(weaver.allocator.AllocatorException, alloc.alloc, group)

    def test_alloc_group(self):
        """
        Tests for the alloc() method allocating memory for a multiple
        objects.
        """
        # Allocate two items that should be next to one another.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)
        item1 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        item2 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        alloc.alloc(group)
        self.assertEqual(item1.get_addr(), PAGE_SIZE)
        self.assertEqual(item2.get_addr(), PAGE_SIZE * 2)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 3)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Allocate two items that should be next to one another on the
        # same page.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)
        item1 = weaver.allocator.AllocatorSimpleItem(size = 1024)
        item2 = weaver.allocator.AllocatorSimpleItem(size = 1024)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        alloc.alloc(group)
        self.assertEqual(item1.get_addr(), PAGE_SIZE)
        self.assertEqual(item2.get_addr(), PAGE_SIZE + 1024)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 2)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Allocate two items that should be next to one another on the
        # same page and the total size crosses a page boundary.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)
        item1 = weaver.allocator.AllocatorSimpleItem(size = 1024)
        item2 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        alloc.alloc(group)
        self.assertEqual(item1.get_addr(), PAGE_SIZE)
        self.assertEqual(item2.get_addr(), PAGE_SIZE + 1024)
        self.assertEqual(len(alloc.freelist), 1)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE * 3)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 1025) - 1)

        # Allocate two items that should be next to one another, but
        # the first record of the freelist only has one page in it.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 2, PAGE_SIZE), True)
        item1 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        item2 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        alloc.alloc(group)
        self.assertEqual(item1.get_addr(), PAGE_SIZE * 3)
        self.assertEqual(item2.get_addr(), PAGE_SIZE * 4)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 2) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 5)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1025) - 1)

        # Allocate one item that should be next to another item that
        # is already at a fixed address.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 2, PAGE_SIZE), True)
        item1 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE,
                                                     addr = PAGE_SIZE * 2)
        item2 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        alloc.alloc(group)
        self.assertEqual(item1.get_addr(), PAGE_SIZE * 2)
        self.assertEqual(item2.get_addr(), PAGE_SIZE * 3)
        self.assertEqual(len(alloc.freelist), 2)
        self.assertEqual(alloc.freelist[0][0], PAGE_SIZE)
        self.assertEqual(alloc.freelist[0][1], (PAGE_SIZE * 2) - 1)
        self.assertEqual(alloc.freelist[1][0], PAGE_SIZE * 4)
        self.assertEqual(alloc.freelist[1][1], (PAGE_SIZE * 1025) - 1)

        # Allocate two items that should be next to one another, but
        # the memory is so fragmented that this is not possible.
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 3, None)
        self.assertEqual(alloc.mark(PAGE_SIZE * 2, PAGE_SIZE), True)
        item1 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        item2 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.alloc, group)

        # Allocate an item at the bottom of a free list with only one
        # entry.  The second item in the group is at a fixed address
        # too far away for group distance check to succeed.
        # (You could argue that this is a bug and the first item
        # should be allocated at the top of the free list to get at
        # close as possible to the second item.  Should it be fixed?)
        alloc = weaver.allocator.Allocator(PAGE_SIZE)
        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 3, None)
        item1 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        item2 = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE,
                                                     addr = PAGE_SIZE * 4)
        group = weaver.allocator.AllocatorGroup(0, [item1, item2])
        self.assertRaises(weaver.allocator.AllocatorException,
                          alloc.alloc, group)
        
    def test_abstract_tracker(self):
        """
        Test that the abstract AllocatorTracker class raises
        exceptions.
        """
        alloc = weaver.allocator.Allocator(PAGE_SIZE,
                                           weaver.allocator.AllocatorTracker())

        alloc.add_memory(PAGE_SIZE, PAGE_SIZE * 1024, None)

        self.assertRaises(NotImplementedError, alloc.mark,
                          PAGE_SIZE, PAGE_SIZE)

        item = weaver.allocator.AllocatorSimpleItem(size = PAGE_SIZE)
        group = weaver.allocator.AllocatorGroup(0, [item])
        self.assertRaises(NotImplementedError, alloc.alloc,
                          group)
