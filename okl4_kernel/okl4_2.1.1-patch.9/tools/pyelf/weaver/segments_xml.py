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

"""Process segment XML elements."""

import re
from elf.constants import PF_X, PF_W, PF_R, EM_386
from weaver import MergeError
from weaver.ezxml import Element, long_attr, bool_attr, str_attr, size_attr

# General element
Segment_el = Element("segment",
                     name      = (str_attr, "required"),
                     phys_addr = (long_attr, "optional"),
                     physpool  = (str_attr, "optional"),
                     align     = (size_attr, "optional"),
                     attach    = (str_attr,  "optional"),
                     direct    = (bool_attr, "optional"),
                     pager     = (str_attr, "optional"),
                     protected = (bool_attr, "optional"),
                     cache_policy = (str_attr, "optional"))

Heap_el = Element("heap",
                  size      = (size_attr, "optional"),
                  virt_addr = (long_attr, "optional"),
                  phys_addr = (long_attr, "optional"),
                  physpool  = (str_attr, "optional"),
                  virtpool  = (str_attr, "optional"),
                  align     = (size_attr, "optional"),
                  attach    = (str_attr,  "optional"),
                  direct    = (bool_attr, "optional"),
                  zero      = (bool_attr, "optional"),
                  pager     = (str_attr, "optional"),
                  cache_policy = (str_attr, "optional"),
                  user_map  = (bool_attr, "optional"))
                  

Patch_el = Element("patch",
                   address = (str_attr, "required"),
                   value   = (long_attr, "required"),
                   bytes   = (long_attr, "optional"))

def start_to_value(start, elf):
    """
    Convert a start value, which may be a number, a text value of a
    number or a symbol, to a number.  ELF is used to resolve the value
    of a symbol.
    """
    # FIXME: depending on the type of start is not so nice
    if start is not None and not isinstance(start, (int, long)):
        if re.match("\s*0[xX][0-9a-fA-F]+$", start):
            start = long(start, 0)
        else:
            sym = elf.find_symbol(start)

            if not sym:
                raise MergeError, "Symbol %s not found" % (start)

            start = sym.value

    return start

def segments_hash(segments_el):
    """Convert a list of segments elements into a hash, indexed by name."""
    shash = {}

    for segment_el in segments_el:
        shash[segment_el.name] = segment_el

    return shash

def elf_segment_names(elf):
    """Return a hash of elf segment names, indexed by segment number."""
    table = {}

    seg_names_sect = elf.find_section_named(".segment_names")

    if seg_names_sect is not None:
        names = seg_names_sect.get_strings()[1:]

        for i in range(len(names)):
            table[i] = names[i]

    return table

def attach_to_elf_flags(attach):
    """Convert the attach 'rwx' string to ELF flags."""
    flags = 0L

    if attach.find('r') != -1:
        flags |= PF_R

    if attach.find('w') != -1:
        flags |= PF_W

    if attach.find('x') != -1:
        flags |= PF_X

    return flags

def make_pager_attr(pager):
    """Check that the pager value is valid."""
    VALID_PAGERS = ("none", "default", "memload")

    if pager not in VALID_PAGERS:
        raise MergeError, \
              '"%s" is not a recognised pager.  Valid values are %s.' % \
              (pager, VALID_PAGERS)

    if pager is 'none':
        pager = None

    return pager

def collect_patches(elf, patch_els, filename, image):
    """Process 'patch' elements."""
    for patch_el in patch_els:
        if re.match("\s*0[Xx][0-9a-fA-F]+L?$", patch_el.address):
            # convert from a string hex representation to a number 
            patch_el.address = long(patch_el.address, 0)
            # ensure we know how big it is
            if not hasattr(patch_el, "bytes"):
                raise MergeError, \
                      "Bytes attribute must be specified if patch " \
                      "address is a number." 
        else:
            # look up address of symbol
            name = patch_el.address
            sym = elf.find_symbol(patch_el.address)

            if not sym:
                raise MergeError, \
                      "symbol %s not found in %s" % (patch_el.address, filename)
            patch_el.address = sym.value

            if not hasattr(patch_el, "bytes"):
                # set the size
                patch_el.bytes = sym.get_size()
                if patch_el.bytes == 0:
                    raise MergeError, \
                          "Elf file does not specify size of symbol " \
                          "%s, please specify 'bytes' in this patch " \
                          "element" % name
        
        image.patch(patch_el.address, patch_el.bytes,
                                 patch_el.value)

