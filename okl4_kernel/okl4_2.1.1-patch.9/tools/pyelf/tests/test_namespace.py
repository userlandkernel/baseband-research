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
import weaver.namespace

modules_under_test = [weaver.namespace]

class TestXMLNamespace(unittest.TestCase):
    def setUp(self):
        # Set up a POSIX-like test tree with the following objects:
        #  /vmlinux
        #  /etc/passwd
        #  /etc/master
        #  /etc/init_d/xdm
        #
        # and the following extra namespaces:
        #  /empty
        #
        self.root = weaver.namespace.ObjectNameSpace(None)
        self.root.add('vmlinux', 'vmlinux')
        self.etc = self.root.add_namespace('etc')
        self.etc.add('passwd', 'passwd')
        self.etc.add('master', 'master')
        self.init_d = self.etc.add_namespace('init_d')
        self.init_d.add('xdm', 'xdm')
        self.empty = self.root.add_namespace('empty')

    def tearDown(self):
        self.empty  = None
        self.init_d = None
        self.etc    = None
        self.root   = None

    def test_get_namespace(self):
        """Test the get_namespace() method."""
        self.assertEqual(self.root.get_namespace("missing"), None)
        self.assertEqual(self.root.get_namespace("vmlinux"), None)
        self.assertNotEqual(self.root.get_namespace("etc"), None)

    def test_name_clash(self):
        """
        Test that adding a name that already exists will raise an exception.
        """
        self.assertRaises(weaver.MergeError, self.init_d.add, 'xdm', 'another_xdm')

    def test_abs_root(self):
        """Look for an object at the top of the namespace."""
        vmlinux = self.root.lookup('/vmlinux')
        self.assertEqual(vmlinux, 'vmlinux')

        vmlinux = self.etc.lookup('/vmlinux')
        self.assertEqual(vmlinux, 'vmlinux')

        vmlinux = self.init_d.lookup('/vmlinux')
        self.assertEqual(vmlinux, 'vmlinux')

        vmlinux = self.empty.lookup('/vmlinux')
        self.assertEqual(vmlinux, 'vmlinux')
        
    def test_abs_path(self):
        """
        Test returning the absolute pathname of a name.
        """
        abs_path = self.root.abs_name('vmlinux')
        self.assertEqual(abs_path, '/vmlinux')

        abs_path = self.init_d.abs_name('xdm')
        self.assertEqual(abs_path, '/etc/init_d/xdm')

        abs_path = self.init_d.abs_name('.')
        self.assertEqual(abs_path, '/etc/init_d')

        abs_path = self.root.abs_name('.')
        self.assertEqual(abs_path, '/')

        abs_path = self.root.abs_name('does_not_exist')
        self.assertEqual(abs_path, None)

        

    def test_search_abs_paths(self):
        """
        Test that objects can be found when referred to via
        absolute paths.
        """
        paths = [
            ('/vmlinux', 'vmlinux'),
            ('/etc/passwd', 'passwd'),
            ('/etc/init_d/xdm', 'xdm')
            ]

        for path in paths:
            thefile = self.root.lookup(path[0])
            self.assertEqual(thefile, path[1],
                             "Looking for '%s' found '%s' instead of '%s'" % (path[0], thefile, path[1]))

    def test_abs_missing(self):
        """Test that non-existent files are not found."""
        paths = [
            ('/vmlinux2', None),
            ('/etc/nothing', None),
            ('/etc/init_d/None', None),
            ('/I/do/not/exist', None)
            ]

        for path in paths:
            thefile = self.root.lookup(path[0])
            self.assertEqual(thefile, path[1],
                             "Looking for '%s' found '%s' instead of '%s'" % (path[0], thefile, path[1]))

    def test_current_dir(self):
        """Test relative searches from various namespaces."""
        thefile = self.root.lookup('vmlinux')
        self.assertEqual(thefile, 'vmlinux')

        thefile = self.root.lookup('missing')
        self.assertEqual(thefile, None)

        thefile = self.etc.lookup('passwd')
        self.assertEqual(thefile, 'passwd')

        thefile = self.etc.lookup('missing')
        self.assertEqual(thefile, None)

        thefile = self.etc.lookup('passwd/foo')
        self.assertEqual(thefile, None)

        thefile = self.init_d.lookup('xdm')
        self.assertEqual(thefile, 'xdm')

        thefile = self.init_d.lookup('missing')
        self.assertEqual(thefile, None)

    def test_relative_paths(self):
        """Test relative path searches from various namespaces."""
        thefile = self.root.lookup('etc/passwd')
        self.assertEqual(thefile, 'passwd')

        thefile = self.etc.lookup('../vmlinux')
        self.assertEqual(thefile, 'vmlinux')

        thefile = self.etc.lookup('init_d/xdm')
        self.assertEqual(thefile, 'xdm')

        thefile = self.init_d.lookup('../../vmlinux')
        self.assertEqual(thefile, 'vmlinux')

    def test_root_parent(self):
        """Test that the parent of the root directory is itself."""
        
        thefile = self.root.lookup('../vmlinux')
        self.assertEqual(thefile, 'vmlinux')

        thefile = self.root.lookup('../../vmlinux')
        self.assertEqual(thefile, 'vmlinux')

        thefile = self.etc.lookup('../../vmlinux')
        self.assertEqual(thefile, 'vmlinux')

    def test_master(self):
        """
        Test that looking for a namespace will return the master
        object it is it present.
        """
        thefile = self.root.lookup('/')
        self.assertEqual(thefile, None)

        thefile = self.root.lookup('.')
        self.assertEqual(thefile, None)

        thefile = self.root.lookup('/etc')
        self.assertEqual(thefile, 'master')
        
        thefile = self.etc.lookup('.')
        self.assertEqual(thefile, 'master')
        
        thefile = self.etc.lookup('init_d')
        self.assertEqual(thefile, None)

        thefile = self.root.lookup('empty')
        self.assertEqual(thefile, None)

        thefile = self.empty.lookup('.')
        self.assertEqual(thefile, None)

