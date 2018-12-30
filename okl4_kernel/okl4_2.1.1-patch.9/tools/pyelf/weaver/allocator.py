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

"""
                   Memory allocator for elfweaver.

   This module is used to keep track of the memory in physical and
                            virtual pools.



               Lasciate ogne speranza, voi ch~intrate.

             (Google it if you don't know the reference)

"""

from elf.util import align_up, align_down

class AllocatorException(Exception):
    """Exception thrown when an item or group could not be allocated."""
    pass


class AllocatorGroupException(Exception):
    """
    Exception thrown when there are problems allocating a group of
    items.

    This is an internal exception and should not be seen by users of
    the allocator class.
    """
    pass


class AllocatorItem(object):
    """
    Abstract class for describing an item that is part of an
    allocation group.
    """

    def get_name(self):
        """Return a text description of the item."""
        raise NotImplementedError

    def get_size(self):
        """Return the size of the item ."""
        raise NotImplementedError

    def get_alignment(self):
        """Return the desired alignment of the item."""
        raise NotImplementedError

    def get_offset(self):
        """Return the desired offset into the page of the item."""
        return 0

    def get_addr(self):
        """
        Return the address of the object.
        Returns None if the address has not been set.
        """
        raise NotImplementedError

    def set_addr(self, addr):
        """Set the address of the object to the given value."""
        raise NotImplementedError
    
class AllocatorSimpleItem(AllocatorItem):
    """
    Simple concrete version of AllocatorItem.
    """
    def __init__(self, size, alignment = None, offset = 0,
                 addr = None, name = "unknown_item"):
        AllocatorItem.__init__(self)

        self.size      = size
        self.alignment = alignment
        self.offset    = offset
        self.addr      = addr
        self.name      = name

    def get_name(self):
        """Return a text description of the item."""
        return self.name

    def get_size(self):
        """Return the size of the item ."""
        return self.size

    def get_alignment(self):
        """Return the desired alignment of the item."""
        return self.alignment

    def get_offset(self):
        """Return the desired offset into the page of the item."""
        return self.offset

    def get_addr(self):
        """
        Return the address of the object.
        Returns None if the address has not been set.
        """
        return self.addr

    def set_addr(self, addr):
        """Set the address of the object to the given value."""
        self.addr = addr

class AllocatorGroup(object):
    """
    A collection of items that are to be allocated together.

    Each item in the group is allocated according to its own alignment
    and size requirements.  Further, each item will be placed no
    further than <distance> bytes from its neighbour.
    """
    def __init__(self, distance, entries = None, error_message = None):
        self.distance      = distance
        if error_message is None:
            self.error_message = "Could not place %(this_item)s within " \
                                 "%(distance)#x bytes of %(last_item)s." 
        else:
            self.error_message = error_message

        if entries is None:
            self.entries = []
        else:
            self.entries = entries

    def get_distance(self):
        """
        Get the maximum distance between items in bytes.

        If the distance is None, then there are no constraints on
        where the items can be placed.
        """
        return self.distance

    def get_entries(self):
        """Return a sequence of the items in the group."""
        return self.entries

    def get_error_message(self):
        """Return the error message to display if the allocation fails."""
        return self.error_message

class AllocatorTracker(object):
    """
    Abstract class for keeping track of memory used by an allocator.

    Such a class could check for overlapping memory allocations.
    """

    def track_mark(self, base, end):
        """
        Record memory that has been marked in the allocator.

        This function will be called by mark().

        Window allocations are not tracked at all.
        """
        raise NotImplementedError

    def track_alloc(self, name, base, end):
        """
        Record memory that has been allocated.

        This method is called once for each item in an allocator
        group.

        """
        raise NotImplementedError

