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

# We ignore the design warning about having too many methods
# and instance variables. While this is a good design guideline
# in general, here we are modelling something underlying that
# has lots of different methods
#We disable the 'too many instance attributes' test. While a good
#idea in general, we are constrained here by the underlying data
#structures that we are modeling
#pylint: disable-msg=R0902
"""
This library contains the classes that represent ELF structures such
as the ELF header itself, program and section headers. These don't
provide a nice way of directly manipulating the structure, but it
is probably best to use the abstract method in core.py rather than
using these directly.
"""

from sys import stdout
from elf.constants import SHF_WRITE, SHF_ALLOC, SHF_EXECINSTR, SHF_MERGE, \
     SHF_STRINGS, SHF_LINK_ORDER, SHF_MASKPROC, PF_R, PF_W, PF_X, EV_CURRENT, \
     PT_NULL, \
     \
     EM_MIPS, EM_ARM, EM_IA_64, \
     \
     EF_MIPS_NOREORDER, EF_MIPS_PIC, EF_MIPS_CPIC, EF_MIPS_XGOT, \
     EF_MIPS_N32, EF_MIPS_ABI_O32, EF_MIPS_ABI_O64, \
     EF_MIPS_ARCH, EF_MIPS_ARCH_1, EF_MIPS_ARCH_2, EF_MIPS_ARCH_3, \
     EF_MIPS_ARCH_4, EF_MIPS_ARCH_5, EF_MIPS_ARCH_32, EF_MIPS_ARCH_64, \
     \
     EF_ARM_EABIMASK, EF_ARM_EABI_UNKNOWN, EF_ARM_EABI_VER1, EF_ARM_EABI_VER2, \
     EF_ARM_EABI_VER3, EF_ARM_EABI_VER4, EF_ARM_EABI_VER5, EF_ARM_RELEXEC, \
     EF_ARM_HASENTRY, EF_ARM_INTERWORK, EF_ARM_APCS_26, EF_ARM_APCS_FLOAT, \
     EF_ARM_PIC, EF_ARM_ALIGN8, EF_ARM_NEW_ABI, EF_ARM_OLD_ABI, \
     EF_ARM_SOFT_FLOAT, EF_ARM_VFP_FLOAT, EF_ARM_MAVERICK_FLOAT, EF_ARM_BE8, \
     EF_ARM_LE8, \
     \
     EF_IA_64_ABI64, \
     \
     ELFCLASSNONE, ELFCLASS32, ELFCLASS64, \
     ELFDATANONE, ELFDATA2LSB, ELFDATA2MSB, ELFOSABI_NONE, \
     SHT_NULL, SHN_UNDEF, ET_NONE, EM_NONE, \
     \
     ElfFormatError, \
     ElfClass, ElfData, ElfVersion, ElfOsabi, ElfType, \
     ElfMachine, ElfPhType, ElfShType
import struct
from elf.ByteArray import ByteArray
from elf.util import is_integer

class InvalidArgument(Exception):
    """This exception is raised when invalid argument are passed
    to a function. Methods that may raise this exception are documented
    appropriately."""

