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

from StringIO import StringIO
import unittest
import parsers.gnu.gnu_parser
import parsers.gnu.gnu_ld_p
import parsers.gnu.gnu_ld_l
from antlr import RecognitionException

modules_under_test = [parsers.gnu.gnu_parser, parsers.gnu.gnu_ld_p, parsers.gnu.gnu_ld_l]

class TestParseLDScript(unittest.TestCase):
    def test_rubbish(self):
        f = StringIO("rubbish")
        self.assertRaises(RecognitionException, parsers.gnu.gnu_parser.extract_section_names, f)

    def test_segments(self):
        f = StringIO("PHDRS\n{\ntcm PT_LOAD;\n}\n")
        segments = parsers.gnu.gnu_parser.extract_segment_names(f)
        self.assertEquals(segments, ["tcm"])

    def test_sections(self):
        f = StringIO("SECTIONS\n{\ntcm . : { *(tcm)}\n}\n")
        segments = parsers.gnu.gnu_parser.extract_section_names(f)
        self.assertEquals(segments, ["tcm"])

    def test_ia32(self):
        f = open("data/ia32_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/ia32_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)

    def test_arm(self):
        f = open("data/arm_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/arm_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)    

    def test_arm_xip(self):
        f = open("data/arm_xip_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/arm_xip_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)    

    def test_mips(self):
        f = open("data/mips_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/mips_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)    

    def test_mips64(self):
        f = open("data/mips64_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/mips64_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)

    def test_mips64_2(self):
        f = open("data/mips64_2_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/mips64_2_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)

    def test_pistachio(self):
        f = open("data/pistachio.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/pistachio.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)

    def test_bug2116(self):
        f = open("data/bug2116_ld.lds")
        parsers.gnu.gnu_parser.extract_section_names(f)
        f = open("data/bug2116_ld.lds")
        parsers.gnu.gnu_parser.extract_segment_names(f)    

