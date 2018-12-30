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
Test file for elf.py
"""

import unittest
import coverage
import array
import copy
import os
import StringIO
import commands
import elf
import sys
import commands
from elf.File import File
import elf.structures
from elf.structures import *
from elf.constants import ElfFormatError, ET_EXEC, \
     EM_ARM, EV_CURRENT, EM_386, EM_MIPS, EM_IA_64, EM_X86_64, \
     PT_PHDR, PT_INTERP, PT_LOAD, PT_DYNAMIC, PT_NOTE, PT_GNU_EH_FRAME, PT_GNU_STACK, \
     SHT_PROGBITS, SHT_STRTAB, SHT_SYMTAB, SHT_NOTE, SHT_HASH, SHT_DYNSYM, SHT_VERSYM, \
     SHT_VERNEED, SHT_RELA, SHT_DYNAMIC, SHT_NOBITS, ELFOSABI_NONE, ELFOSABI_ARM
modules_under_test = [elf.structures]

# Test to see if we have readelf installed

status, output = commands.getstatusoutput("readelf -H")
# status is 0 when readelf gave us its help some large nonzero otherwise
have_readelf = not status
if not have_readelf:
    print >>sys.stderr, "test_structures.py: You don't have readelf " \
          "installed. Not all tests will run."

def copy_array(_ary):
    return array.array(_ary.typecode, _ary.tostring())

elf_files = {
    "data/arm_exec": (ET_EXEC, EM_ARM, EV_CURRENT,
                           0xf0021000L, 52, 1262068, 0x202, 32, 1, 40, 20, 17),
    "data/arm_scatter_load": (ET_EXEC, EM_ARM, EV_CURRENT,
                              0x100000L, 42228, 42260, 0x4000016, 32, 1, 40, 17, 16),
    "data/ia32_exec": (ET_EXEC, EM_386, EV_CURRENT,
                            0x146b60, 52, 1081392, 0x0, 32, 4, 40, 22, 19),
    "data/mips64_exec": (ET_EXEC, EM_MIPS, EV_CURRENT,
                              0x80050030L, 52, 158304, 0x20002001L, 32, 2, 40, 11, 8),
    "data/mips32_exec": (ET_EXEC, EM_MIPS, EV_CURRENT,
                              0x80050020L, 52, 623420, 0x50001001L, 32, 7, 40, 36, 35),
    "data/ia64_exec": (ET_EXEC, EM_IA_64, EV_CURRENT,
                            0x4000000000002f00L, 64, 159936, 0x10, 56, 7, 64, 31, 30),
    "data/amd64_exec": (ET_EXEC, EM_X86_64, EV_CURRENT,
                            0x4022d0, 64, 83776, 0x0, 56, 8, 64, 26, 25),
    }

class TestElfHeader(unittest.TestCase):
    def setUp(self):
        self.elf_headers = {}
        for file_name in elf_files:
            f = File(file_name, "rb")
            ident = ElfIdentification()
            ident.fromdata(f.get_data(0, 16))
            if ident.wordsize == 32:
                data = f.get_data(0, Elf32Header.size())
                header = Elf32Header()
                header.fromdata(data)
            elif ident.wordsize == 64:
                data = f.get_data(0, Elf64Header.size())
                header = Elf64Header()
                header.fromdata(data)
            else:
                raise "Problem"
            self.elf_headers[file_name] = (header, data)
        self.bogus_data_size = f.get_data(0, Elf32Header.size() - 1)

    def test_bogus_data_size(self):
        print "STRUCTURE", elf.structures

        header = Elf32Header()
        self.assertRaises(elf.structures.ElfFormatError, header.fromdata,
                          self.bogus_data_size)

    def test_gettype(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_type,
                             elf_files[file_name][0])
            
    def test_getmachine(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_machine,
                             elf_files[file_name][1])

    def test_getversion(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_version,
                             elf_files[file_name][2])

    def test_getentry(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_entry,
                             elf_files[file_name][3])

    def test_getphoff(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_phoff,
                             elf_files[file_name][4])

    def test_getshoff(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_shoff,
                             elf_files[file_name][5])

    def test_getflags(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_flags,
                             elf_files[file_name][6])

    def test_getphentsize(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_phentsize,
                             elf_files[file_name][7])

    def test_getphnum(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_phnum,
                             elf_files[file_name][8])

    def test_getshentsize(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_shentsize,
                             elf_files[file_name][9])

    def test_getshnum(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_shnum,
                             elf_files[file_name][10])

    def test_getshstrndx(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].e_shstrndx,
                             elf_files[file_name][11])

    def test_todata(self):
        for file_name in elf_files:
            self.assertEqual(self.elf_headers[file_name][0].todata(),
                             self.elf_headers[file_name][1])

    def test_print(self):
        if not have_readelf:
            return
        for file_name in elf_files:
            readelf_data_lines = os.popen("readelf -h %s" % file_name).readlines()
            out = StringIO.StringIO()
            self.elf_headers[file_name][0].output(out)
            out.seek(0)
            program_data_lines = out.readlines()

            for i in range(len(readelf_data_lines)):
                readelf = readelf_data_lines[i]
                program = program_data_lines[i]
                if readelf != program:
                    # If they're not equal, it's because we implement something which is <unkown>
                    # to readelf. They should be equal at least up to this difference.
                    readelf = readelf.split("<unknown>")[0]
                    self.assertEqual(program.startswith(readelf), True)
                else:
                    self.assertEqual(readelf_data_lines[i], program_data_lines[i])

    def foo_test_all_machine(self):
        if not have_readelf:
            return 
        for i in range(1, 115):
            header = elf.Elf32Header()
            header.set_machine(i)
            header.ident.endianess = ">"
            header.ident.wordsize = 32
            #header.output()
            data = header.todata()
            file_name = "/tmp/foo"
            data.tofile(file(file_name, "wb"))
            readelf_data_lines = os.popen("readelf -h %s" % file_name).readlines()
            out = StringIO.StringIO()
            header.output(out)
            out.seek(0)
            program_data_lines = out.readlines()

            for i in range(len(readelf_data_lines)):
                self.assertEqual(readelf_data_lines[i], program_data_lines[i])

    
class TestElfIdentification(unittest.TestCase):
    def setUp(self):
        f = File("data/arm_exec", "rb")
        self.valid_ident_data = f.get_data(0, 16)

        self.valid_32_little = copy_array(self.valid_ident_data)
        self.valid_32_little_ident = ElfIdentification()
        self.valid_32_little_ident.fromdata(self.valid_32_little)

        self.valid_64_little = copy_array(self.valid_ident_data)
        self.valid_64_little[ElfIdentification.EI_CLASS] = ELFCLASS64;
        self.valid_64_little_ident = ElfIdentification()
        self.valid_64_little_ident.fromdata(self.valid_64_little)

        self.valid_32_big = copy_array(self.valid_ident_data)
        self.valid_32_big[ElfIdentification.EI_DATA] = ELFDATA2MSB;        
        self.valid_32_big_ident = ElfIdentification()
        self.valid_32_big_ident.fromdata(self.valid_32_big)

        self.valid_64_big = copy_array(self.valid_ident_data)
        self.valid_64_big[ElfIdentification.EI_CLASS] = ELFCLASS64;
        self.valid_64_big[ElfIdentification.EI_DATA] = ELFDATA2MSB;        
        self.valid_64_big_ident = ElfIdentification()
        self.valid_64_big_ident.fromdata(self.valid_64_big)

        self.bogus_size = f.get_data(0, 10)

        self.bogus_magic = copy_array(self.valid_ident_data)
        self.bogus_magic[0] = 0

        self.bogus_data = copy_array(self.valid_ident_data)
        self.bogus_data[ElfIdentification.EI_DATA] = 0

        self.bogus_class = copy_array(self.valid_ident_data)
        self.bogus_class[ElfIdentification.EI_CLASS] = 0

        self.unknown_class = copy_array(self.valid_ident_data)
        self.unknown_class[ElfIdentification.EI_CLASS] = 200
        self.unknown_class_ident = ElfIdentification()
        self.unknown_class_ident.fromdata(self.unknown_class)

        self.unknown_data = copy_array(self.valid_ident_data)
        self.unknown_data[ElfIdentification.EI_DATA] = 200
        self.unknown_data_ident = ElfIdentification()
        self.unknown_data_ident.fromdata(self.unknown_data)

    def test_get_size(self):
        self.assertEqual(ElfIdentification.get_size(), 16)

    def test_bogus_size(self):
        ident = ElfIdentification()
        self.assertRaises(elf.structures.ElfFormatError, ident.fromdata, self.bogus_size)

    def test_bogus_magic(self):
        ident = ElfIdentification()
        self.assertRaises(elf.structures.ElfFormatError, ident.fromdata, self.bogus_magic)

    def test_bogus_data(self):
        ident = ElfIdentification()
        self.assertRaises(elf.structures.ElfFormatError, ident.fromdata, self.bogus_data)

    def test_bogus_class(self):
        ident = ElfIdentification()
        self.assertRaises(elf.structures.ElfFormatError, ident.fromdata, self.bogus_class)

    def test_getwordsize(self):
        self.assertEqual(self.valid_32_little_ident.wordsize, 32)
        self.assertEqual(self.valid_64_little_ident.wordsize, 64)
        self.assertEqual(self.valid_32_big_ident.wordsize, 32)
        self.assertEqual(self.valid_64_big_ident.wordsize, 64)
        def get_invalid_wordsize(ident):
            x = ident.wordsize
        self.assertRaises(elf.structures.ElfFormatError, get_invalid_wordsize, self.unknown_class_ident)

    def test_setwordsize(self):
        ident = ElfIdentification()
        ident.wordsize = 32
        self.assertEqual(ident.wordsize, 32)
        ident.wordsize = 64
        self.assertEqual(ident.wordsize, 64)
        ident.wordsize = 32
        self.assertEqual(ident.wordsize, 32)
        def set_invalid_wordsize(ident):
            ident.wordsize = 10
        self.assertRaises(elf.structures.ElfFormatError, set_invalid_wordsize, ident)

    def test_getendianess(self):
        self.assertEqual(self.valid_32_little_ident.endianess, '<')
        self.assertEqual(self.valid_64_little_ident.endianess, '<')
        self.assertEqual(self.valid_32_big_ident.endianess, '>')
        self.assertEqual(self.valid_64_big_ident.endianess, '>')
        def get_invalid_endianess(ident):
            x = ident.endianess
        self.assertRaises(elf.structures.ElfFormatError, get_invalid_endianess, self.unknown_data_ident)

    def test_setendianess(self):
        ident = ElfIdentification()
        ident.endianess = '>'
        self.assertEqual(ident.endianess, '>')
        ident.endianess = '<'
        self.assertEqual(ident.endianess, '<')
        ident.endianess = '>'
        self.assertEqual(ident.endianess, '>')
        def set_invalid_endianess(ident):
            ident.endianess = 'q'
        self.assertRaises(elf.structures.ElfFormatError, set_invalid_endianess, ident)

    def test_get_set_osabi(self):
        ident = ElfIdentification()
        self.assertEquals(ident.ei_osabi, ELFOSABI_NONE)
        ident.ei_osabi = ELFOSABI_ARM
        self.assertEquals(ident.ei_osabi, ELFOSABI_ARM)

    def test_get_set_abiversion(self):
        ident = ElfIdentification()
        self.assertEquals(ident.ei_abiversion, 0)
        ident.ei_abiversion = 12
        self.assertEquals(ident.ei_abiversion, 12)


    def test_version(self):
        self.assertEqual(self.valid_32_little_ident.ei_version, 1)
        self.assertEqual(self.valid_64_little_ident.ei_version, 1)
        self.assertEqual(self.valid_32_big_ident.ei_version, 1)
        self.assertEqual(self.valid_64_big_ident.ei_version, 1)

elf_ph_expected = {
    "data/amd64_exec": [ (PT_PHDR, 0x40, 0x400040L, 0x400040L, 0x1c0, 0x1c0,
                          PF_R | PF_X, 8),
                         (PT_INTERP, 0x200, 0x400200L, 0x400200L, 0x1c, 0x1c,
                          PF_R, 1),
                         (PT_LOAD, 0x0, 0x400000L, 0x400000L, 0x1355c, 0x1355c,
                          PF_R | PF_X, 0x100000),
                         (PT_LOAD, 0x14000, 0x514000L, 0x514000L, 0x670, 0xb58,
                          PF_R | PF_W, 0x100000),
                         (PT_DYNAMIC, 0x14028, 0x514028L, 0x514028L, 0x1b0, 0x1b0,
                          PF_R | PF_W, 8),
                         (PT_NOTE, 0x21c, 0x40021cL, 0x40021cL, 0x20, 0x20,
                          PF_R, 4),
                         (PT_GNU_EH_FRAME, 0x11a60, 0x411a60L, 0x411a60L, 0x504, 0x504,
                          PF_R, 4),
                         (PT_GNU_STACK, 0x0, 0x0L, 0x0L, 0x0, 0x0,
                          PF_R | PF_W, 8),
                         ],
    "data/arm_exec": [ (PT_LOAD, 0x8000, 0xf0000000L, 0xa0100000L, 0x24460, 0x24460,
                        PF_R | PF_W | PF_X, 0x8000)],
    "data/arm_scatter_load": [ (PT_LOAD, 0x34, 0x00100000L, 0x00100000L, 0x02330, 0x0245c,
                                0x80000000L | PF_R | PF_W | PF_X, 0x4)],
    "data/ia32_exec": [ (PT_LOAD, 0xc0,   0xf0100200L, 0x00100200L, 0xa74a, 0xa74a,
                         PF_R | PF_W | PF_X, 0x10),
                        (PT_LOAD, 0xb000, 0xf010c000L, 0x0010c000L, 0xf848, 0xf848,
                         PF_R | PF_W | PF_X, 0x800),
                        (PT_LOAD, 0x1b000, 0x0013c000L, 0x0013c000L, 0xaf9c, 0xaf9c,
                         PF_R | PF_W | PF_X, 0x1000),
                        (PT_GNU_STACK, 0x0,    0x0,        0x0,        0x0,    0x0,
                         PF_R | PF_W | PF_X, 0x4)],
    "data/ia64_exec": [ (PT_PHDR, 0x40, 0x4000000000000040L, 0x4000000000000040L, 0x188, 0x188,
                         PF_R | PF_X, 8),
                        (PT_INTERP, 0x1c8, 0x40000000000001c8L, 0x40000000000001c8L, 0x18, 0x18,
                         PF_R, 1),
                        (PT_LOAD, 0x0, 0x4000000000000000L, 0x4000000000000000L, 0x25eb8, 0x25eb8,
                         PF_R | PF_X, 0x10000),
                        (PT_LOAD, 0x25eb8, 0x6000000000005eb8L, 0x6000000000005eb8L, 0x1100, 0x1778,
                         PF_R | PF_W, 0x10000),
                        (PT_DYNAMIC, 0x25ee0, 0x6000000000005ee0L, 0x6000000000005ee0L, 0x200, 0x200,
                         PF_R | PF_W, 8),
                        (PT_NOTE, 0x1e0, 0x40000000000001e0L, 0x40000000000001e0L, 0x20, 0x20,
                         PF_R, 4),
                        (1879048193L, 0x250c0L, 0x40000000000250c0L, 0x40000000000250c0L, 0xdf8, 0xdf8,
                         PF_R, 8),
    ],
    "data/mips64_exec": [(PT_LOAD, 0x1000, 0x80050000L, 0x00050000L, 0x16545L, 0x16545L,
                          PF_R | PF_X, 0x1000),
                         (PT_LOAD, 0x18000, 0x80067000L, 0x00067000L, 0xea18, 0xea18,
                          PF_R | PF_W | PF_X, 0x1000)],
    "data/mips32_exec": [(PT_LOAD, 0x10000, 0x80050000L, 0x00050000L, 0x21a88, 0x21f50,
                          PF_R | PF_W | PF_X, 0x10000),
                         (PT_LOAD, 0x31f50, 0x80071f50L, 0x00071f50L, 0x02ee0, 0x02ee0,
                          PF_R | PF_W | PF_X, 0x10000),
                         (PT_LOAD, 0x40000, 0x00100000L, 0x00480000L, 0x112a0, 0x24000,
                          PF_R | PF_W | PF_X, 0x10000),
                         (PT_LOAD, 0x512a0, 0x00170000L, 0x00075000L, 0x02000, 0x02000,
                          PF_R | PF_W, 0x0),
                         (PT_LOAD, 0x60000, 0x00130000L, 0x004a8000L, 0x09100, 0x09228,
                          PF_R | PF_W | PF_X, 0x10000),
                         (PT_LOAD, 0x70000, 0x00140000L, 0x004b8000L, 0x0a130, 0x0a278,
                          PF_R | PF_W | PF_X, 0x10000),
                         (PT_LOAD, 0x80000, 0x00150000L, 0x004c8000L, 0x18140, 0x1de08,
                          PF_R | PF_W | PF_X, 0x10000)],
    }

class TestElfProgramHeader(unittest.TestCase):
    def test_invalid_endianess(self):
        self.assertRaises(InvalidArgument, Elf32ProgramHeader, "foo")
            

    def test_bogus_data32_size(self):
        ph = Elf32ProgramHeader("<")
        f = File("data/arm_exec", "rb")
        self.assertRaises(elf.structures.ElfFormatError, ph.fromdata, f.get_data(0, 16), ph)
    
    def test_bogus_data64_size(self):
        ph = Elf64ProgramHeader("<")
        f = File("data/arm_exec", "rb")
        self.assertRaises(elf.structures.ElfFormatError, ph.fromdata, f.get_data(0, 16), ph)

    def test_expected(self):
        for file_name in elf_ph_expected:
            f = File(file_name, "rb")
            ident = ElfIdentification()
            ident.fromdata(f.get_data(0, 16))
            if ident.wordsize == 32:
                data = f.get_data(0, Elf32Header.size())
                header = Elf32Header()
                PhClass = Elf32ProgramHeader
                header.fromdata(data)
            elif ident.wordsize == 64:
                data = f.get_data(0, Elf64Header.size())
                header = Elf64Header()
                header.fromdata(data)
                PhClass = Elf64ProgramHeader
            else:
                raise "Problem"
            self.assertEqual(header.e_phnum, len(elf_ph_expected[file_name]))
            entsize = header.e_phentsize
            wordsize = ident.wordsize
            hdr = ELF_HEADER_CLASSES[wordsize]()
            for i in range(header.e_phnum):
                ph_expect = elf_ph_expected[file_name][i]
                ph_data = f.get_data(header.e_phoff + (i * entsize), entsize)
                ph = PhClass(header.ident.endianess)
                ph.fromdata(ph_data, hdr)
                self.assertEqual(ph.todata(), ph_data)
                self.assertEqual(ph.p_type, ph_expect[0])
                self.assertEqual(ph.p_offset, ph_expect[1])
                self.assertEqual(ph.p_vaddr, ph_expect[2])
                self.assertEqual(ph.p_paddr, ph_expect[3])
                self.assertEqual(ph.p_filesz, ph_expect[4])
                self.assertEqual(ph.p_memsz, ph_expect[5])
                self.assertEqual(ph.p_flags, ph_expect[6])
                self.assertEqual(ph.p_align, ph_expect[7])
                                     
    def test_against_readelf(self):
        if not have_readelf:
            return 
        for file_name in elf_ph_expected:
            f = File(file_name, "rb")
            ident = ElfIdentification()
            ident.fromdata(f.get_data(0, 16))
            wordsize = ident.wordsize
            hdr = ELF_HEADER_CLASSES[wordsize]()
            lines = os.popen("readelf -l %s" % file_name).readlines()

            good_lines = []

            if ident.wordsize == 32:
                data = f.get_data(0, Elf32Header.size())
                header = Elf32Header()
                PhClass = Elf32ProgramHeader
                header.fromdata(data)
                
                got_start = False
                for line in lines:
                    if got_start:
                        if line.strip() == "":
                            break
                        good_lines.append(line)
                    else:
                        # Looking for start
                        if line.startswith("  Type"):
                            got_start = True
            elif ident.wordsize == 64:
                data = f.get_data(0, Elf64Header.size())
                header = Elf64Header()
                header.fromdata(data)
                PhClass = Elf64ProgramHeader

                got_start = False
                for line in lines:
                    if got_start:
                        if line.strip() == "":
                            break
                        if not line.strip().startswith("["):
                            good_lines.append(line)
                    else:
                        # Looking for start
                        if line.strip().startswith("FileSiz"):
                            got_start = True
                merged_lines = []
                i = 0
                while i < len(good_lines):
                    merged_lines.append(good_lines[i] + good_lines[i+1])
                    i += 2
                good_lines = merged_lines

            entsize = header.e_phentsize
            for i in range(header.e_phnum):
                ph_data = f.get_data(header.e_phoff + (i * entsize), entsize)
                ph = PhClass(header.ident.endianess)
                ph.fromdata(ph_data, hdr)

                out = StringIO.StringIO()
                ph.output(out)
                out.getvalue()
                self.assertEqual(out.getvalue(), good_lines[i])


A = SHF_ALLOC
AX = SHF_ALLOC | SHF_EXECINSTR
WA = SHF_ALLOC | SHF_WRITE
WAX = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR
MS = SHF_STRINGS | SHF_MERGE

elf_sh_expected = {
    "data/arm_exec": [ (0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0, 0),
                       (27, SHT_PROGBITS, 0xf0000000L, 0x8000L, 0xbb24, 0, AX, 0, 0, 4096),
                       (33, SHT_PROGBITS, 0xf000bb24L, 0x13b24L, 0x0, 0, AX, 0, 0, 4),
                       (41, SHT_PROGBITS, 0xf000bb24L, 0x13b24L, 0x0, 0, AX, 0, 0, 4),
                       (50, SHT_PROGBITS, 0xf000c000L, 0x14000L, 0x9d0, 0, WA, 0, 0, 4),
                       (55, SHT_PROGBITS, 0xf0010000L, 0x18000L, 0xc070, 0, WA, 0, 0, 16384),
                       (61, SHT_PROGBITS, 0xf001c070L, 0x24070L, 0x457c, 0, AX, 0, 0, 4),
                       (69, SHT_PROGBITS, 0xf00205ecL, 0x285ecL, 0x2e4, 0, WA, 0, 0, 4),
                       (75, SHT_PROGBITS, 0xf0021000L, 0x29000L, 0x3460, 0, WAX, 0, 0, 4),
                       
                       (81, SHT_PROGBITS, 0, 0x2c460, 0xc6ba, 0, 0, 0, 0, 1),
                       (95, SHT_PROGBITS, 0, 0x38b1a, 0xdc3ea, 0, 0, 0, 0, 1),
                       (107, SHT_PROGBITS, 0, 0x114f04, 0xcfb1, 0, 0, 0, 0, 1),
                       (119, SHT_PROGBITS, 0, 0x121eb8, 0x1aa8, 0, 0, 0, 0, 4),
                       (132, SHT_PROGBITS, 0, 0x123960, 0x2f70, 0, 0, 0, 0, 1),
                       (148, SHT_PROGBITS, 0, 0x1268d0, 0x918, 0, 0, 0, 0, 1),
                       (163, SHT_PROGBITS, 0, 0x1271e8, 0x2848, 0, 0, 0, 0, 1),
                       (177, SHT_PROGBITS, 0, 0x129a30, 0xa705, 1, MS, 0, 0, 1),
                       
                       (17, SHT_STRTAB, 0, 0x134135, 0xbc, 0, 0, 0, 0, 1),
                       (1, SHT_SYMTAB, 0, 0x134514, 0x58b0, 0x10, 0, 19, 967, 4),
                       (9, SHT_STRTAB, 0, 0x139dc4, 0x5132, 0, 0, 0, 0, 1),
                       ],

    "data/arm_scatter_load": [ (0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0, 0),
                       ( 1, SHT_PROGBITS, 0x00100000L, 0x0034L, 0x0124, 0, AX, 0, 0, 4),
                       ( 9, SHT_PROGBITS, 0x00180000L, 0x0158L, 0x0020, 0, AX, 0, 0, 4),
                       (13, SHT_PROGBITS, 0x00200000L, 0x0178L, 0x0020, 0, AX, 0, 0, 4),
                       (17, SHT_PROGBITS, 0x00300000L, 0x0198L, 0x0020, 0, AX, 0, 0, 4),
                       (23, SHT_PROGBITS, 0x00400000L, 0x01b8L, 0x0020, 0, AX, 0, 0, 4),
                       (28, SHT_PROGBITS, 0x00500000L, 0x01d8L, 0x0020, 0, AX, 0, 0, 4),
                       (33, SHT_PROGBITS, 0x00600000L, 0x01f8L, 0x0020, 0, AX, 0, 0, 4),
                       (37, SHT_PROGBITS, 0x01100000L, 0x0218L, 0x2148, 0, AX, 0, 0, 4),
                       (43, SHT_PROGBITS, 0x00900000L, 0x2360L, 0x0004, 0, WA, 0, 0, 4),
                       (43, SHT_NOBITS,   0x00900004L, 0x2364L, 0x012c, 0, WA, 0, 0, 4),
                       (48, SHT_PROGBITS, 0x00000000L, 0x2364L, 0x0d8c, 0, 0, 0, 0, 0),
                       (61, SHT_SYMTAB,   0,           0x30f0L, 0x1710, 0x10, 0, 13, 255, 4),
                       (69, SHT_STRTAB,   0,           0x4800L, 0x0a2c, 0, 0, 0, 0, 0),
                       (77, SHT_NOTE,     0,           0x522cL, 0x0038, 0, 0, 0, 0, 4),
                       (83, SHT_PROGBITS, 0,           0x5264L, 0x5228, 0, 0, 0, 0, 0),
                       (92, SHT_STRTAB,   0,           0xa48cL, 0x0068, 0, 0, 0, 0, 0),
                       ],

    "data/amd64_exec": [ (0, SHT_NULL, 0, 0, 0, 0, 0, 0, 0, 0),
                         (11, SHT_PROGBITS,  0x400200L, 0x200,   0x1c,   0, A, 0, 0, 1),
                         (19, SHT_NOTE,      0x40021cL, 0x21c,   0x20,      0, A, 0, 0, 4),
                         (33, SHT_HASH,      0x400240L, 0x240,   0x324,      0x4, A, 4, 0, 8),
                         (39, SHT_DYNSYM,    0x400568L, 0x568,   0x990,    0x18, A, 5, 1, 8),
                         (47, SHT_STRTAB,    0x400ef8L, 0xef8,   0x406,   0, A, 0, 0, 1),
                         (55, SHT_VERSYM,    0x4012feL, 0x12fe,  0xcc,   0x2, A, 4, 0, 2),
                         (68, SHT_VERNEED,   0x4013d0L, 0x13d0,  0x70,    0, A, 5, 3, 8),
                         (83, SHT_RELA,      0x401440L, 0x1440,  0x78,   0x18, A, 4, 0, 8),
                         (93, SHT_RELA,      0x4014b8L, 0x14b8,  0x858,   0x18, A, 4, 11, 8),
                         (103, SHT_PROGBITS, 0x401d10,  0x1d10,  0x18,  0, AX, 0, 0, 4),
                         (98, SHT_PROGBITS,  0x401d28,  0x1d28,  0x5a0,   0x10, AX, 0, 0, 4),
                         (109, SHT_PROGBITS, 0x4022d0,  0x22d0,  0xb178,   0, AX, 0, 0, 16),
                         (115, SHT_PROGBITS, 0x40d448,  0xd448,  0xe,   0, AX, 0, 0, 4),
                         (121, SHT_PROGBITS, 0x40d460,  0xd460,  0x4600,    0, A, 0, 0, 32),
                         (129, SHT_PROGBITS, 0x411a60,  0x11a60, 0x504,   0, A, 0, 0, 4),
                         (143, SHT_PROGBITS, 0x411f68,  0x11f68, 0x15f4,   0, A, 0, 0, 8),
                         (153, SHT_PROGBITS, 0x514000,  0x14000, 0x10,    0, WA, 0, 0, 8),
                         (160, SHT_PROGBITS, 0x514010,  0x14010, 0x10,   0, WA, 0, 0, 8),
                         (167, SHT_PROGBITS, 0x514020,  0x14020, 0x8,   0, WA, 0, 0, 8),
                         (172, SHT_DYNAMIC,  0x514028,  0x14028, 0x1b0,   0x10, WA, 5, 0, 8),
                         (181, SHT_PROGBITS, 0x5141d8,  0x141d8, 0x10,    0x8, WA, 0, 0, 8),
                         (186, SHT_PROGBITS, 0x5141e8,  0x141e8, 0x2e0,   0x8, WA, 0, 0, 8),
                         (195, SHT_PROGBITS, 0x5144e0,  0x144e0, 0x190,   0, WA, 0, 0, 32),
                         (201, SHT_NOBITS,   0x514680,  0x14670, 0x4d8, 0, WA, 0, 0, 32),
                         (1,   SHT_STRTAB,   0,         0x14670, 0xce,   0, 0,  0, 0, 1),
                         ],

    }

elf_sh_files = [
    "data/arm_exec",
    "data/arm_scatter_load",
    "data/ia32_exec",
    "data/mips64_exec",
    "data/amd64_exec",
    "data/ia64_exec"
    ]

class TestElfSection(unittest.TestCase):
    def setUp(self):
        self.section = Elf32SectionHeader('<')

    def test_invalid_endianess(self):
        self.assertRaises(InvalidArgument, Elf32SectionHeader, "foo")

    def test_bogus_data32_size(self):
        ph = Elf32SectionHeader("<")
        f = File("data/arm_exec", "rb")
        self.assertRaises(elf.structures.ElfFormatError, ph.fromdata, f.get_data(0, 16), ph)
    
    def test_bogus_data64_size(self):
        ph = Elf32SectionHeader("<")
        f = File("data/arm_exec", "rb")
        self.assertRaises(elf.structures.ElfFormatError, ph.fromdata, f.get_data(0, 16), ph)

    def test_create(self):
        self.failIfEqual(Elf32SectionHeader('<'), None)
        self.failIfEqual(Elf32SectionHeader('>'), None)
        self.failIfEqual(Elf64SectionHeader('<'), None)
        self.failIfEqual(Elf64SectionHeader('>'), None)

    def test_isnull(self):
        for sect_class in [Elf32SectionHeader, Elf64SectionHeader]:
            sect = sect_class('<')
            self.assertEqual(sect.sh_name, 0)
            self.assertEqual(sect.sh_type, SHT_NULL)
            self.assertEqual(sect.sh_flags, 0)
            self.assertEqual(sect.sh_addr, 0)
            self.assertEqual(sect.sh_offset, 0)
            self.assertEqual(sect.sh_size, 0)
            self.assertEqual(sect.sh_link, SHN_UNDEF)
            self.assertEqual(sect.sh_info, 0)
            self.assertEqual(sect.sh_addralign, 0)
            self.assertEqual(sect.sh_entsize, 0)

    def test_name(self):
        self.section.sh_name = 5
        self.assertEqual(self.section.sh_name, 5)

    def test_type(self):
        self.section.sh_type = SHT_PROGBITS
        self.assertEqual(self.section.sh_type, SHT_PROGBITS)
        self.assertEqual(str(self.section.sh_type), "PROGBITS")
                         
    def test_flags(self):
        self.section.sh_flags = SHF_WRITE
        self.assertEqual(self.section.sh_flags, SHF_WRITE)
        
    def test_addr(self):
        self.section.sh_addr = 0x1000
        self.assertEqual(self.section.sh_addr, 0x1000)

    def test_offset(self):
        self.section.sh_offset = 0x9
        self.assertEqual(self.section.sh_offset, 0x9)

    def test_size(self):
        self.section.sh_size = 37
        self.assertEqual(self.section.sh_size, 37)

    def test_link(self):
        self.section.sh_link = 12
        self.assertEqual(self.section.sh_link, 12)

    def test_info(self):
        self.section.sh_info = 2
        self.assertEqual(self.section.sh_info, 2)

    def test_addralign(self):
        self.section.sh_addralign = 0x2000
        self.assertEqual(self.section.sh_addralign, 0x2000)

    def test_entsize(self):
        self.section.sh_entsize = 8
        self.assertEqual(self.section.sh_entsize, 8)

    def test_expected(self):
        for file_name in elf_sh_expected:
            f = File(file_name, "rb")
            ident = ElfIdentification()
            ident.fromdata(f.get_data(0, 16))
            wordsize = ident.wordsize
            hdr = ELF_HEADER_CLASSES[wordsize]()
            if ident.wordsize == 32:
                data = f.get_data(0, Elf32Header.size())
                header = Elf32Header()
                ShClass = Elf32SectionHeader
                header.fromdata(data)
            elif ident.wordsize == 64:
                data = f.get_data(0, Elf64Header.size())
                header = Elf64Header()
                header.fromdata(data)
                ShClass = Elf64SectionHeader
            else:
                raise "Problem"
            self.assertEqual(header.e_shnum, len(elf_sh_expected[file_name]))
            entsize = header.e_shentsize
            for i in range(header.e_shnum):
                sh_expect = elf_sh_expected[file_name][i]
                sh_data = f.get_data(header.e_shoff + (i * entsize), entsize)
                sh = ShClass(header.ident.endianess)
                sh.fromdata(sh_data, hdr)
                self.assertEqual(sh.todata(), sh_data)
                self.assertEqual(sh.sh_name, sh_expect[0])
                self.assertEqual(sh.sh_type, sh_expect[1])
                self.assertEqual(sh.sh_addr, sh_expect[2])
                self.assertEqual(sh.sh_offset, sh_expect[3])
                self.assertEqual(sh.sh_size, sh_expect[4])
                self.assertEqual(sh.sh_entsize, sh_expect[5])
                self.assertEqual(sh.sh_flags, sh_expect[6])
                self.assertEqual(sh.sh_link, sh_expect[7])
                self.assertEqual(sh.sh_info, sh_expect[8])
                self.assertEqual(sh.sh_addralign, sh_expect[9])

    def test_elfsection_output(self):
        es = Elf32SectionHeader('<')
        out = StringIO.StringIO()
        es.output(out)
        self.assertEqual(out.getvalue(), "NULL            00000000 000000 000000 00      0   0  0\n")
        out = StringIO.StringIO()
        es._index = 3
        es._name = "pants"
        es.output(out)
        self.assertEqual(out.getvalue(), "  [ 3] pants             NULL            00000000 000000 000000 00      0   0  0\n")


    def test_get_markedup_flags(self):
        # Each one individually
        es = Elf32SectionHeader('<')
        self.assertEqual(es.get_markedup_flags(), "")
        es.sh_flags = SHF_WRITE
        self.assertEqual(es.get_markedup_flags(), "W")
        es.sh_flags = SHF_ALLOC
        self.assertEqual(es.get_markedup_flags(), "A")
        es.sh_flags = SHF_EXECINSTR
        self.assertEqual(es.get_markedup_flags(), "X")
        es.sh_flags = SHF_MERGE
        self.assertEqual(es.get_markedup_flags(), "M")
        es.sh_flags = SHF_STRINGS
        self.assertEqual(es.get_markedup_flags(), "S")
        es.sh_flags = SHF_LINK_ORDER
        self.assertEqual(es.get_markedup_flags(), "L")
        es.sh_flags = SHF_MASKPROC
        self.assertEqual(es.get_markedup_flags(), "p")

        # Test ordering in multiple
        es.sh_flags = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR
        self.assertEqual(es.get_markedup_flags(), "WAX")
        

    def test_elfsection64_output(self):
        es = Elf64SectionHeader('<')
        out = StringIO.StringIO()
        es.output(out)
        expected = "NULL             0000000000000000  00000000\n       0000000000000000  0000000000000000           0     0     0\n"
        self.assertEqual(out.getvalue(), expected)
        out = StringIO.StringIO()
        es._index = 3
        es._name = "pants"
        es.output(out)
        expected = "  [ 3] pants             NULL             0000000000000000  00000000\n       0000000000000000  0000000000000000           0     0     0\n"
        self.assertEqual(out.getvalue(), expected)


    def test_against_readelf(self):
        if not have_readelf:
            return 
        for file_name in elf_sh_files:
            f = File(file_name, "rb")
            ident = ElfIdentification()
            ident.fromdata(f.get_data(0, 16))
            wordsize = ident.wordsize
            hdr = ELF_HEADER_CLASSES[wordsize]()
            lines = os.popen("readelf -S %s" % file_name).readlines()

            good_lines = []

            if ident.wordsize == 32:
                data = f.get_data(0, Elf32Header.size())
                header = Elf32Header()
                ShClass = Elf32SectionHeader
                header.fromdata(data)
                
                got_start = False
                for line in lines:
                    if got_start:
                        if line.strip().startswith("Key to Flags"):
                            break
                        good_lines.append(line)
                    else:
                        # Looking for start
                        if line.strip().startswith("[Nr]"):
                            got_start = True
            elif ident.wordsize == 64:
                data = f.get_data(0, Elf64Header.size())
                header = Elf64Header()
                header.fromdata(data)
                ShClass = Elf64SectionHeader

                got_start = False
                for line in lines:
                    if got_start:
                        if line.strip().startswith("Key"):
                            break
                        good_lines.append(line)
                    else:
                        # Looking for start
                        if line.strip().startswith("Siz"):
                            got_start = True
                merged_lines = []
                i = 0
                while i < len(good_lines):
                    merged_lines.append(good_lines[i] + good_lines[i+1])
                    i += 2
                good_lines = merged_lines

            good_lines = [line[25:] for line in good_lines]
            entsize = header.e_shentsize
            for i in range(header.e_shnum):
                sh_data = f.get_data(header.e_shoff + (i * entsize), entsize)
                sh = ShClass(header.ident.endianess)
                sh.fromdata(sh_data, hdr)

                out = StringIO.StringIO()
                sh.output(out)
                out.getvalue()
                self.assertEqual(out.getvalue(), good_lines[i])