class ElfIdentification(object):
    """Provides a class for abstracting over the ELF e_ident array. (Ref 1-6)"""
    # Indenfification indexes. (Ref: Fig 1-4)
    EI_MAG0 = 0
    EI_MAG1 = 1
    EI_MAG2 = 2
    EI_MAG3 = 3
    EI_CLASS = 4
    EI_DATA = 5
    EI_VERSION = 6

    # Extensions: see
    # http://www.caldera.com/developers/gabi/2000-07-17/ch4.eheader.html
    EI_OSABI = 7
    EI_ABIVERSION = 8
    
    EI_PAD = 9

    # ELF magic (Ref: 1-7)
    ELFMAG0 = 0x7f
    ELFMAG1 = ord('E')
    ELFMAG2 = ord('L')
    ELFMAG3 = ord('F')

    def get_size(cls):
        """Return the size of the Elf ident structure."""
        return 16
    get_size = classmethod(get_size)

    def __init__(self):
        """ELF identification constructor. Initially all values are set
        to None or zero."""
        self._ei_class = ELFCLASSNONE
        self._ei_data = ELFDATANONE
        self._ei_version = EV_CURRENT
        self._ei_osabi = ELFOSABI_NONE
        self.ei_abiversion = 0

    def todata(self):
        """Convert the ELF identification header to an array of bytes"""
        data = ByteArray('\0' * 16)
        data[self.EI_MAG0] = self.ELFMAG0
        data[self.EI_MAG1] = self.ELFMAG1
        data[self.EI_MAG2] = self.ELFMAG2
        data[self.EI_MAG3] = self.ELFMAG3
        data[self.EI_CLASS] = self.ei_class
        data[self.EI_DATA] = self.ei_data
        data[self.EI_VERSION] = self.ei_version
        data[self.EI_OSABI] = self.ei_osabi
        data[self.EI_ABIVERSION] = self.ei_abiversion
        return data

    def fromdata(self, data):
        """ElfIdentification constructor. Data should be a byte array
        of length 16. Raises ElfFormatError if data is wrong length, or
        the magic identifier doesn't match."""
        if len(data) != 16:
            raise ElfFormatError, "ElfIdentification except 16 bytes of data"
        if not self.check_magic(data):
            raise ElfFormatError, \
                  "ElfIdentification doesn't match. [%x,%x,%x,%x]" % \
                  (data[self.EI_MAG0], data[self.EI_MAG1],
                   data[self.EI_MAG2], data[self.EI_MAG3])

        self.ei_class = data[self.EI_CLASS]
        if self.get_class() == ELFCLASSNONE:
            raise ElfFormatError, "ElfIdentification class is invalid"

        self.ei_data = data[self.EI_DATA]
        if self.get_data() == ELFDATANONE:
            raise ElfFormatError, "ElfIdentification data is invalid"

        self.ei_version = data[self.EI_VERSION]
        self.ei_osabi = data[self.EI_OSABI]
        self.ei_abiversion = data[self.EI_ABIVERSION]

    def check_magic(cls, data):
        """Check that the magic is correct."""
        return ((data[cls.EI_MAG0] == cls.ELFMAG0) and
                (data[cls.EI_MAG1] == cls.ELFMAG1) and
                (data[cls.EI_MAG2] == cls.ELFMAG2) and
                (data[cls.EI_MAG3] == cls.ELFMAG3))
    check_magic = classmethod(check_magic)

    def get_class(self):
        """Return the ELF class data"""
        return self._ei_class
    def set_class(self, elf_class):
        """Set the ELF class field"""
        self._ei_class = ElfClass(elf_class)
    ei_class = property(get_class, set_class)

    def get_data(self):
        """Return elf data field"""
        return self._ei_data
    def set_data(self, data):
        """Set the elf data field"""
        self._ei_data = ElfData(data)
    ei_data = property(get_data, set_data)

    def _get_wordsize(self):
        """Return the word size based on the class information"""
        word_sizes = { ELFCLASS32 : 32, ELFCLASS64 : 64 }
        try:
            return word_sizes[self.ei_class]
        except KeyError:
            raise ElfFormatError, "Unknown Elf Class unknown: %x" % \
                self.ei_class

    def _set_wordsize(self, wordsize):
        """Set the class information based on the given wordsize"""
        word_sizes = { 32: ELFCLASS32, 64 : ELFCLASS64 }
        try:
            self.ei_class =  word_sizes[wordsize]
        except KeyError:
            raise ElfFormatError, "Invalid wordsize %d" % wordsize

    wordsize = property(_get_wordsize, _set_wordsize)

    def _get_endianess(self):
        """Return the endianess. '>' For big endian, '<' for little
        endian. (Following struct module.)"""
        endianesses = { ELFDATA2LSB : "<", ELFDATA2MSB : ">" }
        try:
            return endianesses[self.ei_data]
        except KeyError:
            raise ElfFormatError, "Unknown data encoding format: %x" % \
                  self.ei_data

    def _set_endianess(self, endian):
        """Set the endianess in the ELF header. '>' for big endian,
        '<' for little."""
        endianesses = { "<" : ELFDATA2LSB, ">" : ELFDATA2MSB }
        try:
            self.ei_data = endianesses[endian]
        except KeyError:
            raise ElfFormatError, "Unknown endianess %s" % endian

    endianess = property(_get_endianess, _set_endianess)

    def _get_version(self):
        """Return the ELF version"""
        return self._ei_version
    def _set_version(self, version):
        """Set the ELF version"""
        self._ei_version = ElfVersion(version)
    ei_version = property(_get_version, _set_version)

    def _get_osabi(self):
        """Return the OS/ABI"""
        return self._ei_osabi
    def _set_osabi(self, osabi):
        """Set the OS/ABI"""
        self._ei_osabi = ElfOsabi(osabi)
    ei_osabi = property(_get_osabi, _set_osabi)

