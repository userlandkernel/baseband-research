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
import array
import copy
import os
import StringIO
from elf.ByteArray import ByteArray
import elf.core
from elf.File import File
from elf.structures import Elf32ProgramHeader, Elf32SectionHeader
from elf.core import UnpreparedElfSection, UnpreparedElfStringTable, UnpreparedElfFile, \
     InvalidArgument, PreparedElfFile, BaseElfFile
from elf.constants import PT_LOAD, PT_NULL, PT_PHDR, \
     SHT_NULL, SHT_NOBITS, SHT_PROGBITS, ElfFormatError
from elf.segment import ElfSegment, SectionedElfSegment, DataElfSegment, HeaderElfSegment
from elf.util import Unprepared, Prepared
from elf.section import BaseElfSection

modules_under_test = [elf.core, elf.segment, elf.section]

elf_address = {
    "data/null_elf" : (0, None, 0, None, 0, 0),

    "data/arm_exec" : (0xf0028000L, 0xa0128000L, 0xf0000000L, 0xa0100000L, 20, 1),
    "data/arm_exec_nosect" : (0xf0028000L, 0x43d3000, 0x4330000, 0x04100000, 0, 15),
    "data/arm_object" : (0x1bc, None, 0, None, 9, 0),
    "data/arm_scatter_load" : (0x0010245CL, 0x0010245CL, 0x00100000L, 0x00100000L, 17, 1),
    "data/amd64_exec" : (0x600000L, 0x600000L, 0x400000L, 0x400000L, 26, 8),
    "data/ia32_exec" : (0xF011C000L, 0x147000L, 0x13c000L, 0x00100200L, 22, 4),
    "data/ia64_exec" : (0x6000000000010000L, 0x6000000000010000L, 0x4000000000000000L, 0x4000000000000000L, 31, 7),
    "data/mips64_exec" : (0xffffffff80076000L, 0x76000, 0xffffffff80050000L, 0x50000, 11, 2)
    }



