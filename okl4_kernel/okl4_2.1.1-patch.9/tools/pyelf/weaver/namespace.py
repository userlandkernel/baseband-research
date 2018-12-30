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
A namespace for objects declared in the XML config file and used in
the object environment.

Namespaces are hierarchical and have POSIX-like semantics.
"""

from weaver import MergeError

class ObjectNameSpace:
    """
    A namespace for objects declared in the XML config file and used in
    the object environment.

    Namespaces are hierarchical and have POSIX-like semantics.
    """

    SEPARATOR = '/'

    def __init__(self, parent):
        # If parent is None, then this namespace is the top of the tree.
        if parent is None:
            parent = self
            root   = self
        else:
            root   = parent.root

        self.root    = root
        self.parent  = parent
        self.symbols = {}

    def get_namespace(self, name):
        """
        Return the named child namespace.  Search is non-recursive.

        If the name doesn't refer to a namespace, return None.
        """
        obj = self.symbols.get(name, None)        
        if isinstance(obj, ObjectNameSpace):
            return obj
        return None

    def add(self, name, the_object):
        """
        Add an object to the namespace under the given name if the
        name isn't already in use.
        """
        if name in self.symbols:
            raise MergeError, 'Object name "%s" already in use.' % name

        self.symbols[name] = the_object

    def add_namespace(self, name):
        """Add a new sub-namespace to this namespace."""
        new_namespace = ObjectNameSpace(parent=self)
        self.add(name, new_namespace)

        return new_namespace

    def namei(self, parts):
        """
        Recursively move the list of names, looking for the  target.

        Historical Note: This function is named after a similar
        function in the UNIX v6 kernel.
        """
        
        if len(parts) == 0:
            # If the name refers to a namespace, return the master cap
            # if present.
            return self.symbols.get('master', None)
        elif parts[0] == '.':
            return self.namei(parts[1:])
        elif parts[0] == '..':
            return self.parent.namei(parts[1:])
        elif parts[0] in self.symbols:
            obj = self.symbols[parts[0]]
            if isinstance(obj, ObjectNameSpace):
                return obj.namei(parts[1:])
            elif len(parts) == 1:
                return obj
            else:
                return None            
        else:
            return None

    def lookup(self, name):
        """
        Lookup a name in the namespace.  If found the object will be
        returned, otherwise None.
        """
        # Convert the string into a list of parts.  Absolute paths
        # have an empty string as the first element.
        parts = name.split(ObjectNameSpace.SEPARATOR)

        # If an absolute path, then search from the root of the
        # namespace, otherwise search from the current namespace.
        if parts[0] == '':
            start = self.root
            parts = parts[1:]
        else:
            start = self
        
        return start.namei(parts)

    def abs_name(self, name):
        """
        Return the absolute path of a name in the current namespace.
        Returns None if the name does not exist.
        """
        if name != '.':
            if name not in self.symbols:
                return None
            path = [name]
        else:
            path = []

        node   = self
        parent = self.parent

        while parent is not node:
            for (dir_name, obj) in parent.symbols.items():
                if obj is node:
                    path = [dir_name] + path
                    break

            node = parent
            parent = node.parent

        return self.SEPARATOR + self.SEPARATOR.join(path)
