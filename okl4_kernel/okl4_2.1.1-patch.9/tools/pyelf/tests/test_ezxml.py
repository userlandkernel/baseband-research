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
import weaver.ezxml
from weaver.ezxml import Element, ParsedElement, long_attr, bool_attr, \
     str_attr, size_attr, EzXMLError

modules_under_test = [weaver.ezxml]

class TestSimple(unittest.TestCase):
    def test_basic_usage(self):
        doc = Element("root")
        parsed = doc.parse_xml_str("<root />")
        self.assertEquals(parsed.tag, "root")
        parsed = doc.parse_xml_file("data/ezxml.1")
        self.assertEquals(parsed.tag, "root")

    def test_malformed_xml(self):
        doc = Element("root")
        self.assertRaises(EzXMLError, doc.parse_xml_str, "asdf")
        self.assertRaises(EzXMLError, doc.parse_xml_file, "data/ezxml.2")

    def test_invalidroot(self):
        doc = Element("root")
        self.assertRaises(EzXMLError, doc.parse_xml_str, "<bar />")

    def test_ignoring_comments(self):
        doc = Element("root")
        parsed = doc.parse_xml_str("<root><!-- This is a comment --></root>")
        self.assertEquals(parsed.tag, "root")
        self.assertEquals(len(parsed.children), 0)

    def test_simple_attributes(self):
        doc = Element("root", foo=(long_attr, "required"))
        dom = doc.parse_xml_str('<root foo="1234" />')
        self.assertEquals(dom.foo, 1234)

    def test_str_attributes(self):
        doc = Element("root", foo=(str_attr, "required"))
        dom = doc.parse_xml_str('<root foo="1234" />')
        self.assertEquals(dom.foo, "1234")

    def test_bool_attributes(self):
        doc = Element("root", foo=(bool_attr, "required"))
        dom = doc.parse_xml_str('<root foo="true" />')
        self.assertEquals(dom.foo, True)
        dom = doc.parse_xml_str('<root foo="false" />')
        self.assertEquals(dom.foo, False)
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root foo="foo" />')

    def test_long_attributes(self):
        doc = Element("root", foo=(long_attr, "required"))
        dom = doc.parse_xml_str('<root foo="1234" />')
        self.assertEquals(dom.foo, 1234)
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root foo="foo" />')

    def test_size_attributes(self):
        doc = Element("root", foo=(size_attr, "required"))
        dom = doc.parse_xml_str('<root foo="1234" />')
        self.assertEquals(dom.foo, 1234)
        dom = doc.parse_xml_str('<root foo="1K" />')
        self.assertEquals(dom.foo, 1024L)
        dom = doc.parse_xml_str('<root foo="1M" />')
        self.assertEquals(dom.foo, 1024L * 1024L)
        dom = doc.parse_xml_str('<root foo="1g" />')
        self.assertEquals(dom.foo, 1024L * 1024L * 1024L)
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root foo="foo" />')

    def test_invalid_attributes(self):
        doc = Element("root")
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root bar="1234" />')

        doc = Element("root", foo=(long_attr, "required"))
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root bar="1234" />')

    def test_required_attributes(self):
        doc = Element("root", bar=(bool_attr, "required"))
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root />')

    def test_optional_attributes(self):
        doc = Element("root",  bar=(bool_attr, "optional"))
        dom = doc.parse_xml_str('<root />')
        self.assertEquals(hasattr(dom, "bar"), False)
        dom = doc.parse_xml_str('<root bar="true" />')
        self.assertEquals(hasattr(dom, "bar"), True)
        self.assertEquals(dom.bar, True)

    def test_bogus_attribute_option(self):
        Element("root", bar=(bool_attr, "required"))
        Element("root", bar=(bool_attr, "optional"))
        self.assertRaises(EzXMLError, Element, "root", bar=bool_attr)
        self.assertRaises(EzXMLError, Element, "root", bar=(bool_attr, "asdf"))
        self.assertRaises(EzXMLError, Element, "root", bar=("asdf", "required", "asdf"))
        self.assertRaises(EzXMLError, Element, "root", bar=("asdf", "required"))

    def test_call(self):
        doc = Element("foo")
        dom = doc(ParsedElement("child"), bar=True)
        self.assertEquals(len(dom.children), 1)
        self.assertEquals(dom.bar, True)

    def test_simple_child(self):
        doc = Element("root", Element("child"))
        dom = doc.parse_xml_str('<root><child /></root>')
        self.assertEquals(len(dom.children), 1)

    def test_find_child(self):
        child = Element("child", name=(str_attr, "required"))
        doc = Element("root", child)

        # 1 child
        dom = doc.parse_xml_str('<root><child name="1" /></root>')
        child_el = dom.find_child("child")
        self.assertEquals(child_el.tag, "child")

        # 0 children
        dom = doc.parse_xml_str('<root></root>')
        child_el = dom.find_child("child")
        self.assertEquals(child_el, None)

        # Multiple chidren
        dom = doc.parse_xml_str('<root><child name="2" /><child name="1" /></root>')
        child_el = dom.find_child("child")
        self.assertEquals(child_el.tag, "child")
        self.assertEquals(child_el.name, "2")

    def test_find_children(self):
        child = Element("child", name=(str_attr, "required"))
        doc = Element("root", child)

        dom = doc.parse_xml_str('<root><child name="1" /></root>')
        child_els = dom.find_children("child")
        self.assertEqual(len(child_els), 1)
        self.assertEqual(child_els[0].name, "1")

        dom = doc.parse_xml_str('<root></root>')
        child_els = dom.find_children("child")
        self.assertEqual(len(child_els), 0)

        dom = doc.parse_xml_str('<root><child name="1" /><child name="2" /></root>')
        child_els = dom.find_children("child")
        self.assertEqual(len(child_els), 2)
        self.assertEqual(child_els[0].name, "1")
        self.assertEqual(child_els[1].name, "2")

    def test_inclusion(self):
        """Test the processing of the include element."""
        child = Element("child", name=(str_attr, "required"))
        doc = Element("root", child)

        # Test file not found.
        self.assertRaises(EzXMLError, doc.parse_xml_str,
                          '<root><include file="nofile" /></root>')

        # Test a simple include.
        dom = doc.parse_xml_str('<root><include file="data/ezxml_include.xml" /></root>')
        # Include element should have been removed.
        includes = dom.find_children("include")
        self.assertEqual(len(includes), 0)
        child_els = dom.find_children("child")
        self.assertEqual(len(child_els), 1)
        self.assertEqual(child_els[0].name, "included")

        # Test including a file with multiple elements and thatalso
        # includes another file.
        dom = doc.parse_xml_str('<root><include file="data/ezxml_include2.xml" /></root>')
        # Include element should have been removed.
        includes = dom.find_children("include")
        self.assertEqual(len(includes), 0)
        child_els = dom.find_children("child")
        self.assertEqual(len(child_els), 7)
        self.assertEqual(child_els[0].name, "included1")
        self.assertEqual(child_els[6].name, "included")


    def test_find_all_children(self):
        leaf = Element("leaf", name=(str_attr, "required"))
        child = Element("child", leaf, name=(str_attr, "required"))
        doc = Element("root", child, leaf)

        dom = doc.parse_xml_str("""
        <root>
            <child name="1">
                <leaf name="bottom_leaf" />
            </child>
            <leaf name="top_leaf" />
            </root>""")
        leaf_els = dom.find_all_children("leaf")
        self.assertEqual(len(leaf_els), 2)
        self.assertEqual(leaf_els[0].name, "bottom_leaf")
        self.assertEqual(leaf_els[1].name, "top_leaf")

        dom = doc.parse_xml_str('<root></root>')
        leaf_els = dom.find_children("leaf")
        self.assertEqual(len(leaf_els), 0)


    def test_invalid_child(self):
        doc = Element("root")
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root><child /></root>')
        doc = Element("root", Element("foobar"))
        self.assertRaises(EzXMLError, doc.parse_xml_str, '<root><child /></root>')

    def test_empty_tag(self):
        doc = Element("root")
        doc.parse_xml_str('<root> </root>')

    def test_dump(self):
        # Test the dumped output.
        output = """\
<root >
    <child name="1" >
    </child>
    <child value="0x32" name="2" >
    </child>
</root>
"""
        child = Element("child", name=(str_attr, "required"), value=(long_attr, "optional"))
        doc = Element("root", child)
        dom = doc.parse_xml_str('<root><child name="1" /><child name="2" value="50"/></root>')
        self.assertEqual(dom.dump(), output)

    def test_unknown_node_type(self):
        """This test is a bit hacky, because there is no real way this error
        could be triggered, but we want to ensure we test that exception raise
        somehow, this goes to lengths to do that."""
        class Empty:
            pass
        empty = Empty
        doc = Element("root")
        empty.tagName = "root"
        empty.attributes = {}
        empty2 = Empty
        empty2.nodeType = 37
        empty2.ELEMENT_NODE = 1
        empty2.TEXT_NODE = 3
        empty2.COMMENT_NODE = 8
        empty.childNodes = [empty2]
        self.assertRaises(EzXMLError, doc.walkdom, empty)

    def test_repr(self):
        doc = Element("root", foo=(bool_attr, "required"),
                      bar=(long_attr, "required"))
        parse = doc.parse_xml_str("<root foo='true' bar='16' />")
        self.assertEquals(repr(parse), '<root bar="0x10" foo="True" >')