class ElfHeader(object):
    """This class abstracts the ELF header. (Ref 1-4).
    This is an abstract class and should not be instantiated,
    it is designed to be overloaded by Elf32Header and Elf64Header.
    """
    layout = None # Subclasses should redefine this
    wordsize = None # Subclasses should redefine this

    def size(cls):
        """Return the structure size of the ElfHeader"""
        return struct.calcsize(cls.layout) + 16
    size = classmethod(size)

    def __init__(self, endianess = '<'):
        self.ident = ElfIdentification()
        self.ident.endianess = endianess
        self.ident.wordsize = self.wordsize

        self._e_type = ET_NONE
        self._e_machine = EM_NONE
        self._e_version = EV_CURRENT
        self.e_entry = 0
        self.e_phoff = 0
        self.e_shoff = 0
        self.e_flags = 0
        self.e_ehsize = self.size()
        self.e_phentsize = 0
        self.e_phnum = 0
        self.e_shentsize = 0
        self.e_shnum = 0
        self.e_shstrndx = 0

    def fromdata(self, data):
        """Initialise an ElfHeader object from provided data"""
        if len(data) != self.size():
            raise ElfFormatError, "Data size must be %s. %s provided." % \
                  (self.size(), len(data))

        self.ident = ElfIdentification()
        self.ident.fromdata(data[:16])

        fields = struct.unpack(self.ident.endianess + self.layout, data[16:])
        self.e_type = fields[0]
        self.e_machine = fields[1]
        self.e_version = fields[2]
        self.e_entry = fields[3]
        self.e_phoff = fields[4]
        self.e_shoff = fields[5]
        self.e_flags = fields[6]
        self.e_ehsize = fields[7]
        self.e_phentsize = fields[8]
        self.e_phnum = fields[9]
        self.e_shentsize = fields[10]
        self.e_shnum = fields[11]
        self.e_shstrndx = fields[12]

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        data = self.ident.todata()
        packed = struct.pack(self.ident.endianess + self.layout,
                             self.e_type, self.e_machine, self.e_version,
                             self.e_entry, self.e_phoff, self.e_shoff,
                             self.e_flags, self.e_ehsize, self.e_phentsize,
                             self.e_phnum, self.e_shentsize, self.e_shnum,
                             self.e_shstrndx)
        data.extend(ByteArray(packed))
        return data

    def _get_type(self):
        """Return the machine architecture."""
        return self._e_type
    def _set_type(self, elf_type):
        """Set the machine architecture."""
        assert is_integer(elf_type)
        self._e_type = ElfType(elf_type)
    e_type = property(_get_type, _set_type)
        
    def _get_machine(self):
        """Return the machine architecture."""
        return self._e_machine
    def _set_machine(self, machine):
        """Set the machine architecture."""
        assert is_integer(machine)
        self._e_machine = ElfMachine(machine)
    e_machine = property(_get_machine, _set_machine)

    def _get_version(self):
        """Return the object file version."""
        return self._e_version
    def _set_version(self, version):
        """Set the version field."""
        assert is_integer(version)
        self._e_version = ElfVersion(version)
    e_version = property(_get_version, _set_version)

    def _get_markedup_flags_mips(self):
        """Return the marked up flags for MIPS"""
        f = ""
        for mask in [EF_MIPS_NOREORDER, EF_MIPS_PIC, EF_MIPS_CPIC, \
                         EF_MIPS_XGOT]:
            if self.e_flags & mask:
                f += ", " + mask.__str__()

        if self.e_flags & EF_MIPS_N32:
            if self.e_flags & EF_MIPS_ABI_O32:
                f += ", n32"
            if self.e_flags & EF_MIPS_ABI_O64:
                f += ", n64"
        else:
            if self.e_flags & EF_MIPS_ABI_O32:
                f += ", " + EF_MIPS_ABI_O32.__str__()
            if self.e_flags & EF_MIPS_ABI_O64:
                f += ", " + EF_MIPS_ABI_O64.__str__()

        for arch in [EF_MIPS_ARCH_1, EF_MIPS_ARCH_2, EF_MIPS_ARCH_3, \
                         EF_MIPS_ARCH_4, EF_MIPS_ARCH_5, EF_MIPS_ARCH_32, \
                         EF_MIPS_ARCH_64]:
            if (self.e_flags & EF_MIPS_ARCH) == arch:
                f += ", " + arch.__str__()
        return f

    def _get_markedup_flags_arm(self):
        """Return the marked up flags for ARM"""
        f = ""
        unknown = True
        # Print out has entry point
        if self.e_flags & EF_ARM_HASENTRY:
            f += ", " + EF_ARM_HASENTRY.__str__()
        elif self.e_flags & EF_ARM_RELEXEC:
            f += ", " + EF_ARM_RELEXEC.__str__()
        # Print out the ABI string
        file_abi = (self.e_flags & EF_ARM_EABIMASK)
        for abi in [EF_ARM_EABI_UNKNOWN, EF_ARM_EABI_VER1, EF_ARM_EABI_VER2, \
                        EF_ARM_EABI_VER3, EF_ARM_EABI_VER4, EF_ARM_EABI_VER5]:
            if file_abi == abi:
                f += ", " + abi.__str__()
        if file_abi == EF_ARM_EABI_UNKNOWN:
            for feat in [EF_ARM_INTERWORK, EF_ARM_APCS_26, EF_ARM_APCS_FLOAT, 
                         EF_ARM_PIC, EF_ARM_ALIGN8, EF_ARM_NEW_ABI, 
                         EF_ARM_OLD_ABI, EF_ARM_SOFT_FLOAT, EF_ARM_VFP_FLOAT,
                         EF_ARM_MAVERICK_FLOAT]:
                if (self.e_flags & feat):
                    f += ", " + feat.__str__()
                    unknown = False
        if file_abi in [EF_ARM_EABI_VER1, EF_ARM_EABI_VER2, EF_ARM_EABI_VER3,
                        EF_ARM_EABI_VER4, EF_ARM_EABI_VER5]:
            for feat in [EF_ARM_BE8, EF_ARM_LE8]:
                f += ", " + feat.__str__()
                unknown = False

        if unknown:
            f += ", <unknown>"
        return f

    def _get_markedup_flags_ia64(self):
        """Return the marked up flags for IA-64"""
        if self.e_flags == EF_IA_64_ABI64:
            return ", 64-bit"
        else:
            return ", 32-bit"

    def get_markedup_flags(self):
        """Decode the flags and return as a string."""

        if self.e_machine == EM_MIPS:
            return self._get_markedup_flags_mips()
        if self.e_machine == EM_ARM:
            return self._get_markedup_flags_arm()
        if self.e_machine == EM_IA_64:
            return self._get_markedup_flags_ia64()
        return ""

    def output(self, f=stdout):
        """Print the ELF header in the format seen in the readelf utility."""
        print >> f, "ELF Header:"
        print >> f, "  Magic:  ",
        for i_d in self.ident.todata():
            print >> f, "%02x" % i_d,
        print >> f, ""
        print >> f, "  Class:                             %s" % \
              self.ident.ei_class
        print >> f, "  Data:                              %s" % \
              self.ident.ei_data
        print >> f, "  Version:                           %s" % \
              self.ident.ei_version
        print >> f, "  OS/ABI:                            %s" % \
              self.ident.ei_osabi
        print >> f, "  ABI Version:                       %s" % \
              self.ident.ei_abiversion
        print >> f, "  Type:                              %s" % \
              self.e_type
        print >> f, "  Machine:                           %s" % \
              self.e_machine
        print >> f, "  Version:                           0x%x" % \
              self.e_version
        print >> f, "  Entry point address:               0x%x" % \
              self.e_entry
        print >> f, "  Start of program headers:          %d " \
              "(bytes into file)" % self.e_phoff
        print >> f, "  Start of section headers:          %d " \
              "(bytes into file)" % self.e_shoff
        print >> f, "  Flags:                             0x%x%s" % \
              (self.e_flags, self.get_markedup_flags())
        print >> f, "  Size of this header:               %d (bytes)" % \
              self.e_ehsize
        print >> f, "  Size of program headers:           %d (bytes)" % \
              self.e_phentsize
        print >> f, "  Number of program headers:         %d" % \
              self.e_phnum      
        print >> f, "  Size of section headers:           %d (bytes)" % \
              self.e_shentsize
        print >> f, "  Number of section headers:         %d" % \
              self.e_shnum       
        print >> f, "  Section header string table index: %d" % \
              self.e_shstrndx

