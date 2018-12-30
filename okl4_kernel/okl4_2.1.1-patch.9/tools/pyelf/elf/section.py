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
#    pylint: disable-msg=R0902,R0903,R0904,R0913
"""
ELF section types
"""
__revision__ = 1.0

from elf.ByteArray import ByteArray
from elf.constants import SHT_NULL, SHT_STRTAB, SHT_NOBITS, SHT_SYMTAB, \
     SHT_PROGBITS, STT_NOTYPE, STT_OBJECT, STT_FUNC, STT_SECTION, SHT_NOTE
from elf.structures import InvalidArgument, ELF_SH_CLASSES, \
     ELF_SYMBOL_STRUCT_CLASSES, is_integer
from elf.util import align_up

import struct

class BaseElfSection(object):
    """ElfSection instances represent sections in an ELF file. Note that
    this differs from ElfSectionheader, which just describes the header,
    not the data itself.

    ElfSection objects can exist in two states: prepared and unprepared.
    The subclasses UnpreparedElfSection and PreparedElfSection represent
    these two states.

    In the prepared state the section is ready to be written out to a file,
    but can not be modified (unless the changes maintain certain invariants).

    In the unprepared state the section can be modified, but must be
    prepared for writing before it can actually be written out.

    A section can be intialised from into either the prepared or
    unprepared state. Once in the prepared state, it is not possible to
    revert to the unprepared state.

    An unprepared section has a method .prepare which will return a prepared
    version of the section.

    Two important properties of the section, the wordsize and endianess,
    are only available in the prepared state.

    This class is written with the expectation that it will be sub-classed.
    To create a subclass (e.g. to implement a particular type of section) you
    need to create subclasses following the inheritance pattern below (this
    pattern is a little complicated, but would look much saner in 3D).

                           BaseElfSection
                                 |
             |-------------------------------------------|
             |                      |                    |
    UnpreparedElfSection     PreparedElfSection       BaseSubClass
             |                      |                 |  |
             |                      |-----------------|  |
             |                      |                    |
             |              PreparedSubClass             |
             |                                           |
             |-------------------------------------------|
             |
     UnpreparedSubClass

     class BaseSubClass(BaseElfSection): pass
     class UnpreparedSubClass(UnpreparedElfSection, BaseSubClass): pass
     class PreparedSubClass(PreparedElfSection, BaseSubClass): pass

     In this example, BaseSubClass would provide any class specific methods
     and also override any methods of BaseElfSection. When overriding methods
     of BaseElfSection, it is important to pass these overrides to subclasses.

     For example, if BaseSubClass defines get_size() (overriding
     BaseElfSection.get_size), then each of its subclasses would require a line

     get_size = BaseSubClass.get_size

     This is because python resolves multiple inheritance depth first, left to
     right, so BaseElfSection.get_size would be the first method resolved unless
     the explicit override is given.                
    """

    def __init__(self, name=None, section_type=SHT_NULL, address=0, data=None,
                 flags=0, addralign=0, entsize=0, info=0, link=None, symbols=None):
        self.address = address
        self.type = section_type
        self._name = name
        self.flags = flags
        self.addralign = addralign
        self.entsize = entsize
        self.info = info
        self.link = link
        self._backlinks = []
        self._in_segment_offset = None
        self._in_segment = False

        if symbols is None:
            self.symbols = []
        else:
            self.symbols = symbols
            for sym in self.symbols:
                sym.section = self

        # If the section is a 'NOBITS' section, then it just has size, and
        # no real data. So _data, may be an integer or a ByteArray depending
        # on the different circumstance
        if data is None:
            if section_type == SHT_NOBITS:
                self._data = 0
            else:
                self._data = ByteArray()
        else:
            self._data = data
        if section_type == SHT_NOBITS:
            assert is_integer(self._data)
        else:
            assert self._data.__class__ == ByteArray

    def copy(self):
        raise NotImplementedError

    def __repr__(self):
        return "<%s %s %s>" % (self.__class__.__name__, self.type, self.name)

    def get_size(self):
        """Return the size of the section in bytes."""
        if self.type == SHT_NOBITS:
            return self._data
        else:
            return len(self._data)

    def get_name(self):
        """Return the name of this segment."""
        return self._name
    name = property(get_name)

    def set_link(self, link):
        """Set a related section."""
        if link is not None:
            if not issubclass(link.__class__, BaseElfSection):
                raise InvalidArgument, "Link must be an ElfSection. %s" % str(link.__class__)
            link._backlinks.append(self)
        self._link = link

    def get_link(self):
        """Return the related ElfSection."""
        return self._link
    link = property(get_link, set_link)

    def get_data(self):
        """Return the data the will exist in memory. This may differ
        from what is copied into the file itself."""
        if self.type == SHT_NOBITS:
            return ByteArray('\0' * self.get_size())
        else:
            return self._data

    def get_file_data(self):
        """Return the data that should be copied into the file. The
        data is returned as a byte array."""
        if self.type == SHT_NOBITS:
            return ByteArray()
        else:
            return self._data

    def remove_nobits(self):
        """If this section is nobits, change it to PROGBITS."""
        if self.type == SHT_NOBITS:
            # Note: Order is important here. self.get_size()
            # is implemented differently depending on NOBITS vs PROGBITS
            self._data = ByteArray('\0' * self.get_size())
            self.type = SHT_PROGBITS

    def get_num_entries(self):
        """Return number of entries in the section."""
        return self.get_size() / self.entsize

    def get_entry(self, index):
        """Return the data for a given entry"""
        size = self.entsize
        return self._data[index*size:(index+1)*size]

    def get_entries(self):
        """Return a list of all entries."""
        for i in range(self.get_num_entries()):
            yield self.get_entry(i)

    def place_in_segment(self):
        """Indicate that the section has been placed in a segment."""
        self._in_segment = True

    def is_in_segment(self):
        """Return whether or not the section is in a segment."""
        return self._in_segment

    def get_in_segment_offset(self):
        """Return the offset into the original segment in which the
        section appeared.  Returns None if the section is new."""
        return self._in_segment_offset

    def set_in_segment_offset(self, offset):
        """FIXME: Malcolm what does this do?"""
        if self._in_segment_offset is None:
            self._in_segment_offset = offset
        else:
            self._in_segment_offset = None

    def calc_in_segment_offset(self, segment):
        """Calculate the offset of the segment.
        FIXME: Malcolm to provide better documentation."""
        off = self.get_in_segment_offset()
        if off is None or self.name.startswith('kernel_') or \
                self.name.startswith('ER_'):
            return self.address - segment.vaddr

        return off

    def data_trim(self, frm, to):
        """Trim the data so it is just [frm:to]"""
        self._data = self._data[frm:to]


