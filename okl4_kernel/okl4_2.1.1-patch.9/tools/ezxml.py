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
###############################################################################
"""
ezxml provides support to parse_spec_xml. It extends the basic XML
parsing facility in python to somethign that is simpler to specify and
abstract away the DOM API from the user
"""
from xml.dom.minidom import parse, parseString
from xml.parsers.expat import ExpatError
import types

class ParsedElement:
    """This represents an element once it has been parsed."""
    def __init__(self, tagname):
        self.tag = tagname
        self.children = []
    
    def __repr__(self):
        """Return marked up parsed element"""
        base = "<%s " % self.tag
        for attr in [key for key in self.__dict__ if key
                     not in ["children", "tag"]]:
            base += '%s="%s" ' % (attr, getattr(self, attr))
        base += ">"
        return base

    def find_children(self, tagname):
        return [el for el in self.children if el.tag ==  tagname]

    def find_child(self, tagname):
        els = self.find_children(tagname)
        if len(els) == 0:
            return None
        else:
            return els[0]

class EzXMLError(Exception):
    """This exception is raised if an error occurs during traversing
    a given dom."""

class Element:
    """Defines a type of element, rather than a particular element
    in an XML file. E.g: Can be used to define what attributes and
    child a given node has, rather than representing a particular node
    with specific attributes and elements."""
    
    def __init__(self, __name, *children, **attributes):
        """Construct an Element with a given tagname 'name'. Attributes
        is a dictionary of valid attributes. Children is a last of
        child elements."""
        self.name = __name
        self.attributes = attributes
        for attr in self.attributes.values():
            if type(attr) != types.TupleType:
                raise EzXMLError("Attribute descriptors must be tuples")
            if len(attr) != 2:
                raise EzXMLError("Attribute descriptors must " + \
                                 "be 2-element tuples")
            if not callable(attr[0]):
                raise EzXMLError("First element in descriptor " + \
                                 "must be callable")
            if attr[1] not in ["required", "optional"]:
                raise EzXMLError("Must be 'required' or 'optional'")

        self.elements = dict([(el.name, el) for el in children])

    def parse_xml_str(self, data):
        """Parse a given XML string. Note this should only be used on the
        top level Element. Not child elements."""
        try:
            dom = parseString(data)
        except ExpatError, x:
            raise EzXMLError("Failed to parse: %s" % x)
        return self.walkdom(dom.firstChild)

    def parse_xml_file(self, filename):
        """Parse a given XML file. Note this should only be used on the
        top level Element. Not child elements."""
        try:
            dom = parse(filename)
        except ExpatError, x:
            raise EzXMLError("Failed to parse: %s" % x)
        return self.walkdom(dom.firstChild)

    def walkdom(self, el):
        """This methods walks the DOM and extracts attributes and children
        as it goes. It returns <something>, that provides a high level, pythonic
        representation of the XML file."""
        ret = ParsedElement(self.name)

        if el.tagName != self.name:
            raise EzXMLError("Names don't match: %s %s" % \
                             (el.tagName, self.name))

        if el.attributes:
            for attr in el.attributes.keys():
                if attr not in self.attributes:
                    raise EzXMLError("Unknonwn attribute: %s" % attr)
                setattr(ret, attr,
                        self.attributes[attr][0](el.attributes[attr].value))
        for attr_name, descriptor in self.attributes.items():
            if descriptor[1] == "required" and not hasattr(ret, attr_name):
                raise EzXMLError("Required attribute %s not specified in element %s" % \
                                 (attr_name, el.tagName))

        for child in el.childNodes:
            if child.nodeType == child.ELEMENT_NODE:
                if child.tagName not in self.elements:
                    raise EzXMLError("Unknown child node: %s" % child)
                ret.children.append(self.elements[child.tagName].walkdom(child))
            elif child.nodeType == child.TEXT_NODE:
                # Ignore text nodes for now
                pass
            elif child.nodeType == child.COMMENT_NODE:
                # Ignore command nodes
                pass
            else:
                raise EzXMLError("Unhandled node type: %d\n" % child.nodeType)
                    
        return ret

    def __call__(self, *children, **attributes):
        new = ParsedElement(self.name)
        for attr in attributes:
            setattr(new, attr, attributes[attr])
        for child in children:
            new.children.append(child)
        return new

        
def str_attr(attr):
    """Parses a string attribute."""
    return str(attr)

def long_attr(attr):
    """Parse a long attribute"""
    try:
        val = long(attr, 0)
    except ValueError:
        raise EzXMLError("%s did not parse as an integer" % attr)
    return val

def bool_attr(attr):
    """Parse a boolean attribute"""
    if attr == "true":
        val = True
    elif attr == "false":
        val = False
    else:
        raise EzXMLError("Must be "\
                         "'true' or 'false'. Not %s" % (attr))
    return val

