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
The modify subcommand of elfweaver.

This subcommand is designed to make changes to an existing ELF file to
accommodate the limitations of various boot-loaders.
"""

import optparse
from elf.core import UnpreparedElfFile

class UnknownField(Exception):
    """Exception thrown when an unknown field is encountered."""
    pass

def adjust(elf, field_designator, offset, absolute):
    if field_designator[0] == "segment":
        # We do something for every segment
        for segment in elf.segments:
            if field_designator[1] == "paddr":
                if absolute:
                    segment.paddr = offset
                else:
                    segment.paddr += offset
            else:
                print "don't know about field_designator %s" % field_designator[1]
    else:
        raise UnknownField, "Don't know about field %s" % field_designator[0]

def change(elf, field_designator, old, new):
    if field_designator[0] == "segment":
        for segment in elf.segments:
            if field_designator[1] == "paddr":
                if segment.paddr == old:
                    segment.paddr = new
    else:
        raise UnknownField, "Don't know about field %s" % field_designator[0]

def merge_sections(elf, name):
    print "Merging elf sections", name
    base_section = None
    for section in elf.sections[:]:
        if base_section:
            if section.name.startswith(name):
                base_section.data_append(section.get_data())
                elf.remove_section(section)
            else:
                break
        if not base_section and section.name.startswith(name):
            base_section = section
            base_section.name = name
            base_section.set_in_segment_offset(0)


def modify_cmd(args):
    """Merge command call from main. This parses command line
    arguments and calls merge, which does all the main work."""
    parser = optparse.OptionParser("%prog modify file [options]",
                                   add_help_option=0)

    parser.add_option("-H", "--help", action="help")
    parser.add_option("--adjust", nargs=2, dest="adjust",
                      action="append", default=[])
    parser.add_option("--physical", dest="physical",
                      action="store_true")
    parser.add_option("--physical_entry", dest="physical_entry",
                      action="store_true")
    parser.add_option("--change", dest="change",
                      action="append", nargs=2, default=[])
    parser.add_option("--merge_sections", dest="merge_sections",
                      action="append", default=[])
    parser.add_option("--remove_nobits", dest="remove_nobits",
                      action="store_true")
    parser.add_option("-o", "--output", dest="output", metavar="FILE",
                      help="Display the ELF file header")
    parser.add_option("--no-section-headers", dest="remove_section_headers",
                      action="store_true")


    (options, args) = parser.parse_args(args)

    if len(args) != 1:
        parser.error("Must supply a file to operate on.")

    filename = args[0]

    elf = UnpreparedElfFile(filename=filename)

    for (field_desc, offset) in options.adjust:
        absolute = True
        if offset.startswith("+") or offset.startswith("-"):
            absolute = False
        offset = long(offset, 0)
        field_desc = field_desc.split(".")
        adjust(elf, field_desc, offset, absolute)

    for (field_desc, mod) in options.change:
        field_desc = field_desc.split(".")
        (old, new) = [long(x, 0) for x in mod.split("=")]
        change(elf, field_desc, old, new)
            
    for section_name in options.merge_sections:
        merge_sections(elf, section_name)

    if options.physical_entry:
        entry = elf.entry_point
        for segment in elf.segments:
            if segment.contains_vaddr(entry):
                offset = segment.vaddr - segment.paddr
                elf.entry_point = entry - offset

    if options.physical:
        for segment in elf.segments:
            offset = segment.vaddr - segment.paddr
            segment.vaddr = segment.paddr
            if segment.has_sections():
                for section in segment.get_sections():
                    section.address -= offset

    if options.remove_nobits:
        elf.remove_nobits()

    if options.remove_section_headers:
        elf.remove_section_headers()

    elf = elf.prepare(elf.wordsize, elf.endianess)
    outfile = filename

    if options.output:
        outfile = options.output

    elf.to_filename(outfile)