class PreparedElfSection(BaseElfSection):

    def __init__(self, index, endianess, wordsize, name_offset=None,
                 offset=None, address=0, section_type=SHT_NULL, name=None,
                 data=None, flags=0, addralign=0, entsize=0, info=0,
                 link=None, symbols=None):
        BaseElfSection.__init__(self, name, section_type, address, data,
                 flags, addralign, entsize, info, link, symbols)

        self._index = index
        self._endianess = endianess
        self._wordsize = wordsize
        self.name_offset = name_offset
        self._offset = offset

    def copy(self):
        """Return a copy of the elf section, so for example it can be
        copied into a new elf file. Note: This will still hold
        a pointer to the old link section. You must manually update the
        link if required."""
        # Make a copy of the data
        if self.type == SHT_NOBITS:
            data = self._data
        else:
            data = self.get_file_data().copy()
            assert data.__class__ == ByteArray
        new_section = self.__class__(self.index, self.endianess,
                                     self.wordsize, self.name_offset,
                                     self.offset, self.address,
                                     self.type, self.name, 
                                     data, self.flags,
                                     self.addralign, self.entsize, 
                                     self.info, self.link)
        new_section._in_segment_offset = self._in_segment_offset

        return new_section

    def get_index(self):
        """Return the index of the file, that is,its position in the array
        of sections."""
        return self._index
    index = property(get_index)

    def get_offset(self):
        """Return the offset of this section in the file."""
        return self._offset
    offset = property(get_offset)

    def get_wordsize(self):
        return self._wordsize
    wordsize = property(get_wordsize)

    def get_endianess(self):
        return self._endianess
    endianess = property(get_endianess)

    def get_section_header(self):
        """Return an Elf section header structure for a given
        endianess and wordsize."""
        
        sh = ELF_SH_CLASSES[self.wordsize](self.endianess)
        sh.sh_name = self.name_offset        
        sh.sh_type = self.type
        sh.sh_addr = self.address
        sh.sh_offset = self.offset
        sh.sh_size = self.get_size()
        sh.sh_entsize = self.entsize
        sh.sh_flags = self.flags
        sh.sh_info = self.info
        sh.sh_addralign = self.addralign
        sh._name = self._name
        sh._index = self.index
        if self.link:
            sh.sh_link = self.link.index
        else:
            sh.sh_link = 0

        return sh

    def set_size(self, size):
        """Set the size of this section. If this size is greater
        than the current size then it will be zero-filled. Size
        can not be extended on prepared files."""
        if size > self.get_size():
            raise InvalidArgument, "Cannot extend prepared section"            
        if self.type == SHT_NOBITS:
            assert is_integer(size)
            self._data = size
        else:
            self.data_trim(0, size)


