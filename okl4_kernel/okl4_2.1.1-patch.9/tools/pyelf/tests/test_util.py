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

import elf.util

modules_under_test = [elf.util]

from elf.util import *

class TestIsInteger(unittest.TestCase):
    def test_isinteger(self):
        self.assertEqual(is_integer(0), True)
        self.assertEqual(is_integer(37L), True)
        self.assertEqual(is_integer(None), False)        

class TestAlign(unittest.TestCase):
    def test_align_none(self):
        self.assertEqual(elf.util.align_down(0x23, None), 0x23)

    def test_align_zero(self):
        self.assertEqual(elf.util.align_down(0x23, 0), 0x23)

    def test_align_one(self):
        self.assertEqual(elf.util.align_down(0x23, 1), 0x23)

    def test_align_20(self):
        self.assertEqual(elf.util.align_down(0x23, 0x20), 0x20)

class TestAlignDown(unittest.TestCase):
    def test_align_down(self):
        self.assertEqual(elf.util.align_down(8, 3), 6)
        self.assertEqual(elf.util.align_down(6, 3), 6)
        self.assertEqual(elf.util.align_down(2, 3), 0)
        self.assertEqual(elf.util.align_down(3, 3), 3)

class TestAlignUp(unittest.TestCase):
    def test_align_up(self):
        self.assertEqual(elf.util.align_up(8, 3), 9)
        self.assertEqual(elf.util.align_up(6, 3), 6)
        self.assertEqual(elf.util.align_up(2, 3), 3)
        self.assertEqual(elf.util.align_up(3, 3), 3)
        self.assertEqual(elf.util.align_up(5, 0), 5)

class TestIntString(unittest.TestCase):
    def test_simple(self):
        class TestInt(IntString):
            _show = {}
        class TestFoo(IntString):
            _show = {}
        TestInt(37, "pants")
        TestFoo(37, "foo")
        self.assertEqual(TestInt(37), 37)
        self.assertEqual(str(TestInt(37)), "pants")
        self.assertEqual(str(TestInt(25)), "25")

        self.assertEqual(TestFoo(37), 37)
        self.assertEqual(str(TestFoo(37)), "foo")
        self.assertEqual(str(TestFoo(25)), "25")

    def test_default_string(self):
        class TestDefaultString(IntString):
            _default_string = "%d"

        self.assertEqual(str(TestDefaultString(37)), str(37))

class TestTransformable(unittest.TestCase):
    def test_simple(self):
        class Base(TransformableMixin):
            pass

        class Subclass(Base):
            def test(self):
                return 2
            def transformer(self):
                pass

        base = Base()
        base.transform(Subclass)
        self.assertEquals(base.test(), 2)

    def test_simple_bad_transform(self):
        class Base(TransformableMixin):
            pass

        class Subclass(object):
            def test(self):
                return 2
            def transformer(self):
                pass

        base = Base()
        self.assertRaises(Exception, base.transform, Subclass)



class TestSpan(unittest.TestCase):
    def test_contains(self):
        self.assertEquals(3 in Span(1, 5), True)
        self.assertEquals(0 in Span(1, 5), False)
        self.assertEquals(1 in Span(1, 5), True)
        self.assertEquals(5 in Span(1, 5), False)
