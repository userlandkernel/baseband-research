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

#pylint: disable-msg=R0902,R0904,R0913
# We disable the warning about a class having too many public methods
# because for this class we are sure it is a good thing. We also
# don't care about the classes that have too few methods. It would be
# nice to disable these per class.
# Also we don't really care about having too many instance attributes
"""
ELF parsing library.
"""
__revision__ = 1.0

from elf.ByteArray import ByteArray
from elf.constants import PT_NULL, PT_PHDR, SHT_NOBITS
from elf.structures import InvalidArgument, ELF_PH_CLASSES
from elf.util import Span, Unprepared

class ElfSegment(object):
    """ElfSegment instances represent segments in an ELF file. This
    differs from ElfProgramHeader, which just describes the header
    and not the data itself."""

    def __init__(self, program_header_type=PT_NULL, vaddr=0, paddr=0,
                 flags=0, align=0, ph=None):
        """Create a new ElfSegment, with a given set of initial parameters."""
        if ph:
            program_header_type = ph.p_type
            vaddr = ph.p_vaddr
            paddr = ph.p_paddr
            flags = ph.p_flags
            align = ph.p_align
        self.type = program_header_type
        self.vaddr = vaddr
        self.paddr = paddr
        self.flags = flags
        self.align = align

        self.prepared = False
        self._offset = None

        self._prog_header_size = None
        self._is_scatter_load = False


    def set_scatter_load(self):
        """Mark the segment as a scatter-load segment."""
        self._is_scatter_load = True

    def is_scatter_load(self):
        """Return whether or not the segment is a scatter-load one."""
        return self._is_scatter_load
        
    def set_data(self, data):
        """Set the data for this memory segment"""
        raise NotImplementedError

    def remove_nobits(self):
        """Remove any NOBITS sections from the segment."""
        raise NotImplementedError

    def prepare(self, offset, prog_header_size = None):
        """Prepare this segment ready for writing."""
        if self.prepared:
            raise InvalidArgument, "This segment is already prepared"
        if not prog_header_size and self.type == PT_PHDR:
            raise InvalidArgument, "Must set the program header size on PHDR "\
                  "segments when preparing"
        if prog_header_size and self.type != PT_PHDR:
            raise InvalidArgument, "Program header size should only " \
                  "be set on phdr sections."

        self._offset = offset
        self.prepared = True
        self._prog_header_size = prog_header_size

    def get_memsz(self):
        """Return the size this segment occupies in memory."""
        raise NotImplementedError

    def get_filesz(self):
        """Return the size this segment occupies in memory."""
        raise NotImplementedError

    def get_offset(self):
        """Return the offset of this segment is the physical file."""
        if self.prepared:
            return self._offset
        else:
            raise Unprepared, "Can only get the offset once the" \
                  " segment has been prepared."
    offset = property(get_offset)

    def get_file_data(self):
        """Return the data that will go into the file."""
        raise NotImplementedError
        
    def remove_section(self, section):
        """Remove a section from the segment."""
        raise NotImplementedError

    def has_sections(self):
        """Return true if the segment has internal sections."""
        raise NotImplementedError

    def get_sections(self):
        """Return a list of sections associated with this segment."""
        raise NotImplementedError

    def add_section(self, section):
        """Add a new section to a segment."""
        raise NotImplementedError
        
    def copy(self):
        """Return a copy of the Elf segment."""
        raise NotImplementedError

    def _do_copy(self, **kwargs):
        new_segment = self.__class__(self.type, self.vaddr, self.paddr,
                                     self.flags, self.align, **kwargs)
        new_segment._prog_header_size = self._prog_header_size
        new_segment._is_scatter_load = self._is_scatter_load
        new_segment._offset = self._offset
        new_segment.prepared = self.prepared
        return new_segment
    
    def contains_vaddr(self, vaddr):
        """Return true if this segment contains the given virtual address."""
        return vaddr in self.get_span()

    def get_span(self):
        """Return the span of memory that this segment holds."""
        return Span(self.vaddr, self.vaddr + self.get_memsz())

    def vtop(self, vaddr):
        """Convert a virtual address to a physical address"""
        if vaddr not in self.get_span():
            raise InvalidArgument, "Vaddr must be in segment's range"
        return self.paddr + (vaddr - self.vaddr)
        
    def get_program_header(self, endianess, wordsize):
        """Return a program header for this segment with a given endianess
        and wordsize."""

        if not self.prepared:
            raise Unprepared, "get_program_header can't be called if the " \
                  " segment is unprepared."

        try:
            ph = ELF_PH_CLASSES[wordsize](endianess)
        except KeyError:
            raise InvalidArgument, "wordsize %s is not valid. " \
                  "Only %s are valid" %  \
                  (wordsize, ELF_PH_CLASSES.keys())

        ph.p_type = self.type
        ph.p_vaddr = self.vaddr
        ph.p_paddr = self.paddr
        ph.p_flags = self.flags
        ph.p_align = self.align
        ph.p_offset = self.offset
        ph.p_memsz = self.get_memsz()
        ph.p_filesz = self.get_filesz()

        return ph

    def __str__(self):
        return "<ElfSegment %s>" % self.type