class Elf32Header(ElfHeader):
    """Class abstracting a 32-bit ELF header."""
    layout = "HHIIIIIHHHHHH"
    wordsize = 32

class Elf64Header(ElfHeader):
    """Class abstracting a 64-bit ELF header."""
    layout = "HHIQQQIHHHHHH"
    wordsize = 64

ELF_HEADER_CLASSES = { 32: Elf32Header,
                       64: Elf64Header }

class ElfProgramHeader(object):
    """Abstract the ELF Program header structure. These structures
    describe segments in the ELF file."""
    layout = None # Subclasses should redefine this

    def size(cls):
        """Return the structure size of the ElfHeader"""
        return struct.calcsize(cls.layout)
    size = classmethod(size)

    def __init__(self, endianess):
        if endianess not in ["<", ">"]:
            raise InvalidArgument, "Endianess must be either < or >"
        self.endianess = endianess
        self._p_type = PT_NULL
        self.p_offset = 0
        self.p_vaddr = 0
        self.p_paddr = 0
        self.p_filesz = 0
        self.p_memsz = 0
        self.p_flags = 0
        self.p_align = 0

    def _get_type(self):
        """Return the kind of segment this header describes."""
        return self._p_type
    def _set_type(self, ph_type):
        """Set the kind of segment this header describes."""
        assert is_integer(ph_type)
        self._p_type = ElfPhType(ph_type)
    p_type = property(_get_type, _set_type)

    def get_flags_str(self):
        """Return a string describing the flags"""
        out = [' ', ' ', ' ']
        if self.p_flags & PF_R:
            out[0] =  "R"
        if self.p_flags & PF_W:
            out[1] =  "W"
        if self.p_flags & PF_X:
            out[2] =  "E"
        return "".join(out)

