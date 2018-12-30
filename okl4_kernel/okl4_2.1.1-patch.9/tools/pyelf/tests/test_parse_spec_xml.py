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
import weaver.parse_spec
from weaver.ezxml import EzXMLError

modules_under_test = [weaver.parse_spec]

class TestParseXml(unittest.TestCase):
    def test_rubbish(self):
        f = StringIO("rubbish")
        self.assertRaises(EzXMLError, weaver.parse_spec.parse_spec_xml, f)

    def test_no_bootimage(self):
        f = StringIO("""<asdf></asdf>""")
        self.assertRaises(EzXMLError, weaver.parse_spec.parse_spec_xml, f)

    def test_one_machine(self):
        f = StringIO("""
        <image>
	<machine>
		<word_size size="0x20" />
	</machine>
        </image>""")
        parsed = weaver.parse_spec.parse_spec_xml(f)
        self.assertEquals(parsed.children[0].children[0].size, 0x20)

    def test_badmachine(self):
        f = StringIO("""
        <image>
	<machine>
		<word_size fsize="0x20" />
	</machine>
        </image>""")
        self.assertRaises(EzXMLError, weaver.parse_spec.parse_spec_xml, f)

    def test_good_string(self):
        s = """
<image>
	<machine>
		<word_size size="0x20" />
		<virtual_memory name="virtual">
			<region base="0x1000" size="0xcffff000" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="0xa0000000" size="0x3800000" />
		</physical_memory>
		<page_size size="0x1000" />
	</machine>
	<virtual_pool name="virtual">
		<memory src="virtual"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory src="physical" />
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming">
        </program>
</image>
"""
        weaver.parse_spec.parse_spec_xml_string(s)