class SectionedElfSegment(ElfSegment):

    def __init__(self, program_header_type=PT_NULL, vaddr=0, paddr=0,
                 flags=0, align=0, ph=None, sections=None):
        """Create a new SectionedElfSegment, with a given set of initial parameters."""
        ElfSegment.__init__(self, program_header_type, vaddr, paddr, flags, align, ph)

        if sections is None:
            sections = []

        self.sections = sections
        # Keep the section list sorted by virtual address.  This
        # makes is easier to calculate file and memory size
        self.sections.sort(key=lambda x: x.address)
        
        # Mark the sections as added to the segment.
        for s in self.sections:
            s.place_in_segment()

    def copy(self):
        """Return a copy of the Elf segment."""

        for sect in self.sections:
            assert sect.link is None

        sections = [sect.copy() for sect in self.sections]
        return self._do_copy(sections=sections)

    def has_sections(self):
        """Return true if the segment has internal sections."""
        return True

    def set_data(self, data):
        """Set the data for this memory segment"""
        raise InvalidArgument, "Segment data can not be set when segment" \
              " already has sections."

    def get_sections(self):
        """Return a list of sections associated with this segment."""
        return self.sections

    def remove_section(self, section):
        """Remove a section from the segment."""
        if section not in self.sections:
            raise InvalidArgument, "Section must be in segment to remove it."
        self.sections.remove(section)

    def replace_section(self, old_section, new_section):
        if old_section not in self.sections:
            raise InvalidArgument, "Section must be in segment to replace it."
        self.sections[self.sections.index(old_section)] = new_section

    def get_file_data(self):
        """Return the data that will go into the file."""
        raise InvalidArgument, "You can't get file data on section file"

    def add_section(self, section):
        """Add a new section to a segment."""
        # Mark the section as added to the segment.
        # HACK:  Should detect if adding a section causes a segment to
        # turn into a scatter load segment.
        section.place_in_segment()
        self.sections.append(section)
        self.sections.sort(key=lambda x: x.address)

        # Set the in_segment offset to the right value.
        if section.get_in_segment_offset() is None:
            section.set_in_segment_offset(section.address - self.vaddr)

    def get_filesz(self):
        """Return the size this segment occupies in memory."""
        return self._calc_section_size()

    def _calc_section_size(self):
        """Calculate the size of the segment when it has sections."""
        # Use offsets if the segment is a scatter-load one,
        # otherwise use virtual addresses.  In either case, NOBITS
        # sections are not included in the calculations.
        sects = [sect for sect in self.sections if sect.type != SHT_NOBITS]
        return self._calc_size(sects)

    def get_memsz(self):
        """Return the size this segment occupies in memory."""
        # Use offsets if the segment is a scatter-load one,
        # otherwise use virtual addresses. 
        return self._calc_size(self.sections)

    def _calc_size(self, sects):
        if not sects:
            return 0
        if self.is_scatter_load():
            off, size = max([(sect.calc_in_segment_offset(self), sect.get_size()) for sect in sects])
        else:
            last_sect = sects[-1]
            off, size = last_sect.address - self.vaddr, last_sect.get_size()
        return off + size
        
    def remove_nobits(self):
        """Remove any NOBITS sections from the segment."""
        for section in self.sections:
            section.remove_nobits()


class DataElfSegment(ElfSegment):


    def __init__(self, program_header_type=PT_NULL, vaddr=0, paddr=0,
                 flags=0, align=0, ph=None, data = None):
        """Create a new DataElfSegment, with a given set of initial parameters."""

        ElfSegment.__init__(self, program_header_type, vaddr, paddr, flags, align, ph)
        self._data = data

    def copy(self):
        """Return a copy of the Elf segment."""
        # Make a copy of the data
        data = None
        if self._data is not None:
            data = self._data.copy()
        return self._do_copy(data=data)

    def has_sections(self):
        """Return true if the segment has internal sections."""
        return False

    def set_data(self, data):
        """Set the data for this memory segment"""
        self._data = data

    def get_sections(self):
        """Return a list of sections associated with this segment."""
        raise InvalidArgument, "This segment has no sections."

    def remove_section(self, section):
        """Remove a section from the segment."""
        raise InvalidArgument, "Section must be in segment to remove it."

    def get_file_data(self):
        """Return the data that will go into the file."""
        return self._data

    def add_section(self, section):
        """Add a new section to a segment."""
        raise InvalidArgument, "Can't set both data and sections"

    def get_memsz(self):
        """Return the size this segment occupies in memory."""
        try:
            return self._data.memsz
        except AttributeError:
            raise InvalidArgument, "Data not yet set"

    def get_filesz(self):
        """Return the size this segment occupies in memory."""
        try:
            return len(self._data)
        except TypeError:
            raise InvalidArgument, "Data not yet set"


    def remove_nobits(self):
        """Remove any NOBITS sections from the segment."""
        zeros = ByteArray('\0' * (self.get_memsz() - self.get_filesz()))
        self._data.extend(zeros)


class HeaderElfSegment(ElfSegment):

    def __init__(self, vaddr=0, paddr=0, flags=0, align=0, ph=None):
        """Create a new HeaderElfSegment, with a given set of initial parameters."""
        ElfSegment.__init__(self, PT_PHDR, vaddr, paddr, flags, align, ph)

    def copy(self):
        """Return a copy of the Elf segment."""
        return self._do_copy()

    def has_sections(self):
        """Return true if the segment has internal sections."""
        return False

    def get_sections(self):
        """Return a list of sections associated with this segment."""
        raise InvalidArgument, "This segment has no sections."

    def remove_section(self, section):
        """Remove a section from the segment."""
        raise InvalidArgument, "Section must be in segment to remove it."

    def add_section(self, section):
        """Add a new section to a segment."""
        raise InvalidArgument, "Can't set both data and sections"

    def get_memsz(self):
        """Return the size this segment occupies in memory."""
        if not self.prepared:
            raise Unprepared, "Phdr segments must be prepared before " \
                  "it is possible to get their size."
        return self._prog_header_size

    def get_filesz(self):
        """Return the size this segment occupies in memory."""
        if not self.prepared:
            raise Unprepared, "Phdr segments must be prepared before " \
                  "it is possible to get their size."
        return self._prog_header_size

    def remove_nobits(self):
        """Remove any NOBITS sections from the segment."""
        pass

