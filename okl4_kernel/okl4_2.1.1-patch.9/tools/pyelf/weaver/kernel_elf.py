###############################################################################
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
This module provides the interface to the root server descriptor in
the .roinit section.
"""

from sys import stdout
import struct
from elf.ByteArray import ByteArray
from elf.util import IntString
from weaver import MergeError

# ELF class  (Ref: 1-7)
class MemDescType(IntString):
    """IntString for ELF class"""
    _show = {}

MEMDESC_UNDEFINED    = MemDescType(0, "undefined")
MEMDESC_CONVENTIONAL = MemDescType(1, "conventional")
MEMDESC_RESERVED     = MemDescType(2, "reserved")
MEMDESC_DEDICATED    = MemDescType(3, "dedicated")
MEMDESC_NUMA         = MemDescType(4, "numa")
MEMDESC_GLOBAL       = MemDescType(5, "global")
MEMDESC_TRACE        = MemDescType(0xb, "trace")
MEMDESC_INITIAL      = MemDescType(0xc, "initial mapping")
MEMDESC_BOOT         = MemDescType(0xe, "bootloader")
MEMDESC_ARCH         = MemDescType(0xf, "arch specific")

# Table to map memory descriptor strings to memory descriptor
# objects.
MEMDESC_DICT = {
    str(MEMDESC_UNDEFINED)    : MEMDESC_UNDEFINED,
    str(MEMDESC_CONVENTIONAL) : MEMDESC_CONVENTIONAL,
    str(MEMDESC_RESERVED)     : MEMDESC_RESERVED,
    str(MEMDESC_DEDICATED)    : MEMDESC_DEDICATED,
    str(MEMDESC_NUMA)         : MEMDESC_NUMA,
    str(MEMDESC_GLOBAL)       : MEMDESC_GLOBAL,
    str(MEMDESC_TRACE)        : MEMDESC_TRACE,
    str(MEMDESC_INITIAL)      : MEMDESC_INITIAL,
    str(MEMDESC_BOOT)         : MEMDESC_BOOT,
    str(MEMDESC_ARCH)         : MEMDESC_ARCH,
    }

class MemoryDescriptor(object):
    """Class representing an L4 kip memory descriptor"""

    # Number of fields that are not part of a subfield.
    MAX_FIELDS  = 5

    def get_struct_size(wordsize):
        """Return the binary size of this structure."""
        return wordsize / 8  * MemoryDescriptor.MAX_FIELDS

    get_struct_size = staticmethod(get_struct_size)
    
    def __init__(self, memdesc_type = 0, sub_type = 0,
                 virtual = False, low = 0, high = 0):
        self._memdesc_type = MemDescType(memdesc_type)
        self._sub_type     = sub_type
        self._virtual      = virtual
        self._low          = low
        self._high         = high

    def __repr__(self):
        return "MemoryDescriptor: type: %s, low: %#x, high: %#x" % \
               (self._memdesc_type,self._low, self._high)

    def is_physical(self):
        """Return true if the memory described is physical"""
        return not self._virtual

    def is_virtual(self):
        """Return true if the memory described is virtual."""
        return self._virtual

    def get_low(self):
        """Return the low address."""
        return self._low

    def get_high(self):
        """Return the high address."""
        return self._high

    def set_high(self, high):
        """Set the high address."""
        self._high = high

    def get_type(self):
        """Return the memory descriptor type."""
        return self._memdesc_type

    def get_sub_type(self):
        """Return the memory descriptor sub-type."""
        return self._sub_type

    def fromdata(self, wordsize, endianess, data):
        """Initialise the object from data."""
        if wordsize == 32:
            format_str = "IIIII"
        else:
            format_str = "QQQQQ"            

        _low, _high, _subtype, _type, _virtual = \
              struct.unpack(endianess + format_str, data)

        self._memdesc_type = MemDescType(_type)
        self._sub_type     = _subtype
        self._virtual      = _virtual
        self._low          = _low
        self._high         = _high

    def todata(self, wordsize, endianess):
        """Generate raw data from the object."""
        if wordsize == 32:
            format_str = "IIIII"
        else:
            format_str = "QQQQQ"

        return ByteArray(struct.pack(endianess + format_str,
                                     self._low, self._high,
                                     self._sub_type, self._memdesc_type,
                                     self._virtual))

    def is_type_equal(self, desc):
        """
        Return whether or not two descriptors cover the same memory
        region.
        """
        return (self._virtual == desc._virtual and
               self._memdesc_type == desc._memdesc_type and
               self._sub_type == desc._sub_type)

    def is_overlapping(self, desc):
        """
        Return whether or not two descriptors cover operlapping
        memory.
        """
        return (desc._high >= self._low and self._high >= desc._low)

    def merge(self, desc):
        """Merge two memory descriptors together."""
        if (desc._low < self._low):
            self._low = desc._low

        if (desc._high > self._high):
            self._high = desc._high

class MemorySet(object):
    """Set of memory descriptors."""

    # Maximum number of memory descriptors.
    MEMDESC_MAX = 32

    # Number of fields that are not part of a subfield.
    MAX_FIELDS  = 1

    def get_struct_size(wordsize):
        """Return the binary size of this structure."""
        return (wordsize / 8  * MemorySet.MAX_FIELDS) + \
               (MemorySet.MEMDESC_MAX * MemoryDescriptor.get_struct_size(wordsize))

    get_struct_size = staticmethod(get_struct_size)

    def __init__(self):
        self._descs = []

    def add_mem_descriptor(self, new_desc):
        """Add a memory descriptor to the kernel config data."""
        # Merge overlapping entries
        for desc in self._descs:
            if desc.is_type_equal(new_desc):
                if desc.is_overlapping(new_desc):
                    desc.merge(new_desc)
                    return

        # HACK: With the iguana bootinfo scheme it is possible to
        # overflow the KIP's memdesc table if a large number of
        # memsections are included in the image.  If the new
        # descriptor is the same type as the last one and is "close
        # enough" then they are merged to save entries.  The space
        # between the blocks is not entered in the free list.

        close_enough = 0x10000 # Typical min alignment at the moment.

        if len(self._descs) > 0:
            lmd = self._descs[-1]
            if (lmd.is_type_equal(new_desc) and
                new_desc.get_low() > lmd.get_high() and
                new_desc.get_low() - lmd.get_high() < close_enough):
                lmd.set_high(new_desc.get_high())
                return

        self._descs.append(new_desc)

    def fromdata(self, wordsize, endianess, data):
        """Initialise the object from data."""
        if wordsize == 32:
            format_str = "I"
        else:
            format_str = "Q"            

        _num_descs = \
                   struct.unpack(endianess + format_str,
                                 data[0:(wordsize / 8) * self.MAX_FIELDS])

        mem_map_start = (wordsize / 8) * self.MAX_FIELDS

        self._descs = []
        
        for i in range(_num_descs[0]):
            start = mem_map_start + (i * MemoryDescriptor.get_struct_size(wordsize))
            desc_data = data[start:start + MemoryDescriptor.get_struct_size(wordsize)]
            desc = MemoryDescriptor()
            desc.fromdata(wordsize, endianess, desc_data)
            self._descs.append(desc)

    def todata(self, wordsize,  endianess):
        """Initialise the object from data."""
        if wordsize == 32:
            format_str = "I"
        else:
            format_str = "Q"

        num_descs = len(self._descs)

        if num_descs > self.MEMDESC_MAX:
            raise MergeError, "%d memory descriptors " \
            "are being added to the kernel config but the maximum is %d." % \
            (num_descs, self.MEMDESC_MAX)


        data = ByteArray(struct.pack(endianess + format_str,
                                     num_descs))

        for i in range(num_descs):
            data += self._descs[i].todata(wordsize, endianess)

        # Pad unused fields with zeros.
        data += ByteArray('\0' * (MemoryDescriptor.get_struct_size(wordsize) * \
                                  (self.MEMDESC_MAX - num_descs)))

        return ByteArray(data)

    def output(self, wordsize, f=stdout):
        """
        Print an ASCII representation of the root server section to
        the file f.
        """

        def word():
            """Return the correct format string."""
            return "0x%%%s" % word_format

        if wordsize == 32:
            word_format = "08x"
        elif wordsize == 64:
            word_format = "016x"

        print >> f, "  Memory Descriptors:"

        physical = [memdesc for memdesc in self._descs if
                    memdesc.is_physical()]
        virtual = [memdesc for memdesc in self._descs if
                   memdesc.is_virtual()]
        for cat, name in [(physical, "Physical"),
                          (virtual,  "Virtual ")]:
            if cat:
                for i in range(len(cat)):
                    memdesc = cat[i]
                    if i == 0:
                        print >> f, "    %s:    " % name,
                    else:
                        print >> f, "                 ",
                    if memdesc.get_type() == MEMDESC_ARCH and \
                           memdesc.get_sub_type() == 0:
                        print >> f, (word()+" - "+word()+"   kernel heap") % \
                            (memdesc.get_low(), memdesc.get_high())
                        continue
                    print >> f, (word()+" - "+word()+"   %s") % \
                          (memdesc.get_low(), memdesc.get_high(),
                           memdesc.get_type()),
                    if memdesc.get_type() in [MEMDESC_BOOT,
                                              MEMDESC_ARCH,
                                              MEMDESC_INITIAL]:
                        print >> f, " 0x%x" % memdesc.get_sub_type()
                    else:
                        print >> f

class Mapping(object):
    """Class describing a rootserver segment mapping."""

    def get_struct_size(wordsize):
        """Return the binary size of this structure."""
        return wordsize / 8  * 3

    get_struct_size = staticmethod(get_struct_size)

    def __init__(self, mapping = None):
        if mapping is not None:
            self._virt = mapping[0]
            self._phys = mapping[1]
            self._size = mapping[2]
        else:
            self._virt = None
            self._phys = None
            self._size = None

    def get_virt(self):
        """Return the virtual address of the mapping."""
        return self._virt

    def get_phys(self):
        """Return the physical address of the mapping."""
        return self._phys

    def get_size(self):
        """Return the size of the mapping."""
        return self._size

    def fromdata(self, wordsize,  endianess, data):
        """Initialise the object from data."""
        if wordsize == 32:
            format_str = "III"
        else:
            format_str = "QQQ"

        _phys, _virt, _size = struct.unpack(endianess + format_str, data)

        self._virt = _virt
        self._phys = _phys
        self._size = _size

    def todata(self, wordsize,  endianess):
        """Initialise the object from data."""
        if wordsize == 32:
            format_str = "III"
        else:
            format_str = "QQQ"

        return ByteArray(struct.pack(endianess + format_str,
                                     self._phys, self._virt, self._size))

    def __repr__(self):
        return "V: 0x%x P: 0x%x S: 0x%x" % (self._virt, self._phys, self._size)

class RootServerDescriptor(object):
    """Root server initialisation data."""

    # Maximum number of mappings.
    RS_MAX_MAPS = 8

    # Number of fields that are not part of a subfield.
    MAX_FIELDS = 3

    def get_struct_size(wordsize):
        """Return the binary size of this structure."""
        return (wordsize / 8  * RootServerDescriptor.MAX_FIELDS) + \
               (RootServerDescriptor.RS_MAX_MAPS *
                Mapping.get_struct_size(wordsize))

    get_struct_size = staticmethod(get_struct_size)

    def __init__(self):
        self._entry    = None
        self._stack    = None
        self._mappings = []
    
    def set_entry(self, entry):
        """Set the entry point of the root-server."""
        self._entry = entry

    def set_stack(self, stack):
        """Set the stack pointer of the root-server."""
        self._stack = stack

    def add_mapping(self, mapping):
        """Add a rootserver memory mapping."""
        self._mappings.append(Mapping(mapping))

    def fromdata(self, wordsize,  endianess, data):
        """Initialise the object from data."""
        if wordsize == 32:
            format_str = "III"
        else:
            format_str = "QQQ"

        _entry, _stack, _num_maps = \
                struct.unpack(endianess + format_str,
                              data[0:(wordsize / 8) * self.MAX_FIELDS]) 

        self._entry = _entry
        self._stack = _stack
        self._mappings = []

        mem_map_start = (wordsize / 8) * self.MAX_FIELDS

        for i in range(_num_maps):
            start = mem_map_start + (i * Mapping.get_struct_size(wordsize))
            map_data = data[start:start + Mapping.get_struct_size(wordsize)]
            mem_map = Mapping()
            mem_map.fromdata(wordsize, endianess, map_data)
            self._mappings.append(mem_map)

    def todata(self, wordsize,  endianess):
        """Initialise the data from object."""
        if wordsize == 32:
            format_str = "III"
        else:
            format_str = "QQQ"

        num_mappings = len(self._mappings)

        if num_mappings > self.RS_MAX_MAPS:
            raise MergeError, "Error: %d rootserver mappings " \
            "are being added to the kernel config but the maximum is %d." % \
            (num_mappings, self.RS_MAX_MAPS)


        data = ByteArray(struct.pack(endianess + format_str,
                                     self._entry, self._stack, num_mappings))

        for i in range(num_mappings):
            data += self._mappings[i].todata(wordsize, endianess)

        # Pad unused fields with zeros.
        data += ByteArray('\0' * (Mapping.get_struct_size(wordsize) * \
                                  (self.RS_MAX_MAPS - num_mappings)))

        return ByteArray(data)

    def output(self, wordsize, f=stdout):
        """
        Print an ASCII representation of the root server section to
        the file f.
        """

        def word():
            """Return the correct format string."""
            return "0x%%%s" % word_format

        if wordsize == 32:
            word_format = "08x"
        elif wordsize == 64:
            word_format = "016x"

        print >> f, "  Root Server:"
        print >> f, "    Entry Point: %#x" % self._entry
        print >> f, "    Stack Pointer: %#x" % self._stack

        print >> f, "    Mappings:"
        for mapping in self._mappings:
            print >> f, ("      " + word() + " -> " + word() + " (%#x)") % \
                  (mapping.get_virt(),
                   mapping.get_phys(),
                   mapping.get_size())
                                             

class KernelConfigurationSection(object):
    """The kernel configuration section."""

    # Version of the structure format.
    STRUCTURE_VERSION = 1

    # Number of fields that are not part of a subfield.
    MAX_FIELDS        = 4

    def __init__(self, section):
        """
        Create a new root server section. This is not normally called
        directly, as rootserver instances usually are created first as
        ELfSection objects and then transformed.
        """
        self._section    = section
        self._rootserver = RootServerDescriptor()
        self._memory_set = MemorySet()
        self._data       = None
        self._version    = None
        self._max_spaces = None
        self._max_caps   = None 
        self._max_mutexes = None

        if section.wordsize == 32:
            self._format_str = "4I"
        else:
            self._format_str = "4Q"

        self._format_str = section.endianess + self._format_str

        data = self._section._data

        _version, _max_spaces, _max_mutexes, _max_caps = \
                  struct.unpack(self._format_str,
                                data[0 : (section.wordsize / 8) *
                                     self.MAX_FIELDS])

        if _version != self.STRUCTURE_VERSION:
            raise MergeError, \
                  'Unsupported kernel configuration structure ' \
                  'version.  Expected version %d found %d.' % \
                  (self.STRUCTURE_VERSION, _version)

        self._version    = _version
        self._max_spaces = _max_spaces
        self._max_mutexes = _max_mutexes
        self._max_caps   = _max_caps
        self._rootserver = RootServerDescriptor()
        self._memory_set = MemorySet()

        start = (section.wordsize / 8) * self.MAX_FIELDS
        self._rootserver.fromdata(section.wordsize, section.endianess,
                                  data[start:start +
                                       RootServerDescriptor.get_struct_size(section.wordsize)])

        start = start + RootServerDescriptor.get_struct_size(section.wordsize)
        
        self._memory_set.fromdata(section.wordsize, section.endianess,
                                  data[start:start +
                                       MemorySet.get_struct_size(section.wordsize)])

    def set_config(self, parameter, value):
        """Set config parameters."""
        if parameter == "spaces":
            self._max_spaces = value
        elif parameter == "mutexes":
            self._max_mutexes = value
        elif parameter == "root_caps":
            self._max_caps = value
        else:
            raise MergeError, \
                  'Error: unknown config parameter "%s"' % parameter

    def set_entry(self, entry_point):
        """Set the entry point for the root server."""
        self._rootserver.set_entry(entry_point)

    def set_stack(self, stack_pointer):
        """Set the stack pointer for the root server."""
        self._rootserver.set_stack(stack_pointer)

    def add_mapping(self, mapping):
        """Add a rootserver memory mapping."""
        self._rootserver.add_mapping(mapping)

    def add_meminfo(self, memdesc_type, sub_type, virtual, low, high):
        """Add new memory details to the configuration."""
        new_desc = MemoryDescriptor(memdesc_type, sub_type, virtual, low, high)
        self._memory_set.add_mem_descriptor(new_desc)

    def add_mem_descriptor(self, desc):
        """Add a memory descriptor to the configuration."""
        self._memory_set.add_mem_descriptor(desc)

#     def get_file_data(self):
#         """
#         Return the file data. This overloads the parent class
#         implementation.
#         """
#         self.update_data()
#         return ElfSection.get_file_data(self)

    def update_data(self):
        """Update the section data based on our changes."""

        data = ByteArray(struct.pack(self._format_str,
                                     self._version, self._max_spaces,
                                     self._max_mutexes, self._max_caps))

        data += self._rootserver.todata(self._section.wordsize, self._section.endianess)
        data += self._memory_set.todata(self._section.wordsize, self._section.endianess)

        self._section._data = data

    def output(self, f=stdout):
        """
        Print an ASCII representation of the root server section to
        the file f.
        """

        print >> f, "Kernel Configuration:"
        print >> f, "  Version: %d" % self._version
        print >> f, "  Number of Spaces: %d" % self._max_spaces
        print >> f, "  Number of Mutexes: %d" % self._max_mutexes
        print >> f, "  Number of Root Caps: %d" % self._max_caps

        print >> f
        self._rootserver.output(self._section.wordsize, f)
        print >> f
        self._memory_set.output(self._section.wordsize, f)

def find_kernel_config(elf):
    """
    Find the kernel configuration section in an ELF. Return None if it
    can't be found.
    """
    roinit = elf.find_section_named(".roinit")

    if roinit is None:
        roinit = elf.find_section_named("kernel.roinit")

    if roinit:
        roinit = KernelConfigurationSection(roinit)

    return roinit
    