class UnpreparedElfSection(BaseElfSection):

    prepares_to = PreparedElfSection

    def __init__(self, name=None, section_type=SHT_NULL, address=0, data=None,
                 flags=0, addralign=0, entsize=0, info=0, link=None, symbols=None):

        BaseElfSection.__init__(self, name, section_type, address, data, flags,
                                addralign, entsize, info, link, symbols)
        self.prepared_to = None


    def prepare(self, offset, index, name_offset, wordsize, endianess, **kwargs):
        # Make a copy of the data
        cls = self.__class__.prepares_to
        if self.type == SHT_NOBITS:
            data = self._data
        elif cls == PreparedElfSymbolTable:
            data = self._data.copy()
        else:
            data = self.get_file_data().copy()
            assert data.__class__ == ByteArray

        new_section = cls(index, endianess, wordsize, name_offset,
                          offset, self.address, self.type,
                          self.name, data, self.flags,
                          self.addralign, self.entsize, 
                          self.info, self.link, self.symbols, **kwargs)
        for sect in self._backlinks:
            if sect.link == self:
                sect.link = new_section
        self.prepared_to = new_section
        return new_section
    
    def copy(self):
        """Return a copy of the elf section, so for example it can be
        copied into a new elf file. Note: This will still hold
        a pointer to the old link section. You must manually update the
        link if required."""
        # Make a copy of the data
        if self.type == SHT_NOBITS:
            data = self._data
        else:
            data = self.get_file_data().copy()
            assert data.__class__ == ByteArray
        new_section = self.__class__(self.name, self.type,
                                     self.address, data,
                                     self.flags, self.addralign,
                                     self.entsize, self.info, 
                                     self.link)
        new_section.symbols = self.symbols
        for sym in new_section.symbols:
            sym.section = new_section
        new_section._in_segment_offset = self._in_segment_offset

        return new_section

    def data_append(self, data):
        """Add some data to the section. This is only possible when the
        section is unprepared."""
        self._data.extend(data)

    def set_name(self, name):
        """Set the name of this segment. This can only be called on
        unprepared Sections"""
        self._name = name
    name = property(BaseElfSection.get_name, set_name)
    
    def set_size(self, size):
        """Set the size of this section. If this size is greater
        than the current size then it will be zero-filled. Size
        can not be extended on prepared files."""
        if self.type == SHT_NOBITS:
            assert is_integer(size)
            self._data = size
        else:
            if size < len(self._data):
                self.data_trim(0, size)
            else:
                self.data_append((size - len(self._data))*[0])


