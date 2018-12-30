##############################################################################
# Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
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

"""Manage virtual and physical memory pools."""

from elf.util import align_down, align_up
from weaver import MergeError
from weaver.allocator import Allocator, AllocatorException, \
     AllocatorSimpleItem, AllocatorGroup, AllocatorTracker

class Pool(Allocator):
    def __init__(self, name, min_alloc_size, tracker = None):
        Allocator.__init__(self, min_alloc_size, tracker)
        self.name   = name
        self.direct = False

    def get_name(self):
        """Return the name of the pool."""
        return self.name

    def set_direct(self, direct):
        """
        Set whether or not the pool will be used for direct
        virtual<->physical addressing.

        This is only set for pools of physical addresses.
        """
        self.direct = direct

    def is_direct(self):
        """
        Return whether or not the pool will be used for direct
        virtual<->physical addressing.
        """
        return self.direct

class Zone(Pool):
    # Use a one megabyte region for all windows.
    WINDOW_SIZE = 1024 * 1024

    def __init__(self, name, min_alloc_size, tracker = None):
        Pool.__init__(self, name, min_alloc_size, tracker)

        self.pool    = None
        self.windows = []

    def prime_direct(self, ranges, pools):
        """Does not work for a direct memsection in an *existing* window."""
        direct = pools.get_virtual_pool_by_name('direct')
        process_ranges = []

        # Remove from the list any ranges that are already in a
        # window.
        for (base, size) in ranges:
            end = base + size - 1

            for window_base, window_end in self.windows:
                if (base >= window_base and base <= window_end) or \
                       (end >= window_base and end <= window_end):
                    break
            else:
                process_ranges.append((base, size))

        if len(process_ranges) != 0:
            self.prime_windows(direct, process_ranges)

    def prime(self, pool, ranges, pools):
        self.pool = pools.get_virtual_pool_by_name(pool)
        self.prime_windows(self.pool, ranges)

    def prime_windows(self, pool, ranges):
        # Sort ranges to find those that are in the same window.
        ranges.sort(key=lambda x: x[0])

        # Hack: Fails if there is more than one range in a single window.
        holes     = []
        hole_base = None
        window_size = None

        for (base, size) in ranges:
            range_base = align_down(base, self.WINDOW_SIZE)

            if hole_base is None:
                assert len(holes) == 0

                hole_base = range_base
                window_size = max(align_up(size, self.WINDOW_SIZE),
                                  self.WINDOW_SIZE)
                holes.append((base, size))
            elif range_base == hole_base:
                window_size = max(align_up(size, self.WINDOW_SIZE),
                                  self.WINDOW_SIZE)
                holes.append((base, size))
            else:
                assert len(holes) is not 0
                window_size = align_up(size, self.WINDOW_SIZE)

                free_mem = \
                         self.pool.mark_window(hole_base, window_size,
                                               holes)

                # Add the free parts of the window to our freelist.
                for (free_base, free_end, mem_type) in free_mem:
                    self.add_memory(free_base, free_end - free_base +
                                    1, mem_type)

                # Add the window to the list of claimed windows. 
                self.windows.append((hole_base,
                                     hole_base + self.WINDOW_SIZE - 1))

                hole_base = range_base
                window_size = max(align_up(size, self.WINDOW_SIZE),
                                  self.WINDOW_SIZE)
                holes = [(base, size)]

        # Clean up
        if hole_base is not None:
            assert len(holes) is not 0
            free_mem = \
                     pool.mark_window(hole_base, window_size,
                                      holes)

            # Add the free parts of the window to our freelist.
            for (free_base, free_end, mem_type) in free_mem:
                self.add_memory(free_base, free_end - free_base + 1, mem_type)

            # Add the window to the list of claimed windows. 
            self.windows.append((hole_base,
                                 hole_base + self.WINDOW_SIZE - 1))

    def get_windows(self):
        """Return the zone's windows."""
        return self.windows

    def alloc(self, group):
        """
        Override the standard implementation.

        If the allocation fails, then grab another window from the underlying
        pool and try again.

        Therefore allocating anything larger than the window size is
        problematic in a zone.
        """

        try:
            Pool.alloc(self, group)
        except AllocatorException:
            item = AllocatorSimpleItem(name = self.get_name() + "/zone_window",
                                       size = Zone.WINDOW_SIZE,
                                       alignment = Zone.WINDOW_SIZE)

            self.tracker.am_marking_zone()
            self.pool.alloc(AllocatorGroup(None, [item],
                                           "Zone window."))
            self.add_memory(item.get_addr(), item.get_size(), None)

            # Record the new zone window.
            self.windows.append((item.get_addr(),
                                 item.get_addr() + Zone.WINDOW_SIZE - 1))

            Pool.alloc(self, group)
        

