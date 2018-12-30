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

"""
Properties of the target machine.
"""

import math
from weaver import MergeError
from weaver.device import PhysicalDevice
from weaver.bootinfo_elf import BI_DEFAULT_MEMORY, BI_CACHED_MEMORY, \
     BI_UNCACHED_MEMORY, BI_WRITE_BACK_MEMORY, BI_WRITE_THROUGH_MEMORY, \
     BI_COHERENT_MEMORY, BI_IO_MEMORY, BI_IO_COMBINED_MEMORY

def _assert_no_overlaps(mem_map):
    """Raise an exception if any of the memory regions in the map overlap."""
    # Extract all of the memory regions and record where they came from.
    # Sort by base address
    mem_array = sorted([(base, size, name)
                        for (name, mem) in mem_map.items()
                        for (base, size, mem_type) in mem])

    highest_memory = -1L
    highest = ()

    # Check for overlaps and give a meaningful error message if there
    # is one.
    for (base, size, name) in mem_array:
        if base <= highest_memory:
            raise MergeError, \
                  'The machine memory region 0x%x--0x%x (size 0x%x) in ' \
                  '"%s" overlaps with region 0x%x--0x%x (size 0x%x) in ' \
                  '"%s".' % (base, base + size - 1, size, name,
                             highest[0], highest[0] + highest[1] -1,
                             highest[1], highest[2])
        else:
            highest_memory  = base + size - 1
            highest = (base, size, name)

class Machine:
    """Description of the image's target machine."""

    def __init__(self):
        # Default if no machine is given in XML
        self.page_sizes      = [0x1000]
        self.word_size       = 32
        self.virtual_mem     = {}
        self.physical_mem    = {}
        self.physical_device = {}
        self.physical_memory = []
        self._heap_proximity  = 64 * 1024 * 1024 # 64M is the ARM requirement.
        self.kernel_heap_align  = None

        # Map between cache policy strings and L4 attribute types.
        self.cache_policies = {
            'default'        : BI_DEFAULT_MEMORY,
            'cached'         : BI_CACHED_MEMORY,
            'uncached'       : BI_UNCACHED_MEMORY,
            'writeback'      : BI_WRITE_BACK_MEMORY,
            'writethrough'   : BI_WRITE_THROUGH_MEMORY,
            'coherent'       : BI_COHERENT_MEMORY,
            'device'         : BI_IO_MEMORY,
            'writecombining' : BI_IO_COMBINED_MEMORY
            }

    def set_kernel_heap_proximity(self, distance):
        """
        The maximum distance, in bytes, that the kernel heap can be
        from the kernel.  If the distance is None, then the current
        value is not changed.
        """
        if distance is not None:
            self._heap_proximity = distance

    def get_kernel_heap_proximity(self):
        """
        Return the maximum distance, in bytes, that the kernel heap
        can be from the kernel.
        """
        return self._heap_proximity
    kernel_heap_proximity = property(get_kernel_heap_proximity, set_kernel_heap_proximity)

    def set_page_sizes(self, sizes):
        """Set the allowed page sizes for the machine."""
        self.page_sizes = sizes[:]

        # Sort in descending order to make min_page_size() and
        # superpage_alignment's lives easier.
        self.page_sizes.sort(key=lambda x: -x)

    def add_cache_policies(self, policies):
        """Add extra cache policy values to the default map."""
        for (name, value) in policies:
            self.cache_policies[name] = value

    def get_cache_policy(self, attr):
        """
        Return the numeric value of a named cache policy or raise
        an exception.
        """
        try:
            val = self.cache_policies[attr]
        except:
            raise MergeError, ("Unknown cache policy: '%s'." % attr)

        return val

    def add_virtual_mem(self, name, mem):
        """Add a named list of virtual memory ranges."""
        self.virtual_mem[name] = mem
        _assert_no_overlaps(self.virtual_mem)

    def add_physical_mem(self, name, mem):
        """Add a named list of physical memory ranges."""
        self.physical_mem[name] = mem
        _assert_no_overlaps(self.physical_mem)

        self.physical_memory.extend(mem)
        self.physical_memory.sort()

    def add_phys_device(self, name):
        """Add a named physical device."""
        self.physical_device[name] = PhysicalDevice(name)
        return self.physical_device[name]

    def get_virtual_memory(self, name):
        """Get the names list of virtual memory ranges."""
        if not self.virtual_mem.has_key(name):
            raise MergeError, "Virtual memory called %s not found." % name

        return self.virtual_mem[name]

    def get_physical_memory(self, name):
        """Get the names list of physical memory ranges."""
        # First look for the physical memory in devices...
        for dev in self.physical_device.itervalues():
            pm = dev.get_physical_mem(name)
            if pm is not None:
                return pm

        # ... then try to look for it in the machine
        if not self.physical_mem.has_key(name):
            raise MergeError, "Physical memory called %s not found." % name

        return self.physical_mem[name]

    def min_page_size(self):
        """Return the smallest allowable page size."""
        return self.page_sizes[-1]

    def max_page_size(self):
        """Return the largest allowable page size."""
        return self.page_sizes[0]

    def superpage_alignment(self, size):
        """Calculate the biggest alignment supported on the current
        machine for the given size."""
        alignment = None
            
        for i in self.page_sizes:
            if i <= size:
                alignment = i
                break

        if alignment is None:
            alignment = self.page_sizes[-1]

        return alignment
        
    def natural_alignment(self, size):
        """
        Return the natural alignment for the size.  Eg 64K regions are
        are aligned to 64K boundaries.

        Items smaller than the min page size are min page size aligned
        and items larger than the max page size are max page sise
        aligned.
        """

        # Align small objects to the page size.
        if size <= self.min_page_size():
            return self.min_page_size()

        # Align large items to the max page size.
        if size > self.max_page_size():
            return self.max_page_size()

        return 2 << (int(math.log(size, 2)) - 1)