def collect_elf_segments(elf, file_type, segment_els, section_prefix, namespace, image, machine, pools):
    """
    Process all of the segment elements in a program/kernel, etc.
    """
    elf_seg_names = elf_segment_names(elf)
    shash = segments_hash(segment_els)

    # Test that every segment element references a segment in the ELF
    # file.

    elf_seg_names_txt = elf_seg_names.values()
    for seg_name in shash.keys():
        if seg_name not in elf_seg_names_txt:
            raise MergeError, \
                  '%s: Cannot find segment "%s" in the ELF file. ' \
                  'Valid values are %s' % (namespace.abs_name('.'), seg_name, elf_seg_names_txt)

    collected_segments = []

    i           = 0
    last_segment = None # The last segment processed.
    group = []
    
    for segment in elf.segments:
        attrs = image.new_attrs(namespace, for_segment = True)
        
        attrs.virt_addr = segment.vaddr
        attrs.attach    = segment.flags
        attrs.elf_flags = segment.flags

        # The kernel is *very* picky about alignment, so use the
        # segment's alignment rules by default.
        if file_type == image.KERNEL:
            attrs.align  = segment.align
            # XXX: The kernel image is generated very weirdly on x86.  These
            # hacks are connected to some other PC99 hacks in tools/build.py.
            if elf.machine == EM_386:
                attrs.align = machine.min_page_size()
                attrs.phys_addr = segment.paddr

        if elf_seg_names.has_key(i):
            seg_name = elf_seg_names[i]

            attrs.name = seg_name

            if shash.has_key(seg_name):
                segment_el = shash[seg_name]

                attrs.phys_addr = getattr(segment_el, 'phys_addr', attrs.phys_addr)
                attrs.physpool  = getattr(segment_el, 'physpool', attrs.physpool)
                attrs.align     = getattr(segment_el, 'align', attrs.align)
                attrs.pager     = getattr(segment_el, 'pager', attrs.pager)
                attrs.direct    = getattr(segment_el, 'direct', attrs.direct)
                attrs.protected = getattr(segment_el, 'protected', attrs.protected)

                if hasattr(segment_el, 'attach'):
                    attrs.attach = attach_to_elf_flags(segment_el.attach)

                if hasattr(segment_el, 'pager'):
                    attrs.pager = make_pager_attr(segment_el.pager)

                if hasattr(segment_el, 'cache_policy'):
                    attrs.cache_policy = machine.get_cache_policy(segment_el.cache_policy)
        else:
            attrs.name = str(i)

        s = image.add_segment(segment_index   = i,
                              segment         = segment,
                              section_prefix  = section_prefix,
                              file_type       = file_type,
                              attrs           = attrs,
                              machine         = machine,
                              pools           = pools)

        if s is not None:
            collected_segments.append(s)

            # It is possible for different segments to occupy the same
            # page of memory (yes!  really!).  To accommodate this
            # place segments that have the same flags into the same
            # static allocation groups, where sub-page allocation is
            # allowed.
            #
            # Do not set the maximum distance the segments can be
            # apart.  The distance support was originally designed
            # with this code in mind, but with testing it's been shown
            # that it is difficult to get the distance value right.

            if last_segment is not None and \
                   last_segment.flags != segment.flags:
                image.add_group(None, group)
                group = []

            group.append(s)

        last_segment = segment

        i = i + 1

    image.add_group(None, group)

    return collected_segments
