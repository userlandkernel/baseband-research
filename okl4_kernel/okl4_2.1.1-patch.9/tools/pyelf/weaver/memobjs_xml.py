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
Processing of the memory object XML tags.
"""

from weaver import MergeError
from weaver.ezxml import Element, long_attr, bool_attr, str_attr, size_attr
from weaver.segments_xml import attach_to_elf_flags, make_pager_attr
import weaver.bootinfo

Right_el = Element("right",
                   value = (str_attr, "required"))

Cap_el = Element("cap", Right_el,
                 name      = (str_attr, "required"))

Stack_el = Element("stack",
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
                   cache_policy = (str_attr, "optional"))
                  
Memsection_el = Element("memsection", Cap_el,
                        name      = (str_attr, "required"),
                        file      = (str_attr, "optional"),
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
                        cache_policy = (str_attr, "optional"))

def create_standard_caps(obj, namespace):
    """
    Create the standard caps for any object.

    These caps are for master, read only and read/write access.
    """
    master_cap = weaver.bootinfo.Cap("master", ["master"])
    obj.add_cap(master_cap)
    namespace.add(master_cap.get_name(), master_cap)

    rw_cap = weaver.bootinfo.Cap("rw", ["read", "write"])
    obj.add_cap(rw_cap)
    namespace.add(rw_cap.get_name(), rw_cap)

    ro_cap = weaver.bootinfo.Cap("ro", ["read"])
    obj.add_cap(ro_cap)
    namespace.add(ro_cap.get_name(), ro_cap)

def create_alias_cap(obj, namespace):
    """
    Create an alias cap for any object.

    These caps carry no inherent permission, but points to other real caps.
    """
    alias_cap = weaver.bootinfo.AliasCap(obj.name, obj)

    # no need for obj.add_cap because AliasCap.__init__ does it implicitly
    namespace.add(alias_cap.get_name(), alias_cap)

def collect_memobj_attrs(el, namespace, image, machine):
    """Collect the attributes of a memory section like element."""

    assert el is not None

    values = image.new_attrs(namespace)

    if el.tag == "stack" or el.tag == "heap":
        values.name = el.tag
    else:
        values.name = el.name
        assert el.name is not None

    values.file      = getattr(el, 'file', values.file)
    values.size      = getattr(el, 'size', values.size)
    values.virt_addr = getattr(el, 'virt_addr', values.virt_addr)
    values.phys_addr = getattr(el, 'phys_addr', values.phys_addr)
    values.direct    = getattr(el, 'direct', values.direct)
    values.virtpool  = getattr(el, 'virtpool', values.virtpool)
    values.physpool  = getattr(el, 'physpool', values.physpool)
    values.align     = getattr(el, 'align', values.align)
    values.scrub     = getattr(el, 'zero', values.scrub)
    values.usermap   = getattr(el, 'user_map', values.usermap)

    if hasattr(el, 'attach'):
        values.attach = attach_to_elf_flags(el.attach)

    if hasattr(el, 'pager'):
        values.pager = make_pager_attr(el.pager)

    if hasattr(el, 'cache_policy'):
        values.cache_policy = machine.get_cache_policy(el.cache_policy)
    

    return values

def collect_memsection_element(ms_el, ignore_name, namespace, image,
                               machine, pools):
    """Collect the details of a memsection, or other mem obj, element."""
    attrs = collect_memobj_attrs(ms_el, namespace, image, machine)

    # New namespace for objects living in the memsection.
    ms_namespace = namespace.add_namespace(attrs.name)

    ms = weaver.bootinfo.MemSection(image, machine, pools, attrs = attrs)

    # Add the standard caps for the memsection.
    create_standard_caps(ms, ms_namespace)

    # Collect any custom caps for the memsection.
    for cap_el in ms_el.find_children("cap"):
        if not ignore_name.match(cap_el.name):
            cap = weaver.bootinfo.Cap(cap_el.name)
    
            for right in cap_el.find_children("right"):
                cap.add_right(right.value)

            ms.add_cap(cap)
            ms_namespace.add(cap.get_name(), cap)

    return ms