class Elf32ProgramHeader(ElfProgramHeader):
    """32-bit ELf Program header"""
    layout = "IIIIIIII"

    def fromdata(self, data, ehdr):
        """Initialise an ElfHeader object from provided data"""
        if len(data) != self.size():
            raise ElfFormatError, "Data size must be %s. %s provided." % \
                  (self.size(), len(data))

        fields = struct.unpack(self.endianess + self.layout, data)
        self.p_type = fields[0]
        self.p_offset = fields[1]
        vaddr = fields[2]
        if ehdr.e_machine == ElfMachine(8):
            if ehdr.e_flags & EF_MIPS_ABI_O64:
                if vaddr & 0x80000000:
                    vaddr |= 0xffffffff00000000L
        self.p_vaddr = vaddr
        self.p_paddr = fields[3]
        self.p_filesz = fields[4]
        self.p_memsz = fields[5]
        self.p_flags = fields[6]
        self.p_align = fields[7]

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.p_type, self.p_offset,
                             (self.p_vaddr & 0xffffffffL),
                             self.p_paddr, self.p_filesz,
                             self.p_memsz, self.p_flags, self.p_align)
        data = ByteArray(packed)
        return data

    def output(self, f=stdout):
        """Pretty print the program header to a given file, which
        defaults to stdout."""
        print >> f, "  %-14.14s" % self.p_type,
        print >> f, "0x%6.6x" % self.p_offset,
        print >> f, "0x%8.8x" % (self.p_vaddr & 0xffffffffL),
        print >> f, "0x%8.8x" % self.p_paddr,
        print >> f, "0x%5.5x" % self.p_filesz,
        print >> f, "0x%5.5x" % self.p_memsz,
        print >> f, "%s" % self.get_flags_str(),
        if self.p_align != 0:
            print >> f, "0x%x" % self.p_align
        else:
            print >> f, "%x" % self.p_align