class BaseElfStringTable(BaseElfSection):
    """Represents and ELF string table (Ref 1-16). This is a subclass of
    the ElfSection object and all those methods are available here."""

    def _init(self):
        self.offsets = {"": 0}
        self.string_dict = {0: ""}

        if self._data == ByteArray('\x00'):
            self.strings = ["\x00"]
        else:
            offsets = [0] + [i+1 for (i, c) in enumerate(self._data.tostring()) if c == "\x00"][:-1]
            self.strings = [x + '\x00' for x in
                            self._data.tostring().split('\x00')[:-1]]
            for offset, s in zip(offsets, self.strings):
                s = s[:-1]
                self.offsets[s] = offset
                self.string_dict[offset] = s

    def get_file_data(self):
        """Return an array of bytes representing the data."""
        data = ByteArray()
        for s in self.strings:
            data.extend(ByteArray(s))
        return data

    def get_string_idx(self, idx):
        """Get the idx-th string. Return None if there is
        no string at the offset."""
        if idx < len(self.strings):
            return self.strings[idx][:-1]
        return None

    def get_string_ofs(self, offset):
        """Return the string at a given offset. None if it there is no
        string."""
        s = self.string_dict.get(offset, None)
        if s is None:
            cur_off = 0
            for s in self.strings:
                if cur_off <= offset < cur_off + len(s):
                    return s[offset - cur_off:-1]
                cur_off += len(s)
        return s

    def get_strings(self):
        """Return a list of string in the string table."""
        return [x[:-1] for x in self.strings]

    def get_size(self):
        """Return the size of the string table in bytes"""
        return sum([len(s) for s in self.strings])


    def index_of(self, string):
        """Return the index of a given string if it exists in the string
        table. Return None if it doesn't exist."""
        try:
            return [s[:-1] for s in self.strings].index(string)
        except ValueError:
            return None

    def offset_of(self, string):
        """Return the offset of a given string if it exists in the string
        table. Return None if it doesn't exist."""
        return self.offsets.get(string, None)


class PreparedElfStringTable(PreparedElfSection, BaseElfStringTable):

    get_size = BaseElfStringTable.get_size
    get_file_data = BaseElfStringTable.get_file_data
    
    def __init__(self, index, endianess, wordsize, name_offset=None,
                 offset=None, address=0, section_type=SHT_STRTAB, name=None,
                 data=None, flags=0, addralign=1, entsize=0, info=0,
                 link=None, symbols=None):

        PreparedElfSection.__init__(self, index, endianess, wordsize, name_offset,
                                    offset, address, section_type, name, data,
                                    flags, addralign, entsize, info, link, symbols)
        self._init()


class UnpreparedElfStringTable(UnpreparedElfSection, BaseElfStringTable):

    prepares_to = PreparedElfStringTable

    def __init__(self, name=None,  section_type=SHT_STRTAB, address=0,
                 data=ByteArray('\x00'), flags=0, addralign=1, entsize=0,
                 info=0, link=None, symbols=None):
        UnpreparedElfSection.__init__(self, name, section_type, address, data, flags,
                                      addralign, entsize, info, link, symbols)
        self._init()

    get_size = BaseElfStringTable.get_size
    get_file_data = BaseElfStringTable.get_file_data

    def add_string(self, string):
        """Add a new string to the table. Return the data offset."""
        if not string:
            return 0
        if len(self.offsets) > 1:
            offset = self.offsets[self.strings[-1][:-1]] + len(self.strings[-1])
        else:
            offset = 1
        self.strings.append(string + '\x00')
        self.offsets[string] = offset
        self.string_dict[offset] = string
        return offset

    def copy(self):
        new = UnpreparedElfSection.copy(self)
        new.strings = self.strings[:]
        new.offsets = self.offsets.copy()
        new.string_dict = self.string_dict.copy()
        return new



