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
# We disable the warning about a class having too many public methods
# because for this class we are sure it is a good thing.
#pylint: disable-msg=R0904
"""
This module provides the File class.
"""

__revision__ = 0.1

import stat
import os
from elf.ByteArray import ByteArray

class File(file):
    """File extends the in-built python "file" type to give it some
    useful extra functions. In particular is support a convenient way
    of getting stat-like data out of it. E.g: File.size()"""
    
    def stat(self):
        """Return stat tuple."""
        return os.fstat(self.fileno())        
    
    def inode_mode(self):
        """Inode protection mode."""
        return self.stat()[stat.ST_MODE]

    def inode(self):
        """Inode number."""
        return self.stat()[stat.ST_INO]
    
    def device(self):
        """Device inode resides on"""
        return self.stat()[stat.ST_DEV]

    def num_links(self):
        """Number of links to the inode"""
        return self.stat()[stat.ST_NLINK]

    def uid(self):
        """User id of the owner"""
        return self.stat()[stat.ST_UID]

    def gid(self):
        """Group id of the owner"""
        return self.stat()[stat.ST_GID]

    def size(self):
        """Size in bytes of a plain file; amount of data
        waiting on some special files."""
        return self.stat()[stat.ST_SIZE]

    def atime(self):
        """Time of last access."""
        return self.stat()[stat.ST_ATIME]

    def mtime(self):
        """Time of last modification."""
        return self.stat()[stat.ST_MTIME]
    
    def ctime(self):
        """The ``ctime'' as reported by the operating system."""
        return self.stat()[stat.ST_CTIME]

    def is_dir(self):
        """Return true if the file is a directory."""
        return stat.S_ISDIR(self.inode_mode())

    def is_character(self):
        """Return true if the file is a character device."""
        return stat.S_ISDIR(self.inode_mode())

    def is_block(self):
        """Return true if the file is a block device."""
        return stat.S_ISDIR(self.inode_mode())

    def is_device(self):
        """Return true if the file is a device node.
        (Either chararacter, or block)."""
        return self.is_block() or self.is_character()

    def is_regular(self):
        """Return true if the file is a regular file."""
        return stat.S_ISREG(self.inode_mode())

    def is_fifo(self):
        """Return true if the file is a FIFO."""
        return stat.S_ISFIFO(self.inode_mode())

    def is_symlink(self):
        """Return true if the file is a symbolic link."""
        return stat.S_ISLNK(self.inode_mode())

    def is_socket(self):
        """Return true if the file is a socket."""
        return stat.S_ISSOCK(self.inode_mode())

    def get_data(self, base, size):
        """Get some size bytes of data starting at base.
        base must be > 0."""
        assert 'b' in self.mode
        self.seek(base)
        data = ByteArray()
        data.fromfile(self, size)
        return data
