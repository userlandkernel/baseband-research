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

import unittest
import weaver.kernel_elf
from StringIO import StringIO
from elf.core import PreparedElfFile
from weaver.kernel_elf import find_kernel_config, MemoryDescriptor
modules_under_test = [weaver.kernel_elf]

class TestL4Kcp(unittest.TestCase):
    def test_find_kcp_valid(self):
        elf = PreparedElfFile(filename="data/eg_weave/l4kernel")
        find_kernel_config(elf)

    def test_output(self):
        # Test the kernel before weaving.
        out = StringIO()
        elf = PreparedElfFile(filename="data/eg_weave/l4kernel")
        kcp = find_kernel_config(elf)
        kcp.output(out)

        # Test the kernel in a build image.
        out = StringIO()
        elf = PreparedElfFile(filename="data/eg_weave/image_ia32.elf")
        kcp = find_kernel_config(elf)
        kcp.output(out)

    def test_find_kcp_valid(self):
        elf = PreparedElfFile(filename="data/arm_object")
        self.assertEquals(find_kernel_config(elf), None)
        
    def test_add_memory_descriptor(self):
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        kconfig.add_meminfo(0x1, 0x2, True, 0x1000, 0x2000)
        # This will merge with the previous descriptor.
        kconfig.add_meminfo(0x1, 0x2, True, 0x3000, 0x4000)
        kconfig.add_mem_descriptor(MemoryDescriptor(0x1, 0x2, True, 0x1000, 0x2000))

    def test_updated_data(self):
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        kconfig.add_meminfo(0x1, 0x2, True, 0x1000, 0x2000)
        kconfig.update_data()

    def test_lots_memory_descriptor(self):
        """Add more descriptors than are allowed in the outout image."""
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        for i in range(0x1000, 0x600000, 0x20000):
            kconfig.add_meminfo(0x1, 0x2, True, i, i + 0x1000)
        self.assertRaises(weaver.MergeError, kconfig.update_data)

    def test_add_mapping(self):
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        kconfig.add_mapping((0x1000, 0x100000, 0x4000))
        kconfig.update_data()

    def test_lots_add_mapping(self):
        """Add more mappings than are allowed in the outout image."""
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        for i in range(0, 10):
            kconfig.add_mapping((0x1000, 0x100000, 0x4000))

        self.assertRaises(weaver.MergeError, kconfig.update_data)
            

    def test_set_entry(self):
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        kconfig.set_entry(0x20000000)

    def test_set_stack(self):
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        kconfig.set_stack(0x20000000)

    def test_set_config(self):
        kconfig = find_kernel_config(PreparedElfFile(filename="data/eg_weave/l4kernel"))
        kconfig.set_config("spaces", 10)
        kconfig.set_config("mutexes", 10)
        kconfig.set_config("root_caps", 10)
        self.assertRaises(weaver.MergeError, kconfig.set_config, "unknown", 10)

class TestMemoryDescriptor(unittest.TestCase):
    def test_memory_descriptor(self):
        memdesc = MemoryDescriptor(0, 0, True, 0, 0x1000)

        self.assertEqual(str(memdesc),
                         "MemoryDescriptor: type: undefined, low: 0x0, high: 0x1000")

    def test_physical(self):
        memdesc = MemoryDescriptor(0, 0, True, 0, 0x1000)
        self.assertEquals(memdesc.is_physical(), False)
        memdesc = MemoryDescriptor(0, 0, False, 0, 0x1000)
        self.assertEquals(memdesc.is_physical(), True)        

    def test_virtual(self):
        memdesc = MemoryDescriptor(0, 0, True, 0, 0x1000)
        self.assertEquals(memdesc.is_virtual(), True)
        memdesc = MemoryDescriptor(0, 0, False, 0, 0x1000)
        self.assertEquals(memdesc.is_virtual(), False)        

    def test_gethigh(self):
        memdesc = MemoryDescriptor(0, 0, True, 0, 0x1000)
        self.assertEquals(memdesc.get_high(), 0x1000)

    def test_sethigh(self):
        memdesc = MemoryDescriptor(0, 0, True, 0, 0x1000)
        self.assertEquals(memdesc.get_high(), 0x1000)
        memdesc.set_high(0x2000)
        self.assertEquals(memdesc.get_high(), 0x2000)

    def test_getlow(self):
        memdesc = MemoryDescriptor(0, 0, True, 0x37, 0x1000)
        self.assertEquals(memdesc.get_low(), 0x37)

    def test_gettype(self):
        memdesc = MemoryDescriptor(5, 0, True, 0x37, 0x1000)
        self.assertEquals(memdesc.get_type(), 5)

    def test_getsubtype(self):
        memdesc = MemoryDescriptor(5, 3, True, 0x37, 0x1000)
        self.assertEquals(memdesc.get_sub_type(), 3)

    def test_merge(self):
        memdesc1 = MemoryDescriptor(0, 0, True, 0x1000, 0x2000)
        memdesc2 = MemoryDescriptor(0, 0, True, 0, 0x3000)

        memdesc1.merge(memdesc2)
        self.assertEquals(memdesc1.get_low(), 0)
        self.assertEquals(memdesc1.get_high(), 0x3000)