class Allocator(object):
    """
    A memory allocator.

    This allocator is based on the first fit algorithm with an
    interface tuned to the rather strange requirements of elfweaver.

    The allocator keeps a sorted list of free address ranges.  Each
    range is a multiple of <min_alloc_size> bytes (aka a page), and is
    aligned on a <min_alloc_size> boundary.

    Separate calls to the allocator will result in items being
    allocated in different pages.  However it is possible to allocate
    items on adjacent bytes, provided that that are part of the same
    allocation group.

    Allocation groups are a method of allocating multiple items at one
    time and ensuring that they are placed sufficiently close to one
    another.  Some items in a group may already have their addresses
    set (for instance, allocating a stack near a program's text
    segment).  These items must have addresses that are not in the
    allocator's free list.

    An individual item will be placed at the lowest address that
    satify its size, offset and alignment requirements.  Items in a
    group are allocated in increasing order of address.

    Memory at a particular address can be removed from the free list
    with the mark() method.  It is not an error to mark memory that is
    wholly or in part missing from the free list.

    A window of memory can also be marked with the mark_window()
    method.  A window is similar to regular marking, except that it is
    an error for the region of memory to contain memory outside of the
    freelist unless those regions are present in a list supplied to
    the method.  The entire memory region must have once been present
    in the allocator's freelist.  Windows are used to implement
    zones.

    """

    def __init__(self, min_alloc_size, tracker = None):
        """
        __init__(min_alloc_size)

        min_alloc_size is the smallest size, in bytes, that can be
        allocated.  Alignment must be a multiple of this size.
        """

        # Alloc size must be a power of 2.
        assert (min_alloc_size & (min_alloc_size - 1)) == 0

        self.freelist  = []
        self.fulllist  = []
        self.min_alloc = min_alloc_size
        self.tracker   = tracker

    def __merge(self, the_list):
        """
        Merge adjacent regions into one.

        Merging regions simplifies the logic of the rest of the
        allocation code.
        """
        last     = None
        new_list = []

        for region in the_list:
            if last is None:
                last = region
            else:
                if last[1] == region[0] - 1:
                    last = (last[0], region[1], last[2])
                else:
                    new_list.append(last)
                    last = region

        if last is not None:
            new_list.append(last)

        return new_list
        
    def sort(self):
        """
        Sort the free and full memory lists and merge any adjacent
        memory regions.
        """
        self.freelist.sort(key=lambda x: x[0])
        self.freelist = self.__merge(self.freelist)

        self.fulllist.sort(key=lambda x: x[0])
        self.fulllist = self.__merge(self.fulllist)


    def get_freelist(self):
        """Return the current free list."""
        return self.freelist


    def add_memory(self, base, size, mem_type):
        """
        Add a region of free memory to the pool.

        The memory must be ia multiple of <min_alloc_size> bytes in
        size, and aligned on a <min_alloc_size> boundary.

        Memory addresses may only be added to the allocator once. 
        """
        # Check that the memory is page aligned.
        assert align_down(base, self.min_alloc) == base
        assert size % self.min_alloc == 0

        end = base + size - 1

        # Check that the memory does not overlap any of the regions
        # already in use.
        for list_base, list_end, list_mem_type in self.fulllist:
            if (base >= list_base and base <= list_end) or \
               (end >= list_base and end <= list_end):
                raise AllocatorException, \
                      "Cannot add overlapping memory regions to the " \
                      "Allocator.  Address (0x%x--0x%x) already in " \
                      "(0x%x--0x%x)" % \
                      (base, end, list_base, list_end)

        self.fulllist.append((base, end, mem_type))
        self.freelist.append((base, end, mem_type))
        self.sort()


    def mark(self, base, size):
        """
        mark(base, end) -> marked
        
        Remove the given range from the free list.  This is used to
        record were fixed address objects are located.

        Returns whether or not the range was removed from the free list.

        It is *not* an error if the range is wholly outside the
        freelist.
        """

        end = base + size - 1

        if end < base:
            raise AllocatorException, \
                  "Mark end address (0x%x) less than mark base address (0x%x)" \
                  % (end, base)

        if base == end:
            end = end + 1

        # Remove all of the pages containing the region.
        base = align_down(base, self.min_alloc)
        end  = align_up(end, self.min_alloc) - 1

        new_freelist = []
        marked       = False

        for free_start, free_end, mem_type in self.freelist:
            assert free_start % self.min_alloc == 0
            assert (free_end + 1) % self.min_alloc == 0

            if (free_start <= base and base <= free_end) or \
                   (base <= free_start and free_start <= end):
                marked = True

                # Report the allocation to the tracker, if there is
                # one.
                if self.tracker is not None:
                    self.tracker.track_mark(base, end)

                # Insert into the freelist any remaining parts of the
                # free memory region.
                if free_start < base:
                    new_freelist.append((free_start, base - 1, mem_type))

                if end < free_end:
                    new_freelist.append((end + 1, free_end, mem_type))
            else:
                new_freelist.append((free_start, free_end, mem_type))

        self.freelist = new_freelist
                
        return marked


    def mark_window(self, window_base, size, holes):
        """
        Mark a window of memory.

        A window is region of memory whose addresses must be either in
        the free list or listed in the holes list.  Every address in
        the region must have been present in the freelist at some
        stage.

        The holes variable is a list of (base, size) tuples.

        Returns the parts of the freelist that were removed.
        """
        if holes is None:
            holes = []

        window_end = window_base + size - 1

        if window_end < window_base:
            raise AllocatorException, \
                  "alloc_window: Window end address (0x%x) less " \
                  "than mark base address (0x%x)" % \
                  (window_end, window_base)

        if window_base == window_end:
            window_end = window_end + 1

        window_base = align_down(window_base, self.min_alloc)
        window_end  = align_up(window_end, self.min_alloc) - 1

        # First check that the proposed window is in the memory that
        # was originally passed to add_memory().
        contained      = False

        for full_start, full_end, mem_type in self.fulllist:
            # If the window is fully contained within one record, then
            # we're sweet.
            if full_start <= window_base <= full_end and \
               full_start <= window_end <=  full_end:
                contained = True
                break

        # OK, what's the answer?
        if not contained:
            raise AllocatorException, \
                  "alloc_window: Window not in allocator controlled memory." 

        # Transform the hole list from (base, size) to (base, end),
        # rounded to page boundaries, and sort in increasing order of
        # address.
        holes = [(align_down(hole_base, self.min_alloc),
                  align_up(hole_base + hole_size - 1, self.min_alloc) - 1)
                 for (hole_base, hole_size) in holes]
        holes.sort(key=lambda x: x[0])

        # Holes must be in the range of the window and can't overlap.
        for hole_base, hole_end in holes:
            assert window_base <= hole_base <= window_end and \
                   window_base <= hole_end <= window_end

        free_iter  = iter(self.freelist)
        hole_iter  = iter(holes)
        curr_addr  = window_base
        free_done  = False
        holes_done = False
        curr_free = None
        curr_hole = None
        new_freelist = []
        removed_freelist = []

        while curr_addr <= window_end:
            assert curr_addr % self.min_alloc == 0

            if not free_done and curr_free is None:
                try:
                    curr_free = free_iter.next()

                    # If the freelist range is outside where we are
                    # working, then loop and get another one.
                    if curr_free[1] < curr_addr or \
                       curr_free[0] > window_end:
                        new_freelist.append(curr_free)
                        curr_free = None
                        continue
                except StopIteration:
                    free_done = True
                    curr_free = None

            if not holes_done and curr_hole is None:
                try:
                    curr_hole = hole_iter.next()
                except StopIteration:
                    holes_done = True
                    curr_hole = None

            if curr_free is not None and \
                   curr_free[0] <= curr_addr and \
                   curr_free[1] >= curr_addr:
                if curr_hole is not None and \
                       curr_hole[0] <= curr_free[1]:
                    raise AllocatorException, \
                          "alloc_window: Hole (0x%x-0x%x) overlaps " \
                          "with free block (0x%x-0x%x)." % \
                          (curr_hole[0], curr_hole[1], curr_free[0],
                           curr_free[1])
                else:
                    # Remove the part we're interested in from the
                    # freelist.  Add the excess.
                    if curr_free[0] < curr_addr:
                        new_freelist.append((curr_free[0], curr_addr -
                                             1, curr_free[2]))

                    if curr_free[1] > window_end:
                        new_freelist.append((window_end + 1,
                                             curr_free[1], curr_free[2]))
                        removed_freelist.append((curr_addr,
                                                 window_end, curr_free[2]))
                    else:
                        removed_freelist.append((curr_addr,
                                                 curr_free[1], curr_free[2]))

                    curr_addr = curr_free[1] + 1
                    curr_free = None
            elif curr_hole is not None and \
                     curr_hole[0] == curr_addr:
                if curr_free is not None and \
                       curr_free[0] <= curr_hole[1]:
                    raise AllocatorException, \
                          "alloc_window: Hole (0x%x-0x%x) overlaps " \
                          "with free block (0x%x-0x%x)." % \
                          (curr_hole[0], curr_hole[1], curr_free[0],
                          curr_free[1])
                else:
                    curr_addr = curr_hole[1] + 1
                    curr_hole = None
            else:
                raise AllocatorException, \
                      "Address %#x should be in a zone but is neither " \
                      "free or in an already allocated block.  Is it part " \
                      "of a direct addressing pool?" % \
                      curr_addr 

        # Copy any remaining free list records into the new freelist.
        for curr_free in free_iter:
            new_freelist.append(curr_free)

        self.freelist = new_freelist

        return removed_freelist

    def __simple_alloc(self, freelist, size, alignment, offset):
        """
        Allocate a single block of memory or raise an exception.

        Search for a place in the freelist to allocate the requested
        block of memory.

        Returns a tuple with the following values:

        - before_free - The freelist up to the place where the memory
          was allocated.
        - after_free  - The freelist after the place where the memory
          was allocated
        - addr        - The allocated addresses.

        The free lists are not necessarily page aligned.

        Returning the tuple allows the abort and retry semantics of
        group allocation to be implemented.
        """
        before_free = []
        after_free  = []
        addr        = None
        free_iter   = iter(freelist)

        # Search the freelist for a suitable block.
        for (free_start, free_end, mem_type) in free_iter:
            # Calculate the proposed address.
            alloc_start = align_down(free_start, alignment) + offset
            alloc_end   = alloc_start + size - 1

            # If alignment adjustments push the block below the
            # start of the free region, bump it up.
            if alloc_start < free_start:
                alloc_start += alignment
                alloc_end += alignment

            # If the range is within free memory, we've found it.
            if alloc_start >= free_start and alloc_end <= free_end:
                # Put the remaining parts of the region back into the
                # correct freelists.
                if free_start < alloc_start - 1:
                    before_free.append((free_start, alloc_start - 1, mem_type))

                if alloc_end + 1 < free_end:
                    after_free.append((alloc_end + 1, free_end, mem_type))

                addr = alloc_start
                break
            else:
                # Not useful, so add to the before list.
                before_free.append((free_start, free_end, mem_type))

        # Abort if nothing suitable was found.
        if addr is None:
            raise AllocatorException, "Out of memory"

        # Copy any remaining free list records into the after_freelist
        for curr_free in free_iter:
            after_free.append(curr_free)

        return (before_free, after_free, alloc_start)

    def __group_alloc(self, group, freelist):
        """
        Allocate memory for a group of items and ensure that they have
        been allocated sufficiently close together.

        Items in the group are allocted with increasing addresses.

        Returns a tuple with the following values:

        new_freelist - The revised freelist
        addrs        - A list of the addresses of the items, in item
                       order.
        """
        addrs        = []
        new_freelist = []
        last_item    = None

        for i in group.get_entries():
            # If the address is not fixed, then allocate
            if i.get_addr() is None:
                (before, freelist, addr) = \
                         self.__simple_alloc(freelist, i.get_size(),
                                            i.get_alignment(),
                                            i.get_offset())
                new_freelist.extend(before)
            else:
                addr = i.get_addr()

                # Split the freelist around the fixed address to that
                # the next item will be allocated at a higher
                # addresses.  Problems will arise fixed items
                # are placed in the list unsorted.
                updated_freelist = []
                for (base, end, mem_type) in freelist:
                    if end < addr:
                        new_freelist.append((base, end, mem_type))
                    else:
                        assert base > addr
                        updated_freelist.append((base, end, mem_type))

                # Use the freelist above the fixed item for further
                # allocations.
                freelist = updated_freelist

            # Check the distance between the items and throw if
            # they're too far apart.
            if last_item is not None and group.get_distance() is not None:
                if addr - (addrs[-1] + last_item.get_size()) > \
                       group.get_distance():
                    err_txt = {
                        'last_item' : last_item.get_name(),
                        'this_item' : i.get_name(),
                        'distance'  : group.get_distance()
                        }
                    raise AllocatorGroupException, \
                          group.get_error_message() % err_txt

            addrs.append(addr)
            last_item = i

        # Add remaining parts of the freelist to the final freelist.
        new_freelist.extend(freelist)

        return (new_freelist, addrs)

    def alloc(self, group):
        """
        Allocate memory for a group of items.  Items within the group
        will be allocated no more that group.get_distance() bytes
        appart, otherwise an exception will be raised.  
        """
        new_freelist  = []
        curr_freelist = self.freelist
        addrs         = []
        completed     = False

        # How to allocate a group of items:
        #
        # 1) Try to allocate the group from the bottom of the free
        #    list.  If that works, good!
        # 2) If an AllocatorException is thrown, then an individual
        #    item could not find the memory it needs, so give up now
        #    and throw again.
        # 3) If an AllocatorGroupException is thrown, then the group's
        #    distance requirements could now be fulfilled.  This could
        #    be as a result of freelist fragmentation, so remove the
        #    first item from the freelist and try again.
        while not completed:
            try:
                (ret_freelist, addrs) = self.__group_alloc(group, curr_freelist)
            except AllocatorException:
                raise AllocatorException, group.get_error_message()
            except AllocatorGroupException, agex:
                # Remove the first freelist record and try again.  If
                # there is fragmentation in the freelist, this may
                # work around it.  This exception may be raised again
                # if the group failed for another reason (for
                # instance, the could not be satisfied at all).
                if len(curr_freelist) <= 1:
                    raise AllocatorException, str(agex)
                else:
                    new_freelist.append(curr_freelist.pop(0))
            else:
                # Success!
                completed = True

                # Rebuild the freelist.
                new_freelist.extend(ret_freelist)

                # Assign to each item its address.
                for (item, addr) in zip(group.get_entries(), addrs):
                    # Report the allocation to the tracker, if there
                    # is one.  For completely accurate coverage the
                    # pages that were allocated should really be
                    # tracked, but that is:
                    # a) really hard.
                    # b) not what the original version did.
                    if self.tracker is not None and \
                           addr != item.get_addr():
                        self.tracker.track_alloc(item.get_name(), addr,
                                                 addr + item.get_size() - 1)
                    item.set_addr(addr)

        # Restore the invariant by removing all partial pages from the
        # freelist.
        self.freelist = []
        for (base, end, mem_type) in new_freelist:
            if base % self.min_alloc != 0:
                base = align_up(base, self.min_alloc)

            if ((end + 1) % self.min_alloc) != 0:
                end = align_down(end, self.min_alloc) - 1

            # Don't include pages that have been rounded out of
            # existence.
            if base < end:
                self.freelist.append((base, end, mem_type))

    def next_avail(self):
        """
        Return the base address of the biggest block of memory in
        the free list.
        """

        if len(self.freelist) == 0:
            raise AllocatorException, \
                      "next_avail(): Free list empty."

        return max([(end - base, base) for (base, end, _) in self.freelist])[1]