class ElfSymbol(object):
    """Represents an ELF symbol"""

    type_strings = {STT_NOTYPE : "Unspecified",
                    STT_OBJECT : "Data",
                    STT_FUNC : "Code",
                    STT_SECTION : "Section",
                    }

    def __init__(self, name, section, sym_struct):
        self.name = name
        self.section = section
        self.value = sym_struct.st_value
        self.size = sym_struct.st_size
        self.type = sym_struct.st_type
        self.bind = sym_struct.st_bind
        self.other = sym_struct.st_other

    def get_type_string(self):
        """Return a string representing the symbol's type. Returns
        unknown if we don't know what the type value means."""
        return ElfSymbol.type_strings.get(self.type, "Unknown")

    def __repr__(self):
        """Return a representation of the ElfSymbol instance."""
        rep =  "<ElfSymbol (%s) %s @ 0x%x Size: %d " % \
            (self.get_type_string(), self.name, self.value, self.size)
        if self.section:
            rep += self.section.name
        return rep + ">"


class BaseElfSymbolTable(BaseElfSection):
    """Represents an ELF symbol table (Ref 1-17). This is a subclass of
    the BaseElfSection object and all those methods are available here.
    """
    
    def _init(self):
        # List of all the symbols held in the section.
        # Note: Not called 'symbols' as that is a list of symbols associated 
        # with the section, and is part of the superclass
        self.int_syms = []

    def get_size(self):
        """Return the size of the string table in bytes"""
        return len(self.int_syms) * self.entsize

    def get_file_data(self):
        """Return an array of bytes representing the data."""
        data = ByteArray()
        for sym in self.int_syms:
            data.extend(sym.todata())
        return data

    def unpack(self, wordsize, endianess):

        sym_cls = ELF_SYMBOL_STRUCT_CLASSES[wordsize]
        self.entsize = sym_cls.size()
        data = self.get_data()
        if not data:
            return
        for i in range(self.entsize, len(data), self.entsize):
            sym_struct = sym_cls(endianess)
            sym_struct.fromdata(data[i:i+sym_struct.size()])
            self.int_syms.append(sym_struct)
        self._data = None # Only unpack once

    def clear_table(self):
        self.int_syms = []
        self._data = ByteArray()

class PreparedElfSymbolTable(PreparedElfSection, BaseElfSymbolTable):

    def __init__(self, index, endianess, wordsize, name_offset=None,
                 offset=None, address=0, section_type=SHT_SYMTAB, name=None,
                 data=None, flags=0, addralign=0, entsize=0, info=0,
                 link=None, symbols=None, int_syms=None):
        PreparedElfSection.__init__(self, index, endianess, wordsize, name_offset, offset,
                                    address, section_type, name, data,
                                    flags, addralign, entsize, info, link, symbols)
        self._init()
        self.unpack(wordsize, endianess)
        if int_syms is not None:
            self.int_syms += int_syms

        # Start with a null symbol
        sym_cls = ELF_SYMBOL_STRUCT_CLASSES[wordsize]
        new_syms = [sym_cls(endianess)]
        for sym in self.int_syms:
            if sym.__class__ == sym_cls:
                new_syms.append(sym)
                continue

            symstruct = sym_cls(endianess)
            symstruct.st_size = sym.size
            symstruct.st_value = sym.value
            symstruct.st_other = sym.other
            symstruct.st_bind = sym.bind
            symstruct.st_type = sym.type
            if sym.section:
                symstruct.st_shndx = sym.section.index
            else:
                symstruct.st_shndx = 0
            index = self.link.offset_of(sym.name)
            symstruct.st_name = index
            new_syms.append(symstruct)

        self.int_syms = new_syms

    get_size = BaseElfSymbolTable.get_size
    get_file_data = BaseElfSymbolTable.get_file_data

    def get_symbol_structs(self):
        """Return a list of symbol structures. This is only available when the
        section is prepared."""
        return self.int_syms


