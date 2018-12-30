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
Processing of the pool XML tags.
"""

from weaver.ezxml import Element, long_attr, bool_attr, str_attr, size_attr
from weaver.memobjs_xml import create_standard_caps
import weaver.bootinfo

PoolMemory_el = Element("memory",
                        src  = (str_attr, "optional"),
                        base = (long_attr, "optional"),
                        size = (size_attr, "optional"))
                       
VirtualPool_el = Element("virtual_pool", PoolMemory_el,
                         name = (str_attr, "required"))

PhysicalPool_el = Element("physical_pool", PoolMemory_el,
                          name = (str_attr, "required"),
                          direct = (bool_attr, "optional"))

def setup_direct_pool(namespace, machine, bootinfo, pools):
    # At the moment we just have a special virtpool.
    direct_pool = weaver.bootinfo.VirtPool("direct", machine, pools)
    bootinfo.add_virtpool(direct_pool)
    new_namespace = namespace.add_namespace("direct")

    # This cap is probably never used.
    master = weaver.bootinfo.Cap("master", ["master"])
    direct_pool.add_cap(master)
    new_namespace.add(master.get_name(), master)

def collect_memory_pool_el(memory_pool_el, namespace, machine,
                           bootinfo, pools, virtual):
    """ Record a memory pool."""

    if virtual:
        pool = weaver.bootinfo.VirtPool(memory_pool_el.name, machine,
                                        pools)
        bootinfo.add_virtpool(pool)
    else:
        pool = weaver.bootinfo.PhysPool(memory_pool_el.name, machine,
                                        pools)
        bootinfo.add_physpool(pool)

    # New namespace for the memory pool's caps.
    new_namespace = namespace.add_namespace(memory_pool_el.name)

    # Add the standard caps for the pool.
    create_standard_caps(pool, new_namespace)

    for el in memory_pool_el.children:
        if el.tag == 'memory':
            src  = getattr(el, 'src', None)
            base = getattr(el, 'base', None)
            size = getattr(el, 'size', None)
            
            pool.add_memory(src = src, base = base, size = size,
                            machine=machine, pools=pools)

    if not virtual and getattr(memory_pool_el, "direct", False):
        pool.set_direct(True)

        for base, end, mem_type in pool.pool.get_freelist():
            size = end - base + 1
            pools.add_direct_memory(base, size, mem_type)

def collect_memory_pools_elements(parsed, ignore_name, namespace,
                                  machine, bootinfo, pools):
    """Go through and handle all the iguana memory pools objects that we have."""

    # Now create the implicit direct pool.
    setup_direct_pool(namespace, machine, bootinfo, pools)

    for el in parsed.children:
        if el.tag == "virtual_pool":
            if not ignore_name.match(el.name):
                collect_memory_pool_el(el, namespace, machine,
                                       bootinfo, pools, virtual = True)

    for el in parsed.children:
        if el.tag == "physical_pool":
            if not ignore_name.match(el.name):
                collect_memory_pool_el(el, namespace, machine,
                                       bootinfo, pools, virtual = False)