class AllocatedMemoryTracker (AllocatorTracker):
    """Class for keeping track of memory allocation."""
    def __init__(self, title):
        AllocatorTracker.__init__(self)

        self.mem            = []
        self.title          = title
        self.alloc_name     = "unknown"
        self.marking_direct = False
        self.marking_zone   = False

    def track_mark(self, base, end):
        """
        Record memory that has been marked in the allocator.

        This function will be called by mark().

        Window allocations are not tracked at all.
        """
        # If a region of direct virtual memory is being removed from
        # the pools then do nothing.
        if self.marking_direct:
            self.marking_direct = False
        else:
            self.__add(self.alloc_name, base, end, can_match_exact = True)

    def track_alloc(self, name, base, end):
        """
        Record memory that has been allocated.

        This method is called once for each item in an allocator
        group.

        """
        # If a new zone window is being allocated then do nothing.
        if self.marking_zone:
            self.marking_zone = False
        else:
            self.__add(name, base, end)

    def set_alloc_name(self, name):
        """Set the name to use for the next region of marked memory."""
        self.alloc_name = name

    def am_marking_direct(self):
        """
        Indicate that the next call to track_mark() relates to moving
        a region of virtual memory to the direct pool, and can be
        ignored.
        """
        self.marking_direct = True

    def am_marking_zone(self):
        """
        Indicate that the next call to track_alloc() relates to the
        allocation of a new window for a zone and can be ignored.
        """
        self.marking_zone = True

    def add_size(self, name, base, size):
        """Record memory usage [base, base + size - 1]"""
        end = base + size - 1
        self.__add(name, base, end)

    def check(self, name, base, size, can_match_exact = False):
        """
        Check if the memory region overlaps with any tracked memory.

        Raises a MergeError if there is an overlap.
        """
        self.__check(name, base, base + size - 1, can_match_exact)

    def __check(self, name, base, end, can_match_exact = False):
        """
        Internal function to check if a memory region overlaps with
        any of the tracked memory.
        """
        size = end - base + 1

        # Check for overlaps and give a meaningful error message if there
        # is one.
        for (alloc_name, alloc_base, alloc_end) in self.mem:
            if base == alloc_base and end == alloc_end and can_match_exact:
                continue
            if (base >= alloc_base and base <= alloc_end) or \
                   (end >= alloc_base and end <= alloc_end):
                raise MergeError, \
                      '%s: Parts of "%s" (0x%x-0x%x, size 0x%x) overlap with ' \
                      '"%s" (0x%x-0x%x, size 0x%x).' % \
                      (self.title, name, base, base + size - 1, size,
                       alloc_name, alloc_base, alloc_end,
                       alloc_end - alloc_base + 1) 

    def __add(self, name, base, end, can_match_exact = False):
        """
        Record the memory usage and raise an exception if the range
        overlaps an existing allocation.

        If can_match_exact is True, then is a range overlaps an
        existing range exactly, then no error is raised.
        """

        self.__check(name, base, end, can_match_exact)

        self.mem.append((name, base, end))
        self.mem.sort()
        self.alloc_name = "unknown"
        

#---------------------------------------------------------------------------
# POOLS
#---------------------------------------------------------------------------

