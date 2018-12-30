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

#pylint: disable-msg=R0903
"""
A set of useful stuff
"""

import types

class IntString(long):
    """A sub-type of integer that allows you to associate
    a string with a given integer"""
    _show = {}
    _default_string = None
    def __new__(cls, arg, string=None):
        if string:
            cls._show[arg] = string
        return long.__new__(cls, arg)

    def __str__(self):
        if long(self) in self._show:
            return self._show[long(self)]
        elif self._default_string:
            return self._default_string % long(self)
        else:
            return long.__str__(self)

def align_up(val, alignment):
    """Round val up to a given alignment."""
    if not alignment:
        return val
    ovr = val % alignment
    if (ovr):
        val = val + alignment - ovr
    return val

def align_down(val, alignment):
    """Round val down to a given alignment"""
    if alignment:
        val -= (val % alignment)
    return val

def is_integer(val):
    """Return true if the val is an integer or long type."""
    return isinstance(val, types.IntType) or isinstance(val, types.LongType)

class TransformableMixin:
    """This is a mix-in class which allows a class instance to be
    specialised after creation. For example this allows us to make
    subclasses of ElfSection, and then change existing instance into
    the subclass. This is very useful if you consider that the subclass
    may be being pointed to from various different places, and having back
    links to them all would really complicate things. This mixin is used
    for ElfSection."""
    # This class is pretty magic, and pylint doesn't deal with magic very 
    # well. In particular, it doesn't like the fact we don't have __init__,
    # and it doesn't realise that there is a builtin __class__ instance
    # variable.
    #pylint: disable-msg=W0232,W0201,E0203
    def transform(self, cls):
        """Transform is called to morph the current instance into
        a new class 'cls'. The new class must be a subclass of the
        of the current instances class. After changing the class,
        trasnformed() will be called to do any post fixups.

        This is called after a it is prepared.

        A subclass should provide a "transformer(self)" method.
        This method is called immediately after the class transformation
        and allows the subclass to doing any fixup necessary. It is designed
        to be overridden by subclasses, and is simply empty here.
        """
        if not issubclass(cls, self.__class__):
            raise Exception, "Can only transform into subclassess"
        self.__class__ = cls
        self.transformer()

class Span:
    """A simple little helper class that lets you easily test if
    a value is in between a given span of numbers. E.g:

    >>> 3 in Span(1, 5)
    True
    >>> 0 in Span(1, 5)
    False
    >>> 1 in Span(1, 5)
    True
    >>> 5 in Span(1, 5)
    False
    """
    def __init__(self, base, end):
        """Initialise a span starting from base and going to end."""
        self.base = base
        self.end = end

    def __contains__(self, val):
        """Return true if a given value is in the span."""
        return val >= self.base and val < self.end

class Prepared(Exception):
    """This exception is raised when a method is called and the
    object should be in the unprepared state, but is actually in
    the prepared state."""

class Unprepared(Exception):
    """This exception is raised when a method is called and the
    object should be in the prepared state, but is actually in the
    unprepared state."""