class Elf64ProgramHeader(ElfProgramHeader):
    """64-bit ELf Program header"""
    layout = "IIQQQQQQ"

    #ehdr is unused, but that is fine. It is used by other subclasses
    #pylint: disable-msg=W0613
    def fromdata(self, data, ehdr):
        """Initialise an ElfHeader object from provided data"""
    
        if len(data) != self.size():
            raise ElfFormatError, "Data size must be %s. %s provided." % \
                  (self.size(), len(data))

        fields = struct.unpack(self.endianess + self.layout, data)
        self.p_type = fields[0]
        self.p_flags = fields[1]
        self.p_offset = fields[2]
        self.p_vaddr = fields[3]
        self.p_paddr = fields[4]
        self.p_filesz = fields[5]
        self.p_memsz = fields[6]
        self.p_align = fields[7]

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.p_type, self.p_flags, self.p_offset,
                             self.p_vaddr, self.p_paddr, self.p_filesz,
                             self.p_memsz, self.p_align)
        data = ByteArray(packed)
        return data

    def output(self, f=stdout):
        """Pretty print the program header to a given file, which
        defaults to stdout."""
        print >> f, "  %-14.14s" % self.p_type,
        print >> f, "0x%16.16x" % self.p_offset,
        print >> f, "0x%16.16x" % self.p_vaddr,
        print >> f, "0x%16.16x" % self.p_paddr
        print >> f, "                ",
        print >> f, "0x%16.16x" % self.p_filesz,
        print >> f, "0x%16.16x" % self.p_memsz,
        print >> f, " %s   " % self.get_flags_str(),
        print >> f, "%x" % self.p_align

ELF_PH_CLASSES = { 32: Elf32ProgramHeader,
                   64: Elf64ProgramHeader }

