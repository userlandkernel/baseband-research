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
Test file for File.py
"""

import unittest
import elf.File

modules_under_test = [elf.File]

File = elf.File.File

class TestFile(unittest.TestCase):
    def setUp(self):
        self.l4kernel = File("data/arm_exec", "rb")
    
    def test_stat(self):
        self.l4kernel.stat()

    def test_inode(self):
        self.l4kernel.inode()

    def test_device(self):
        self.l4kernel.device()

    def test_num_links(self):
        links = self.l4kernel.num_links()
        self.assertEqual(links, 1)

    def test_uid(self):
        self.l4kernel.uid()

    def test_gid(self):
        self.l4kernel.gid()

    def test_size(self):
        size = self.l4kernel.size()
        self.assertEqual(size, 1306358)

    def test_atime(self):
        self.l4kernel.atime()

    def test_mtime(self):
        self.l4kernel.mtime()

    def test_ctime(self):
        """Test that ctime() work. This only checks it runs without
        errors, not the value."""
        self.l4kernel.ctime()

    def test_isdir(self):
        """Test the is_dir() method."""
        self.assertEqual(self.l4kernel.is_dir(), False)

    def test_ischaracter(self):
        """Test the is_character() method."""
        self.assertEqual(self.l4kernel.is_character(), False)

    def test_isblock(self):
        """Test the is_block() method."""
        self.assertEqual(self.l4kernel.is_block(), False)

    def test_isdevice(self):
        """Test the is_device() method."""
        self.assertEqual(self.l4kernel.is_device(), False)

    def test_regular(self):
        """Test the is_regular() method."""
        self.assertEqual(self.l4kernel.is_regular(), True)

    def test_isfifo(self):
        """Test the is_fifo() method."""
        self.assertEqual(self.l4kernel.is_fifo(), False)

    def test_issymlink(self):
        """Test the is_symlink() method."""
        self.assertEqual(self.l4kernel.is_symlink(), False)

    def test_issocket(self):
        """Test the is_socket() method."""
        self.assertEqual(self.l4kernel.is_socket(), False)

    def test_getdata(self):
        """Test the get_data() method."""
        self.assertEqual(len(self.l4kernel.get_data(0, 10)), 10)
        self.assertRaises(EOFError, self.l4kernel.get_data,
                          0, self.l4kernel.size() + 1)
        self.assertRaises(IOError, self.l4kernel.get_data, -1, 0)