class TestElfSymbol(unittest.TestCase):
    def test_init(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            ef.init_and_prepare_from_filename(fn)
            if not ef.sections: continue
            for section in ef.sections:
                for symbols in section.symbols:
                    pass

class TestElfSegment(unittest.TestCase):
    def test_init(self):
        ElfSegment()
        ElfSegment(PT_LOAD)
        ElfSegment(program_header_type = PT_LOAD)
        sections = [UnpreparedElfSection("foo")]
        self.assertRaises(TypeError, ElfSegment,
                          data = ByteArray("pants"), sections = sections)


    def test_copy(self):
        seg = DataElfSegment()
        new_seg = seg.copy()

        seg = SectionedElfSegment()
        seg.sections = [UnpreparedElfSection("test")]
        new_seg = seg.copy()

        seg = DataElfSegment()
        seg._data = ByteArray()
        new_seg = seg.copy()

        seg = DataElfSegment()
        seg.prepare(34)
        new_seg = seg.copy()

        seg = HeaderElfSegment()
        new_Seg = seg.copy()

    def test_setdata(self):
        ef = DataElfSegment()
        data = ByteArray("foo")
        ef.set_data(data)
        self.assertEqual(ef.get_file_data(), data)

        ef = SectionedElfSegment(sections = [])
        self.assertRaises(InvalidArgument, ef.set_data, data)


    def test_get_file_data(self):
        ef = SectionedElfSegment(sections = [])
        self.assertRaises(InvalidArgument, ef.get_file_data)

        data = ByteArray("pants")
        ef = DataElfSegment(data = data)
        self.assertEqual(ef.get_file_data(), data)

    def test_has_sections(self):
        seg = DataElfSegment()
        self.assertEquals(seg.has_sections(), False)

        seg = SectionedElfSegment(sections = [])
        self.assertEquals(seg.has_sections(), True)

        seg = HeaderElfSegment()
        self.assertEquals(seg.has_sections(), False)
    
    def test_get_section(self):
        ef = SectionedElfSegment(sections = [])
        self.assertEquals(ef.get_sections(), [])

        data = ByteArray("pants")
        ef = DataElfSegment(data = data)
        self.assertRaises(InvalidArgument, ef.get_sections)

        seg = HeaderElfSegment()
        self.assertRaises(InvalidArgument, seg.get_sections)

    def test_replace_section(self):
        seg = SectionedElfSegment(sections=[UnpreparedElfSection("test")])
        old_section = seg.get_sections()[0]
        new_section = UnpreparedElfSection("new")
        seg.replace_section(old_section, new_section)
        self.assertEqual(seg.get_sections(), [new_section])
        new_seg = seg.copy()

        self.assertRaises(InvalidArgument, seg.replace_section, None, new_section)
    
    def test_prepare(self):
        ef = DataElfSegment()
        ef.prepare(37)
        self.assertEqual(ef.prepared, True)
        self.assertEqual(ef.offset, 37)
        self.assertRaises(InvalidArgument, ef.prepare, 12)

        ef = DataElfSegment()
        self.assertRaises(InvalidArgument, ef.prepare, 37, 12)

        ef = HeaderElfSegment()
        self.assertRaises(InvalidArgument, ef.prepare, 12)
        ef.prepare(37, 12)
        self.assertEqual(ef.get_memsz(), 12)
        self.assertEqual(ef.offset, 37)


    def test_gettype(self):
        seg = ElfSegment()
        self.assertEquals(seg.type, PT_NULL)
        seg = ElfSegment(PT_LOAD)
        self.assertEquals(seg.type, PT_LOAD)

    def test_settype(self):
        seg = ElfSegment()
        self.assertEquals(seg.type, PT_NULL)
        seg.type = PT_LOAD
        self.assertEquals(seg.type, PT_LOAD)

    def test_getvaddr(self):
        seg = ElfSegment()
        self.assertEquals(seg.vaddr, 0)
        seg = ElfSegment(vaddr = 37)
        self.assertEquals(seg.vaddr, 37)

    def test_setvaddr(self):
        seg = ElfSegment()
        self.assertEquals(seg.vaddr, 0)
        seg.vaddr = 37
        self.assertEquals(seg.vaddr, 37)

    def test_getpaddr(self):
        seg = ElfSegment()
        self.assertEquals(seg.paddr, 0)
        seg = ElfSegment(paddr = 37)
        self.assertEquals(seg.paddr, 37)

    def test_setpaddr(self):
        seg = ElfSegment()
        self.assertEquals(seg.paddr, 0)
        seg.paddr = 37
        self.assertEquals(seg.paddr, 37)

    def test_getalign(self):
        seg = ElfSegment()
        self.assertEquals(seg.align, 0)
        seg = ElfSegment(align = 37)
        self.assertEquals(seg.align, 37)

    def test_setalign(self):
        seg = ElfSegment()
        self.assertEquals(seg.align, 0)
        seg.align = 37
        self.assertEquals(seg.align, 37)

    def test_getflags(self):
        seg = ElfSegment()
        self.assertEquals(seg.flags, 0)
        seg = ElfSegment(flags = 37)
        self.assertEquals(seg.flags, 37)

    def test_setflags(self):
        seg = ElfSegment()
        self.assertEquals(seg.flags, 0)
        seg.flags = 37
        self.assertEquals(seg.flags, 37)

    def test_getmemsz(self):
        seg = DataElfSegment()
        self.assertRaises(InvalidArgument, seg.get_memsz)
        data = ByteArray()
        data.memsz = 10
        seg.set_data(data)
        self.assertEquals(seg.get_memsz(), 10)

        seg = HeaderElfSegment()
        self.assertRaises(Unprepared, seg.get_memsz)

    def test_getfilesz(self):
        seg = DataElfSegment()
        self.assertRaises(InvalidArgument, seg.get_filesz)
        data = ByteArray("1234567890")
        data.memsz = 10
        seg.set_data(data)
        self.assertEquals(seg.get_filesz(), 10)
        seg = HeaderElfSegment()
        self.assertRaises(Unprepared, seg.get_filesz)

        seg = SectionedElfSegment()
        self.assertEqual(seg.get_filesz(), 0)

    def test_getoffset(self):
        seg = ElfSegment()
        self.assertRaises(Unprepared, seg.get_offset)
        seg.prepare(10)
        self.assertEquals(seg.offset, 10)

    def test_get_program_header(self):
        data = ByteArray()
        data.memsz = 0
        seg = DataElfSegment(data = data)
        self.assertRaises(Unprepared, seg.get_program_header, '<', 32)
        seg.prepare(0)
        self.assertRaises(InvalidArgument, seg.get_program_header, '<', 33)
        self.assertRaises(InvalidArgument, seg.get_program_header, 'ads', 32)

        ph = seg.get_program_header('<', 32)
        self.assertEquals(ph.todata(), Elf32ProgramHeader('<').todata())

    def test_remove_section(self):
        seg = SectionedElfSegment()
        section = UnpreparedElfSection()
        self.assertRaises(InvalidArgument, seg.remove_section, section)
        seg.add_section(section)
        self.assertEqual(section in seg.sections, True)
        seg.remove_section(section)
        self.assertEqual(section in seg.sections, False)

        self.assertRaises(InvalidArgument, seg.remove_section, section)

        seg = DataElfSegment()
        data = ByteArray("foo")
        seg.set_data(data)
        self.assertRaises(InvalidArgument, seg.remove_section, None)

        seg = HeaderElfSegment()
        self.assertRaises(InvalidArgument, seg.remove_section, None)
  

    def test_add_section(self):
        seg = DataElfSegment()
        data = ByteArray("foo")
        seg.set_data(data)
        section = UnpreparedElfSection()
        self.assertRaises(InvalidArgument, seg.add_section, section)
        
        seg = SectionedElfSegment()
        seg.add_section(section)
        self.assertEqual(section in seg.sections, True)

    def test_remove_nobits(self):
        seg = DataElfSegment()
        self.assertRaises(InvalidArgument, seg.remove_nobits)       

    def test___str__(self):
        seg = ElfSegment()
        self.assertEqual(str(seg), "<ElfSegment NULL>")
        seg = ElfSegment(program_header_type = PT_LOAD)
        self.assertEqual(str(seg), "<ElfSegment LOAD>")

    def test_base_class(self):
        seg = ElfSegment()
        self.assertRaises(NotImplementedError, seg.set_data, None)
        self.assertRaises(NotImplementedError, seg.remove_nobits)
        self.assertRaises(NotImplementedError, seg.get_memsz)
        self.assertRaises(NotImplementedError, seg.get_filesz)
        self.assertRaises(NotImplementedError, seg.get_file_data)
        self.assertRaises(NotImplementedError, seg.remove_section, None)
        self.assertRaises(NotImplementedError, seg.has_sections)
        self.assertRaises(NotImplementedError, seg.get_sections)
        self.assertRaises(NotImplementedError, seg.add_section, None)
        self.assertRaises(NotImplementedError, seg.copy)
    

class TestElfSection(unittest.TestCase):
    def test_init(self):
        UnpreparedElfSection("test")
        UnpreparedElfSection("test", SHT_PROGBITS)
        UnpreparedElfSection("test", section_type = SHT_PROGBITS)
        UnpreparedElfSection("test", entsize = 10)

    def test_copy(self):
        sect = BaseElfSection("test")
        self.assertRaises(NotImplementedError, sect.copy)

        sect = UnpreparedElfSection("test")
        new_sect = sect.copy()
        self.assertEquals(sect.name, new_sect.name)

        prep_sect = sect.prepare(0, 0, 0, 32, '<')
        new_sect = prep_sect.copy()
        self.assertEquals(prep_sect.name, new_sect.name)

        sect = UnpreparedElfSection("test", SHT_NOBITS)
        new_sect = sect.copy()
        self.assertEquals(sect.name, new_sect.name)

        prep_sect = sect.prepare(0, 0, 0, 32, '<')
        new_sect = prep_sect.copy()
        self.assertEquals(prep_sect.name, new_sect.name)

        sect = UnpreparedElfStringTable("string")
        strings = ["foo", "bar", "baz"]
        for string in strings:
            sect.add_string(string)
        new_sect = sect.copy()
        for i in range(len(strings)):
            self.assertEquals(sect.get_string_idx(i), new_sect.get_string_idx(i))
            

    def test_gettype(self):
        sect = UnpreparedElfSection("test", SHT_PROGBITS)
        self.assertEqual(sect.type, SHT_PROGBITS)

    def test_settype(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.type, SHT_NULL)
        sect.type = SHT_PROGBITS
        self.assertEqual(sect.type, SHT_PROGBITS)

    def test_getname(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.name, "test")

    def test_setname(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.name, "test")
        sect.name = "pants"
        self.assertEqual(sect.name, "pants")

    def test_setname_prepared(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.name, "test")
        sect = sect.prepare(0, 0, 0, 32, '<')
        self.assertEqual(hasattr(sect, "set_name"), False)

    def test_get_name_offset(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(hasattr(sect, "name_offset"), False)
        sect = sect.prepare(0, 0, 5, 32, '<')
        self.assertEqual(sect.name_offset, 5)

    def test_set_name_offset(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(hasattr(sect, "name_offset"), False)
        sect = sect.prepare(0, 0, 5, 32, '<')
        self.assertEqual(sect.name_offset, 5)
        sect.name_offset = 25
        self.assertEqual(sect.name_offset, 25)

    def test_get_address(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.address, 0)
        sect = UnpreparedElfSection("test", address=25)
        self.assertEqual(sect.address, 25)

    def test_set_address(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.address, 0)
        sect.address = 25
        self.assertEqual(sect.address, 25)

    def test_get_size(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.get_size(), 0)

        sect = UnpreparedElfSection("test", data = ByteArray("pants"))
        self.assertEqual(sect.get_size(), 5)

        sect = UnpreparedElfSection("test", SHT_NOBITS)
        sect.set_size(40)
        self.assertEqual(sect.get_size(), 40)

    def test_set_size(self):
        sect = UnpreparedElfSection("test")
        sect.set_size(37)
        self.assertEqual(sect.get_size(), 37)

        sect = UnpreparedElfSection("test", SHT_NOBITS)
        sect.set_size(37)
        self.assertEqual(sect.get_size(), 37)
        
        sect = UnpreparedElfSection("test", data = ByteArray("pants"))
        sect.set_size(3)
        self.assertEqual(sect.get_size(), 3)

        sect = UnpreparedElfSection("test", data = ByteArray("pants"))
        sect.set_size(10)
        self.assertEqual(sect.get_size(), 10)

        sect = UnpreparedElfSection("test", data = ByteArray("pants"))
        sect = sect.prepare(0, 0, 0, 32, '<')
        sect.set_size(4)
        self.assertEqual(sect.get_size(), 4)

        sect = UnpreparedElfSection("test", data = ByteArray("pants"))
        sect = sect.prepare(0, 0, 0, 32, '<')
        self.assertRaises(InvalidArgument, sect.set_size, 14)

        sect = UnpreparedElfSection("test", SHT_NOBITS)
        sect.set_size(37)
        sect = sect.prepare(0, 0, 0, 32, '<')
        self.assertRaises(InvalidArgument, sect.set_size, 69)

        sect = UnpreparedElfSection("test", SHT_NOBITS)
        sect.set_size(37)
        sect = sect.prepare(0, 0, 0, 32, '<')
        sect.set_size(12)
        self.assertEqual(sect.get_size(), 12)

    def test_get_flags(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.flags, 0)

        sect = UnpreparedElfSection("test", flags = 0xf)
        self.assertEqual(sect.flags, 0xf)

    def test_set_flags(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.flags, 0)
        sect.flags = 0xff
        self.assertEqual(sect.flags, 0xff)

    def test_get_link(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.link, None)

        sect2 = UnpreparedElfSection("test_link", link = sect)
        self.assertEqual(sect2.link, sect)

        sect3 = self.assertRaises(InvalidArgument, UnpreparedElfSection, "test_link", link = 80)

    def test_get_info(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.info, 0)
        sect = UnpreparedElfSection("test", info = 13)
        self.assertEqual(sect.info, 13)

    def test_set_info(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.info, 0)
        sect.info = 13
        self.assertEqual(sect.info, 13)
        
    def test_get_entsize(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.entsize, 0)
        sect = UnpreparedElfSection("test", entsize = 13)
        self.assertEqual(sect.entsize, 13)

    def test_set_entsize(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.entsize, 0)
        sect.entsize = 13
        self.assertEqual(sect.entsize, 13)

    def test_get_addralign(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.addralign, 0)
        sect = UnpreparedElfSection("test", addralign = 13)
        self.assertEqual(sect.addralign, 13)

    def test_set_addralign(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(sect.addralign, 0)
        sect.addralign = 13
        self.assertEqual(sect.addralign, 13)

    def test_get_offset(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(hasattr(sect, "get_offset"), False)
        sect = sect.prepare(15, 0, 0, 32, '<')
        self.assertEqual(sect.offset, 15)

    def test_get_index(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(hasattr(sect, "get_index"), False)
        sect = sect.prepare(0, 15, 0, 32, '<')
        self.assertEqual(sect.index, 15)

    def test_get_file_data(self):
        data = ByteArray("pants")
        sect = UnpreparedElfSection("test", data = data)
        self.assertEquals(sect.get_file_data(), data)
        sect = UnpreparedElfSection("test", section_type = SHT_NOBITS)
        sect.set_size(10)
        self.assertEquals(sect.get_file_data(), ByteArray())

    def test_get_data(self):
        data = ByteArray("pants")
        sect = UnpreparedElfSection("test", data = data)
        self.assertEquals(sect.get_data(), data)
        sect = UnpreparedElfSection("test", section_type = SHT_NOBITS)
        sect.set_size(10)
        self.assertEquals(sect.get_data(), ByteArray('\0' * 10))

    def test_data_append(self):
        data = ByteArray("pants")
        sect = UnpreparedElfSection("test", data = data)
        self.assertEquals(sect.get_data(), data)
        sect.data_append(ByteArray("foo"))
        self.assertEquals(sect.get_data(), ByteArray("pantsfoo"))
        sect = sect.prepare(0, 0, 0, 32, '<')
        self.assertEqual(hasattr(sect, "data_append"), False)

    def test_get_section_header(self):
        sect = UnpreparedElfSection("test")
        self.assertEqual(hasattr(sect, "get_section_header"), False)
        sect = sect.prepare(0, 0, 0, 32, '<')

        sh = sect.get_section_header()
        self.assertEquals(sh.todata(), Elf32SectionHeader('<').todata())

        sect = UnpreparedElfSection("test")
        sect2 = UnpreparedElfSection("test")
        sect.link = sect2
        sect2 = sect2.prepare(0, 2, 0, 32, '<')
        sect = sect.prepare(0, 0, 0, 32, '<')
        sh = sect.get_section_header()
        self.assertEquals(sh.sh_link, 2)

    def test_get_num_entries(self):
        sect = UnpreparedElfSection("test", entsize=4)
        sect.data_append(ByteArray("12345678"))
        self.assertEquals(sect.get_num_entries(), 2)

    def test_get_entry(self):
        sect = UnpreparedElfSection("test", entsize=4)
        sect.data_append(ByteArray("12345678"))
        self.assertEquals(sect.get_entry(0), ByteArray("1234"))
        self.assertEquals(sect.get_entry(1), ByteArray("5678"))

    def test_get_entries(self):
        sect = UnpreparedElfSection("test", entsize=4)
        sect.data_append(ByteArray("12345678"))
        self.assertEquals(list(sect.get_entries()), [ByteArray("1234"), ByteArray("5678")])

    def test_repr(self):
        sect = UnpreparedElfSection("test")
        self.assertEquals(repr(sect), "<UnpreparedElfSection NULL test>")
        

class TestElfFile(unittest.TestCase):
    def test__addr_type_is_virt(self):
        self.assertEquals(BaseElfFile._addr_type_is_virt("virtual"), True)
        self.assertEquals(BaseElfFile._addr_type_is_virt("physical"), False)
        self.assertRaises(InvalidArgument, BaseElfFile._addr_type_is_virt, None)
        self.assertRaises(InvalidArgument, BaseElfFile._addr_type_is_virt, 37)
        self.assertRaises(InvalidArgument, BaseElfFile._addr_type_is_virt, "asdf")

    def test_null_elf(self):
        for endianess, wordsize, expectedsize in \
            [ ('<', 32, 52), ('>', 32, 52),
              ('<', 64, 64), ('>', 64, 64) ]:
            ef = UnpreparedElfFile()
            self.assertEqual(len(ef.sections), 1)
            self.assertEqual(ef.has_segments(), False)
            ef = ef.prepare(wordsize, endianess)
            f = File("test.elf", "wb")
            data = ef.todata()
            for offset, _dat in data:
                f.seek(offset)
                _dat.tofile(f)
            f.flush()
            self.assertEqual(f.size(), expectedsize)
            f.close()
            os.remove("test.elf")

    def test_add_section(self):
        ef = UnpreparedElfFile()
        sect = UnpreparedElfSection()
        ef.add_section(sect)
        self.assertEqual(ef.sections[-1], sect)

        seg = HeaderElfSegment()
        self.assertRaises(InvalidArgument, seg.add_section, sect)

        seg = DataElfSegment()
        self.assertRaises(InvalidArgument, seg.add_section, sect)

    def test_remove_section(self):
        ef = UnpreparedElfFile()
        sect = UnpreparedElfSection()
        self.assertRaises(InvalidArgument, ef.remove_section, sect)
        ef.add_section(sect)
        self.assertEqual(sect in ef.sections, True)
        ef.remove_section(sect)
        self.assertEqual(sect in ef.sections, False)

        seg = SectionedElfSegment()
        ef.add_segment(seg)
        ef.add_section(sect)
        seg.add_section(sect)
        self.assertEqual(sect in ef.sections, True)
        self.assertEqual(sect in seg.sections, True)  
        ef.remove_section(sect)
        self.assertEqual(sect in ef.sections, False)
        self.assertEqual(sect in seg.sections, False)  

    def test_remove_nobits(self):
        for file_name in ["data/null_elf",
                          "data/arm_exec",
                          "data/arm_exec_nosect",
                          "data/arm_object",
                          "data/mips64_exec",
                          "data/mips32_exec",
                          "data/ia32_exec",
                          "data/amd64_exec",
                          "data/ia64_exec",
                          ]:
            ef = PreparedElfFile(filename=file_name)
            if ef.segments:
                seg_sizes = []
                for segment in ef.segments:
                    seg_sizes.append((segment.get_memsz(), segment.get_filesz()))
                ef.remove_nobits()
                count = 0
                for segment in ef.segments:
                    self.assertEqual(segment.get_memsz(), segment.get_filesz())
                    self.assertEqual(segment.get_memsz(), seg_sizes[count][0])
                    count += 1

        ef = UnpreparedElfFile()
        sec = UnpreparedElfSection(section_type = SHT_NOBITS, data = 10)
        self.assertEqual(sec.get_size(), 10)
        ef.add_section(sec)
        ef.remove_nobits()
        sec = ef.sections[1]
        self.assertEqual(sec.type, SHT_PROGBITS)
        self.assertEqual(sec.get_size(), 10)
        self.assertEqual(sec.get_file_data(), ByteArray('\0' * 10))

    def test_add_segment(self):
        ef = UnpreparedElfFile()
        seg = DataElfSegment(data=ByteArray("pants"))
        ef.add_segment(seg)
        self.assertEqual(ef.segments[-1], seg)

    """
    def test_malformed_stringtable(self):
        ef = ElfFile()
        self.assertRaises(elf.core.ElfFormatError, ef.init_and_prepare_from_filename,
                          "data/malformed_stringtable")
    """

    def test_prepare(self):
        ef = UnpreparedElfFile()
        segment = SectionedElfSegment(align=0x1)
        new_sect = UnpreparedElfSection("pants")
        ef.add_section(new_sect)
        segment.add_section(new_sect)
        ef.add_segment(segment)
        ef.add_segment(HeaderElfSegment())
        ef = ef.prepare(32, '<')

    def test_round_trip(self):
        for file_name in ["data/null_elf",
                          "data/arm_exec",
                          "data/arm_exec_nosect",
                          "data/arm_object",
                          "data/arm_scatter_load",
                          "data/mips64_exec",
                          "data/mips32_exec",
                          "data/ia32_exec",
                          "data/amd64_exec",
                          "data/ia64_exec",
                          ]:
            ef = PreparedElfFile(filename=file_name)
            ef.to_filename("elf.tmp")
            # FIXME
            # self.assertEqual(open("elf.tmp", "rb").read(), open(file_name, "rb").read(), "%s: failed to read back correctly" % file_name)
            os.remove("elf.tmp")

    def test_long_round_trip(self):
        for file_name in ["data/null_elf",
                          "data/arm_exec",
                          "data/arm_exec_nosect",
                          "data/arm_object",
                          "data/arm_scatter_load",
                          "data/mips64_exec",
                          "data/mips32_exec",
                          "data/ia32_exec",
                          "data/amd64_exec",
                          "data/ia64_exec",
                          ]:
            ef = UnpreparedElfFile(filename=file_name)
            ef = ef.prepare(ef.wordsize, ef.endianess)
            ef.to_filename("elf.tmp")
            # FIXME: We can't be sure that the files produced will be byte for
            # byte equal at this point. We need to come up with a test for
            # equivalance independant of things such as section ordering.
            # self.assertEqual(open("elf.tmp", "rb").read(), open(file_name, "rb").read(), "%s: failed to read back correctly" % file_name)
            os.remove("elf.tmp")



    def test_from_file(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            # self.assertEqual(len(ef.sections), elf_address[fn][4])
            self.assertEqual(len(ef.segments), elf_address[fn][5])

    def test_vtop(self):
        ef = PreparedElfFile(filename="data/arm_exec")
        seg = ef.find_segment_by_vaddr(0xf0001000L)
        self.assertEqual(seg.vtop(0xf0001000L), 0xa0101000L)

        self.assertRaises(InvalidArgument, seg.vtop, 0xa0001000L)

    def test_find_segment_by_vaddr(self):
        ef = PreparedElfFile(filename="data/arm_exec")
        seg = ef.find_segment_by_vaddr(0xf0001000L)
        self.assertEqual(0xf0001000L in seg.get_span(), True)

        seg = ef.find_segment_by_vaddr(0xa0001000L)
        self.assertEqual(seg, None)

    def test_contains_vaddr(self):
        ef = PreparedElfFile(filename="data/arm_exec")
        seg = ef.find_segment_by_vaddr(0xf0001000L)
        self.assertEqual(seg.contains_vaddr(0xf0001000L), True)

    def test_find_section_named(self):
        ef = PreparedElfFile(filename="data/arm_exec")
        sect = ef.find_section_named(".kip")
        self.assertEqual(sect.name, ".kip")

        sect = ef.find_section_named("notexist")
        self.assertEqual(sect, None)

    def test_get_first_addr(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            self.assertEqual(ef.get_first_addr(), elf_address[fn][2])


    def test_get_last_addr(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            self.assertEqual(ef.get_last_addr(), elf_address[fn][0],
                             "%s 0x%x != 0x%x" % (fn, ef.get_last_addr(),
                                                  elf_address[fn][0]))

    def test_get_address_span(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            self.assertEqual(ef.get_address_span(), elf_address[fn][0] - elf_address[fn][2],
                             "%s 0x%x != 0x%x" % (fn, ef.get_address_span(),
                                                  elf_address[fn][0] - elf_address[fn][2]))

    def test_get_first_paddr(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            if elf_address[fn][3] is None:
                self.assertRaises(InvalidArgument, ef.get_first_addr, "physical")
            else:
                self.assertEqual(ef.get_first_addr("physical"), elf_address[fn][3])

    def test_get_last_paddr(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            if elf_address[fn][1] is None:
                self.assertRaises(InvalidArgument, ef.get_last_addr, "physical")
            else:
                self.assertEqual(ef.get_last_addr("physical"), elf_address[fn][1],
                                 "%s 0x%x != 0x%x" % (fn, ef.get_last_addr("physical"),
                                                      elf_address[fn][1]))

    def test_get_address_span_phys(self):
        for fn in elf_address:
            ef = PreparedElfFile(filename=fn)
            if elf_address[fn][1] is None:
                self.assertRaises(InvalidArgument, ef.get_last_addr, "physical")
            else:
                self.assertEqual(ef.get_address_span("physical"), elf_address[fn][1] - elf_address[fn][3],
                                 "%s 0x%x != 0x%x" % (fn, ef.get_address_span(),
                                                      elf_address[fn][1] - elf_address[fn][3]))

    def test_set_ph_offset(self):
        ef = UnpreparedElfFile()
        ef.set_ph_offset(100, True)
        self.assertEqual(ef._ph_offset, 100)
        self.assertRaises(InvalidArgument, ef.set_ph_offset, 1, True)
        ef = ef.prepare(32, "<")
        self.assertEqual(hasattr(ef, "set_ph_offset"), False)



    def test_et_ph_offset(self):
        ef = UnpreparedElfFile()
        ef.set_ph_offset(100, True)
        self.assertEqual(ef.get_ph_offset(), 100)
        ef = ef.prepare(32, "<")
        self.assertEqual(ef.get_ph_offset(), 100)

    def test_find_symbol(self):
        ef = PreparedElfFile(filename="data/arm_exec")
        self.assertEquals(ef.find_symbol("notexistant"), None)
        sym = ef.find_symbol("do_ipc_copy")
        self.assertEqual(sym.name, "do_ipc_copy")

        sym = ef.find_symbol("_end_kip")
        self.assertEqual(sym.name, "_end_kip")

    def test_get_paddr(self):
        ef = PreparedElfFile(filename="data/arm_exec")
        self.assertEqual(ef.get_paddr(), 0xa0100000)
        ef = PreparedElfFile(filename="data/arm_object")
        self.assertRaises(InvalidArgument, ef.get_paddr)

class TestStringTable(unittest.TestCase):
    def test_getstringidx_out_of_bounds(self):
        st = UnpreparedElfStringTable()
        self.assertEqual(st.get_string_idx(1), None)

    def test_nulltable(self):
        st = UnpreparedElfStringTable()
        self.assertEqual(st.get_string_idx(0), "")
        self.assertEqual(st.index_of(""), 0)
        self.assertEqual(st.index_of("anything"), None)
        self.assertEqual(st.offset_of(""), 0)
        self.assertEqual(st.offset_of("anything"), None)
        self.assertEqual(st.get_file_data(), array.array('B', '\x00'))

    def test_simpletable(self):
        st = UnpreparedElfStringTable()
        self.assertEqual(st.add_string("name."), 1)
        self.assertEqual(st.add_string("Variable"), 7)
        self.assertEqual(st.add_string("able"), 16)
        self.assertEqual(st.add_string("xx"), 21)
        self.assertEqual(len(st.get_strings()), 5)
        self.assertEqual(st.get_string_idx(1), "name.")
        self.assertEqual(st.get_string_idx(2), "Variable")
        self.assertEqual(st.get_string_idx(3), "able")
        self.assertEqual(st.get_string_idx(4), "xx")
        self.assertEqual(st.get_string_ofs(0), "")
        self.assertEqual(st.get_string_ofs(1), "name.")
        self.assertEqual(st.get_string_ofs(7), "Variable")
        self.assertEqual(st.get_string_ofs(11), "able")
        self.assertEqual(st.get_string_ofs(16), "able")                                
        self.assertEqual(st.get_file_data(), array.array('B', '\0name.\0Variable\0able\0xx\0'))

    def test_fromdata(self):
        data = ByteArray('\0name.\0Variable\0able\0xx\0')
        st = UnpreparedElfStringTable(data = data)
        st = st.prepare(0, 0, 0, 32, '<')
        self.assertEqual(len(st.get_strings()), 5)
        self.assertEqual(st.get_string_idx(1), "name.")
        self.assertEqual(st.get_string_idx(2), "Variable")
        self.assertEqual(st.get_string_idx(3), "able")
        self.assertEqual(st.get_string_idx(4), "xx")
        self.assertEqual(st.get_string_ofs(0), "")
        self.assertEqual(st.get_string_ofs(1), "name.")
        self.assertEqual(st.get_string_ofs(7), "Variable")
        self.assertEqual(st.get_string_ofs(11), "able")
        self.assertEqual(st.get_string_ofs(16), "able")                                
        self.assertEqual(st.get_file_data(), data)

class TestNote(unittest.TestCase):
    """Test the NOTE elf section."""

    pass