class ElfSectionHeader(object):
    """Class representing an ELF section header. (Ref 1-9). This class should 
    not be directly instantatied and should be subclassed, by Elf32Section and
    Elf64Section."""

    def __init__(self, endianess):
        if endianess not in ["<", ">"]:
            raise InvalidArgument, "Endianess must be either < or >"
        self.endianess = endianess
        self.sh_name = 0
        self._sh_type = SHT_NULL
        self.sh_flags = 0
        self.sh_addr = 0
        self.sh_offset = 0
        self.sh_size = 0
        self.sh_link = SHN_UNDEF
        self.sh_info = 0
        self.sh_addralign = 0
        self.sh_entsize = 0

        self._name = None
        self._index = None

    layout = None # Subclasses should redefine this

    def size(cls):
        """Return the structure size of the ElfHeader"""
        return struct.calcsize(cls.layout)
    size = classmethod(size)

    def get_type(self):
        """Property getter for sh_type"""
        return self._sh_type
    def set_type(self, sh_type):
        """Property setter for sh_type. Ensures is a ElfShType object."""
        self._sh_type = ElfShType(sh_type)
    sh_type = property(get_type, set_type)

    def fromdata(self, data, ehdr):
        """Initialise an ElfHeader object from provided data"""
        if len(data) != self.size():
            raise ElfFormatError, "Data size must be %s. %s provided." % \
                  (self.size(), len(data))

        fields = struct.unpack(self.endianess + self.layout, data)
        self.sh_name = fields[0]
        self.sh_type = fields[1]
        self.sh_flags = fields[2]
        vaddr = fields[3]
        if ehdr.e_machine == ElfMachine(8):
            if ehdr.e_flags & EF_MIPS_ABI_O64:
                if vaddr & 0x80000000:
                    vaddr |= 0xffffffff00000000L
        self.sh_addr = vaddr
        self.sh_offset = fields[4]
        self.sh_size = fields[5]
        self.sh_link = fields[6]
        self.sh_info = fields[7]
        self.sh_addralign = fields[8]
        self.sh_entsize = fields[9]

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.sh_name, self.sh_type, self.sh_flags,
                             (self.sh_addr & 0xffffffffL),
                             self.sh_offset, self.sh_size, self.sh_link,
                             self.sh_info, self.sh_addralign, self.sh_entsize)
        data = ByteArray(packed)
        return data

    def get_markedup_flags(self):
        """Return the flags as a string showing which bits are set."""
        ret = ""
        if self.sh_flags & SHF_WRITE:
            ret = ret + 'W'
        if self.sh_flags & SHF_ALLOC:
            ret = ret + 'A'
        if self.sh_flags & SHF_EXECINSTR:
            ret = ret + 'X'
        if self.sh_flags & SHF_MERGE:
            ret = ret + 'M'
        if self.sh_flags & SHF_STRINGS:
            ret = ret + 'S'
        if self.sh_flags & SHF_LINK_ORDER:
            ret = ret + 'L'
        if self.sh_flags & SHF_MASKPROC:
            ret = ret + 'p'

        return ret

class Elf32SectionHeader(ElfSectionHeader):
    """This class is used to represent 32-bit ELF section
    headers."""
    layout = "IIIIIIIIII"

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.sh_name, self.sh_type, self.sh_flags,
                             (self.sh_addr & 0xffffffffL),
                             self.sh_offset, self.sh_size, self.sh_link,
                             self.sh_info, self.sh_addralign, self.sh_entsize)
        data = ByteArray(packed)
        return data

    def output(self, f=stdout):
        """Pretty print the section header to a given file, which
        defaults to stdout."""
        if self._index is not None:
            print >> f, "  [%2d]" % self._index,
            print >> f, "%-17.17s" % self._name,
        print >> f, "%-15.15s" % self.sh_type,
        print >> f, "%8.8x" % (self.sh_addr & 0xffffffffL),
        print >> f, "%6.6x" % self.sh_offset,
        print >> f, "%6.6x" % self.sh_size,
        print >> f, "%2.2x" % self.sh_entsize,
        print >> f, "%3.3s" % self.get_markedup_flags(),
        print >> f, "%2d" % self.sh_link,
        print >> f, "%3d" % self.sh_info,
        print >> f, "%2d" % self.sh_addralign

class Elf64SectionHeader(ElfSectionHeader):
    """This class is used to represent 64-bit ELF section
    headers."""
    layout = "IIQQQQIIQQ"

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.sh_name, self.sh_type, self.sh_flags,
                             self.sh_addr, self.sh_offset, self.sh_size,
                             self.sh_link, self.sh_info, self.sh_addralign,
                             self.sh_entsize)
        data = ByteArray(packed)
        return data

    def output(self, f=stdout):
        """Pretty print the section header to a given file, which
        defaults to stdout."""
        if self._index is not None:
            print >> f, "  [%2d]" % self._index,
            print >> f, "%-17.17s" % self._name,
        print >> f, "%-16.16s" % self.sh_type,
        print >> f, "%16.16x " % self.sh_addr,
        print >> f, "%8.8x" % self.sh_offset
         
        print >> f, "       %16.16x " % self.sh_size,
        print >> f, "%16.16x" % self.sh_entsize,
        print >> f, "%3.3s" % self.get_markedup_flags(),
        print >> f, "     %2d" % self.sh_link,
        print >> f, "  %3d" % self.sh_info,
        print >> f, "    %d" % self.sh_addralign