class Pools:
    """A collection of physical and virtual memory pools."""

    def __init__(self):
        self.virtual_pools      = {}
        self.physical_pools     = {}
        # Ensure no overlaps in memory pools or memory allocations
        # across pools.
        self.virtual_pool_mem   = AllocatedMemoryTracker("Virtual Pools")
        self.physical_pool_mem  = AllocatedMemoryTracker("Physical Pools")
        self.allocated_virtual  = AllocatedMemoryTracker("Virtual Memory")
        self.allocated_physical = AllocatedMemoryTracker("Physical Memory")

    def get_virtual_pools(self):
        """Return all virtual pools."""
        return self.virtual_pools.values()

    def get_physical_pools(self):
        """Return all physical pools."""
        return self.physical_pools.values()

    def get_virtual_pool_by_name(self, name):
        """Return the virtual pool with the given name."""
        return self.virtual_pools[name]

    def get_physical_pool_by_name(self, name):
        """Return the physical pool with the given name."""
        return self.physical_pools[name]

    def remove_virtual(self, base, size):
        """
        Remove a region of virtual memory.
        """
        marked = False
        if base is None:
            return marked
        else:
            self.allocated_virtual.am_marking_direct()

            for alloc in self.virtual_pools.values():
                marked = marked or alloc.mark(base, size)
        return marked

    def mark_direct_zone(self, name, range_name, base, size, pools):
        """
        If <name> is a Zone pool then tell the zone to grab the window
        around the given address range.  This is necessary then
        allocating direct memsections because the allocation of direct
        memory does not follow the normal allocation patterns.
        """
        self.allocated_virtual.set_alloc_name(range_name)

        if name is None:
            raise MergeError, "No virtual pool/zone specified."

        if not self.virtual_pools.has_key(name):
            raise MergeError, 'Virtual pool/zone "%s" not found.' % name

        if isinstance(self.virtual_pools[name], Zone):
            self.virtual_pools[name].prime_direct([(base, size)], pools)

    def mark_virtual(self, range_name, base, size):
        """
        Mark a region of virtual memory as used.
        """
        marked = False
        if base is None:
            return marked
        else:
            # Unlike for physical addresses, it is not an error for
            # the range to be outside every pool.  This is chiefly
            # because the kernel virtual addresses will always be
            # outside every pool.
            self.allocated_virtual.set_alloc_name(range_name)

            for (name, alloc) in self.virtual_pools.items():
                try:
                    if alloc.mark(base, size):
                        marked = True
                except AllocatorException, text:
                    raise MergeError, 'Virtual pool "%s": %s' % (name, text)

            if not marked:
                self.allocated_virtual.check(range_name, base, size,
                                             can_match_exact=True)

        return marked

    def mark_physical(self, range_name, base, size):
        """
        Mark a region of physical memory as used.  The region must
        exist in a pool.
        """
        marked = False
        if base is None:
            return marked
        else:
            self.allocated_physical.set_alloc_name(range_name)

            for (name, alloc) in self.physical_pools.items():
                try:
                    if alloc.mark(base, size):
                        marked = True
                except AllocatorException, text:
                    raise MergeError, 'Physical pool "%s": %s' % (name, text)

            if not marked:
                self.allocated_physical.check(range_name, base, size,
                                              can_match_exact=True)

        return marked

    def alloc_virtual(self, name, group):
        """
        Allocate memory for a group of items from a virtual pool.
        """
        if name is None:
            raise MergeError, "No virtual pool/zone specified."

        if not self.virtual_pools.has_key(name):
            raise MergeError, 'Virtual pool/zone "%s" not found.' % name

        try:
            self.virtual_pools[name].alloc(group)
        except AllocatorException, text:
            raise MergeError, 'Physical pool "%s": %s' % (name, text)

    def alloc_physical(self, name, group):
        """
        Allocate memory for a group of items from a physical pool.
        """
        if name is None:
            raise MergeError, "No physical pool/zone specified."

        if not self.physical_pools.has_key(name):
            raise MergeError, 'Physical pool/zone "%s" not found.' % name

        try:
            self.physical_pools[name].alloc(group)
        except AllocatorException, text:
            raise MergeError, 'Physical pool "%s": %s' % (name, text)

    def is_physical_direct(self, name):
        """
        Return whether or not the physical pool is a direct pool.
        """
        if name is None:
            raise MergeError, "No physical pool/zone specified."

        if not self.physical_pools.has_key(name):
            raise MergeError, 'Physical pool/zone "%s" not found.' % name

        return self.physical_pools[name].is_direct()

    def new_virtual_pool(self, name, machine):
        """Create a new virtual pool."""
        if self.virtual_pools.has_key(name):
            raise MergeError, 'Virtual pool "%s" already exists.' % name

        alloc = Pool(name, machine.min_page_size(),
                     self.allocated_virtual)
        self.virtual_pools[name] = alloc

        return alloc

    def new_zone(self, name, machine):
        """Create a new zone."""
        # Zones and virtual pools share the same address space, so they share
        # the same namespace.
        if self.virtual_pools.has_key(name):
            raise MergeError, 'Zone  "%s" already exists.' % name

        alloc = Zone(name, machine.min_page_size(),
                     self.allocated_virtual)
        self.virtual_pools[name] = alloc

        return alloc

    def new_physical_pool(self, name, machine):
        """Create a new physical pool."""
        if self.physical_pools.has_key(name):
            raise MergeError, 'Physical pool "%s" already exists.' % name

        alloc = Pool(name, machine.min_page_size(),
                     self.allocated_physical)
        self.physical_pools[name] = alloc

        return alloc

    def add_virtual_memory(self, virtpool, machine, src = None, base = None,
                           size = None):
        """Add memory to the freelist of a pool."""
        # Convert the free list into (base, end) pairs.
        if src is not None:
            regions = machine.get_virtual_memory(src)
        else:
            assert(base is not None and size is not None)

            regions = [(base, size, None)]

        for (base, size, mem_type) in regions:
            self.virtual_pool_mem.add_size(virtpool, base, size)
            self.virtual_pools[virtpool].add_memory(base, size, mem_type)

    def add_physical_memory(self, physpool, machine, src = None, base = None,
                           size = None):
        """Add memory to the freelist of a pool."""
        # Convert the free list into (base, end) pairs.
        if src is not None:
            regions = machine.get_physical_memory(src)
        else:
            assert(base is not None and size is not None)

            regions = [(base, size, None)]

        for (base, size, mem_type) in regions:
            self.physical_pool_mem.add_size(physpool, base, size)
            self.physical_pools[physpool].add_memory(base, size, mem_type)

    def add_direct_memory(self, base, size, mem_type):
        """Add memory to the freelist of a pool."""
        self.remove_virtual(base, size)
        self.virtual_pools["direct"].add_memory(base, size, mem_type)

    def print_last_phys(self):
        """
        Print next address to be allocated from each physical pool.
        The pools are sorted to give a consistent output.
        """
        for (name, alloc) in sorted(self.physical_pools.items()):
            print "%s: 0x%x" % (name, alloc.next_avail())