class UnpreparedElfSymbolTable(UnpreparedElfSection, BaseElfSymbolTable):

    prepares_to = PreparedElfSymbolTable

    def __init__(self, name=None, section_type=SHT_SYMTAB, address=0, data=None,
                 flags=0, addralign=0, entsize=0, info=0, link=None, symbols=None):
        UnpreparedElfSection.__init__(self, name, section_type, address, data,
                                      flags, addralign, entsize, info, link, symbols)
        self._init()

    get_size = BaseElfSymbolTable.get_size
    get_file_data = BaseElfSymbolTable.get_file_data

    def add_symbol(self, symbol):
        """Add a new symbol to the section. Only possible when the
        section is unprepared."""
        self.int_syms.append(symbol)

    def prepare(self, offset, index, name_offset, wordsize, endianess):
        """Prepare this ElfSection by setting its offset into
        the file, its index and name offset."""
        return UnpreparedElfSection.prepare(self, offset, index, name_offset,
                                            wordsize, endianess,
                                            int_syms=self.int_syms)

    def copy(self):
        new = UnpreparedElfSection.copy(self)
        new.int_syms = self.int_syms[:]
        return new

class BaseElfNote(BaseElfSection):
    """Represents an ELF note section (Ref 2-4). This is a subclass of
    the BaseElfSection object and all those methods are available here.
    """

    bytes_per_word = 4 # This is defined in Ref 2-4

    def _init(self):
        self.note_name  = None
        self.note_type  = None
        self._desc_data  = None
        
    def get_desc_data(self):
        """Return the note descriptpr data."""
        return self._desc_data                                                                                            

    def unpack(self, endianess):
        
        self._endianess = endianess
        self._format_chr =  "I"
            
        if len(self._data) == 0:
            return
            
        name_start = self.bytes_per_word * 3
        namesz, descsz, _type = \
                struct.unpack(self._endianess + self._format_chr * 3,
                              self._data[:name_start])

        name = struct.unpack(self._endianess + str(namesz) + 's',
                             self._data[name_start:name_start + namesz])
        
        # Remove the terminating 0.
        self.note_name = name[0].split('\x00')[:-1][0]
        self.note_type = _type
        
        desc_start = name_start + align_up(namesz, self.bytes_per_word)
        
        self._desc_data = self._data[desc_start: desc_start + descsz]

    def get_file_data(self):
        """
        Return the file data. This overloads the parent class
        implementation.
        """
        self._update_data()
        return BaseElfSection.get_file_data(self)
    
    def _update_data(self):
        """Update the note data based on our changes."""
        
        if not (self.note_name is None or self.note_type is None or \
                self._desc_data is None):
            data = ByteArray(struct.pack(self._endianess + self._format_chr * 3,
                                         len(self.note_name) + 1, len(self._desc_data),
                                         self.note_type))
            
            data += ByteArray(struct.pack(self._endianess +
                                          str(align_up(len(self.note_name) + 1,
                                                       self.bytes_per_word)) + 's',
                                          self.note_name))
            
            data += self._desc_data
            self._data = data
                                                                                                                                

class PreparedElfNote(PreparedElfSection, BaseElfNote):

    def __init__(self, index, endianess, wordsize, name_offset=None,
                 offset=None, address=0, section_type=SHT_NOTE, name=None,
                 data=None, flags=0, addralign=0, entsize=0, info=0,
                 link=None, symbols=None):
        PreparedElfSection.__init__(self, index, endianess, wordsize, name_offset, offset,
                                    address, section_type, name, data,
                                    flags, addralign, entsize, info, link, symbols)
        self._init()
        self.unpack(endianess)

    get_file_data = BaseElfNote.get_file_data


class UnpreparedElfNote(UnpreparedElfSection, BaseElfNote):

    prepares_to = PreparedElfNote

    def __init__(self, name=None, section_type=SHT_NOTE, address=0,
                 data=ByteArray('\x00'), flags=0, addralign=0, entsize=0,
                 info=0, link=None, symbols=None):

        UnpreparedElfSection.__init__(self, name, section_type, address, data,
                                      flags, addralign, entsize, info, link, symbols)
        self._init()
        
    get_file_data = BaseElfNote.get_file_data

    def copy(self):
        new = UnpreparedElfSection.copy(self)
        new.note_name = self.note_name
        new.note_type = self.note_type
        new._desc_data = self._desc_data
        return new



ELF_SECTION_TYPES = {
    SHT_STRTAB : UnpreparedElfStringTable,
    SHT_SYMTAB : UnpreparedElfSymbolTable,
    SHT_NOTE : UnpreparedElfNote
    }