ELF_SH_CLASSES = { 32: Elf32SectionHeader,
                   64: Elf64SectionHeader }

class ElfSymbolStruct(object):
    """Abstract the ELF symbol header structure. These structures
    describe segments in the ELF file."""
    layout = None # Subclasses should redefine this

    def size(cls):
        """Return the structure size of the ElfHeader"""
        return struct.calcsize(cls.layout)
    size = classmethod(size)

    def __init__(self, endianess):
        if endianess not in ["<", ">"]:
            raise InvalidArgument, "Endianess must be either < or >"
        self.endianess = endianess
        self.st_name = 0
        self.st_value = 0
        self.st_size = 0
        self.st_info = 0
        self.st_other = 0
        self.st_shndx = 0

    def get_bind(self):
        """Return bind"""
        return self.st_info >> 4

    def set_bind(self, bind):
        """Set the binding"""
        self.st_info = (bind << 4) | (self.st_info & 0xf)

    st_bind = property(get_bind, set_bind)

    def get_type(self):
        """Return the type of the symbol"""
        return self.st_info & 0xf

    def set_type(self, sym_type):
        """Set the type"""
        self.st_info = (self.st_info & 0xf0) | sym_type

    st_type = property(get_type, set_type)

class Elf32SymbolStruct(ElfSymbolStruct):
    """This class is used to represent 32-bit ELF Symbols."""
    layout = "IIIBBH"

    def output(self, f=stdout):
        """Pretty print the symbol to a given file, which
        defaults to stdout."""
        print >> f, "%8.8x" % self.st_name,
        print >> f, "%8.8x" % self.st_value,
        print >> f, "%8.8x" % self.st_size,
        print >> f, "%2.2x" % self.st_info,
        print >> f, "%2.2x" % self.st_other,
        print >> f, "%4.4x" % self.st_shndx

    def fromdata(self, data):
        """Initialise a Symbol object from provided data"""
        if len(data) != self.size():
            raise ElfFormatError, "Data size must be %s. %s provided." % \
                  (self.size(), len(data))

        fields = struct.unpack(self.endianess + self.layout, data)
        self.st_name = fields[0]
        self.st_value = fields[1]
        self.st_size = fields[2]
        self.st_info = fields[3]
        self.st_other = fields[4]
        self.st_shndx = fields[5]

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.st_name, self.st_value, self.st_size,
                             self.st_info, self.st_other, self.st_shndx)
        data = ByteArray(packed)
        return data

class Elf64SymbolStruct(ElfSymbolStruct):
    """This class is used to represent 64-bit ELF Symbols."""
    layout = "IBBHQQ"

    def output(self, f=stdout):
        """Pretty print the symbol to a given file, which
        defaults to stdout."""
        print >> f, "%8.8x" % self.st_name,
        print >> f, "%2.2x" % self.st_info,
        print >> f, "%2.2x" % self.st_other,
        print >> f, "%4.4x" % self.st_shndx,
        print >> f, "%16.16x" % self.st_value,
        print >> f, "%16.16x" % self.st_size

    def fromdata(self, data):
        """Initialise a Symbol object from provided data"""
        if len(data) != self.size():
            raise ElfFormatError, "Data size must be %s. %s provided." % \
                  (self.size(), len(data))

        fields = struct.unpack(self.endianess + self.layout, data)
        self.st_name = fields[0]
        self.st_info = fields[1]
        self.st_other = fields[2]
        self.st_shndx = fields[3]
        self.st_value = fields[4]
        self.st_size = fields[5]

    def todata(self):
        """Convert the ELF header to an array of bytes"""
        packed = struct.pack(self.endianess + self.layout,
                             self.st_name, self.st_info, self.st_other,
                             self.st_shndx, self.st_value, self.st_size)
        data = ByteArray(packed)
        return data

ELF_SYMBOL_STRUCT_CLASSES = { 32: Elf32SymbolStruct,
                              64: Elf64SymbolStruct }
