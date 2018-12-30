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

"""Implements a command for displaying an ELF file in a format very
similar to readelf. It also is able to print out l4 data structures
such as the Kip and Bootinfo."""

from optparse import OptionParser
from elf.core import PreparedElfFile
from weaver.kernel_elf import find_kernel_config
from weaver.bootinfo_elf import find_bootinfo
import sys

def print_pheaders(elf, summary=True, fout=sys.stdout):
    """Print out the program headers of a given elf file. If summary is
    true, then provide a summary of the Elf file."""
    segments = elf.segments
    if summary:
        if not elf.has_segments():
            print >> fout
            print >> fout,  "There are no program headers in this file."
        else:
            print >> fout
            print >> fout,  "Elf file type is %s" % elf.elf_type
            print >> fout,  "Entry point 0x%x" % elf.entry_point
            print >> fout,  "There are %d program headers, starting at offset %s" % \
                  (len(segments), elf._ph_offset)
    if not elf.has_segments():
        return
    print >> fout
    print >> fout,  "Program Headers:"
    if elf.wordsize == 64:
        print >> fout,  "  Type           Offset             VirtAddr           PhysAddr\n" +  \
        "                 FileSiz            MemSiz              Flags  Align"
    else:
        print >> fout,  "  Type           Offset   VirtAddr   PhysAddr   " \
              "FileSiz MemSiz  Flg Align"

    for segment in segments:
        segment.get_program_header(elf.endianess, elf.wordsize).output(fout)
    if elf.sections:
        print >> fout
        print >> fout,  " Section to Segment mapping:"
        print >> fout,  "  Segment Sections..."
        for i, segment in enumerate(segments):
            print >> fout,  "   %02d    " % i,
            if segment.has_sections():
                for sect in segment.sections:
                    if sect.get_size():
                        print >> fout,  sect.name,
            print >> fout,  ""


def print_sheaders(elf, summary=True, fout=sys.stdout):
    """Print out the section headers of a given elf file. If summary is
    true, then provide a summary of the Elf file."""
    sections = elf.sections
    if summary:
        if sections is None or sections == []:
            print >> fout
            print >> fout, "There are no sections in this file."
        else:
            print >> fout, "There are %d section headers, starting at offset 0x%x:" % \
                  (len(sections), elf._sh_offset)
    if sections is None or sections == []:
        return
    hdrs = elf.get_section_headers()
    print >> fout
    print >> fout, "Section Headers:"
    if elf.wordsize == 64:
        print >> fout, "  [Nr] Name              Type             Address           Offset\n" \
                      "       Size              EntSize          Flags  Link  Info  Align"
    else:
        print >> fout, "  [Nr] Name              Type            Addr" \
              "     Off    Size   ES Flg Lk Inf Al"
    for sect in hdrs:
        sect.output(fout)

    print >> fout, "Key to Flags:"
    print >> fout, "  W (write), A (alloc), X (execute), M (merge), S (strings)"
    print >> fout, "  I (info), L (link order), G (group), x (unknown)"
    print >> fout, "  O (extra OS processing required) o (OS specific), "\
          "p (processor specific)"


def print_cmd(args):
    """Display an ELF file. args contains command line arguments passed
    from sys.argv. These are parsed using option parser."""

    parser = OptionParser("%prog print [options] file", add_help_option=0)
    parser.add_option("-H", "--help", action="help")
    parser.add_option("-a", "--all", action="store_true", dest="all",
                      help="Print all information")
    parser.add_option("-h", "--header", action="store_true", dest="header",
                      help="Print ELF header")
    parser.add_option("-l", "--pheaders", action="store_true", dest="pheaders",
                      help="Print ELF sections headers")
    parser.add_option("-S", "--sheaders", action="store_true", dest="sheaders",
                      help="Print ELF sections headers")
    parser.add_option("-k", "--kconfig", action="store_true", dest="kconfig", 
                      help="Print L4 kernel config data structure (Default)")
    parser.add_option("-B", "--bootinfo", action="store_true", dest="bootinfo",
                      help="Print L4 Bootinfo")
    parser.add_option("-s", "--segnames", action="store_true", dest="segnames",
                      help="Print segment names")

    (options, args) = parser.parse_args(args)
    
    if len(args) != 1:
        parser.error("incorrect number of arguments")

    if options.all:
        options.header = options.pheaders = options.sheaders = \
                             options.kconfig = options.bootinfo = True

    elf = PreparedElfFile(filename=args[0])

    if options.header:
        elf.get_elf_header().output(sys.stdout)

    if options.sheaders:
        print_sheaders(elf, not options.header, sys.stdout)

    if options.pheaders:
        print_pheaders(elf, not options.header, sys.stdout)

    if options.kconfig:
        print
        kconfig = find_kernel_config(elf)
        if kconfig:
            kconfig.output(sys.stdout)
        else:
            print "There is no kernel configuration in this file."

    if options.bootinfo:
        print
        bootinfo = find_bootinfo(elf)
        if bootinfo:
            bootinfo.output(sys.stdout)
        else:
            print "There is no Bootinfo section in this file."

    if options.segnames:
        print
        segnames = get_segnames(elf)
        if segnames:
            for segname in segnames.strings:
                idx = segname.index('\x00')
                segname = segname[:idx]
                segname = segname.strip()
                if segname == "":
                    continue
                print segname
        else:
            # TODO - use the first section in each segment
            # as the segment name and print that.
            print "There is no .segnames section in this file"

    return 0


def get_segnames(elf):
    """Find the segnames data structructure in an ELF. Return none if it
    can't be found."""
    return elf.find_section_named(".segment_names")
