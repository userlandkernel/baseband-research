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

import unittest
import sys
from StringIO import StringIO
from elf.core import PreparedElfFile
import weaver.parse_spec

class TestCmd(unittest.TestCase):
    def _run_command(self, args):
        _old_stdout = sys.stdout
        _old_stderr = sys.stderr
        _old_exit = sys.exit
        self.exit_value = None

        _stdout = StringIO()
        _stderr = StringIO()        

        class ExitFinished(Exception):
            pass
        def _exit(status):
            if status is None:
                exit_code = 0
            elif type(status) != type(0):
                print status
                exit_code = 1
            else:
                exit_code = status
            self.exit_value = exit_code
            raise ExitFinished("Finished")

        sys.stdout = _stdout
        sys.stderr = _stderr
        sys.exit = _exit
        try:
            try:
                self.exit_value = self.cmd[0](args)
            except ExitFinished:
                pass
        finally:
            sys.stdout = _old_stdout
            sys.stderr = _old_stderr
            sys.exit = _old_exit
            #print _stdout.getvalue()
            #print _stderr.getvalue()
            
        if self.exit_value is None:
            self.exit_value = 0

        ret_stdout = _stdout.getvalue()
        ret_stderr = _stderr.getvalue()        
        _stdout.close()
        _stderr.close()        

        return self.exit_value, ret_stdout, ret_stderr
        
    def check_segments_and_sections(self, xml_filename, elf_filename):
        xml_segments, parsed_elf = self.check_segment_addr(xml_filename, elf_filename)
        exit_value = self.check_section_name(xml_segments, parsed_elf)
        return exit_value

    def check_segment_addr(self, xml_filename, elf_filename):
        parsed_elf = Pr
        eparedElfFile(filename=elf_filename)
        elf_paddrs = []
        for segment in parsed_elf.segments:
            elf_paddrs.append(segment.paddr)
        parsed_xml = weaver.parse_spec.parse_spec_xml(xml_filename)
        mergable_elements = [elm for elm in parsed_xml.children if elm.tag in ["kernel", "rootprogram", "program"]]
        segment_elements = {}
        for elm in mergable_elements:
            for elm_segchild in elm.find_children("segment"):
                if hasattr(elm, "name"):
                    segment_elements[elm_segchild] = elm.name
                else:
                    segment_elements[elm_segchild] = elm.tagname
        xml_paddrs = [elm.phys_addr for elm in segment_elements.keys()]
        xml_paddrs.sort()
        elf_paddrs.sort()
        if xml_paddrs == elf_paddrs:
            return segment_elements, parsed_elf
        xml_paddrs_copy = [] + xml_paddrs
        for x_paddr in xml_paddrs_copy:
            for e_paddr in elf_paddrs:
                if e_paddr == x_paddr:
                    elf_paddrs.remove(e_paddr)
                    xml_paddrs.remove(x_paddr)
                    break
        if (len(elf_paddrs) and (len(xml_paddrs) == 0)):
            raise weaver.merge.MergeError, "Too many segments in elf file"
        if (len(xml_paddrs) and (len(elf_paddrs) == 0)):
            raise weaver.merge.MergeError, "Missing segments in elf file"
        if (len(xml_paddrs) and len(elf_paddrs)):
            raise weaver.merge.MergeError, "Non matching segment physical address between elf and xml files"

        raise weaver.merge.MergeError, "Should not be here" 

    def check_section_name(self, xml_segment_elements, parsed_elf):
        elf_segment_elements = parsed_elf.segments
        elf_section_elements = parsed_elf.sections
        addr_to_name = {}
        for xml_segment, xml_segment_name in xml_segment_elements.iteritems():
            addr_to_name[xml_segment.phys_addr] = xml_segment_name
        for elf_segment in elf_segment_elements:
            if not elf_segment.paddr in addr_to_name:
                raise weaver.merge.MergeError, "Unknown segment in elf file"
            addr_to_name[elf_segment.vaddr] = addr_to_name.pop(elf_segment.paddr)
        for elf_section in elf_section_elements:
            if elf_section.get_address() in addr_to_name:
                section_name = elf_section.name
                if not section_name.startswith(addr_to_name[elf_section.get_address()]):
                    raise weaver.merge.MergeError, "Bad section name"
        return 0
