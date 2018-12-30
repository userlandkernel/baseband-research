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
Processing of the machine XML element.
"""

from weaver.ezxml import Element, long_attr, str_attr, size_attr
from weaver.kernel_elf import MEMDESC_DICT, MEMDESC_UNDEFINED, MEMDESC_RESERVED
from weaver import MergeError
from weaver.memobjs_xml import create_alias_cap
from weaver.kernel_xml import Kernel_el
from weaver.rootprogram_xml import RootProgram_el

Region_el = Element("region",
                    base = (long_attr, "required"),
                    size = (size_attr, "required"),
                    type = (str_attr, "optional"))

VirtualMemory_el = Element("virtual_memory",
                            Region_el,
                            name = (str_attr, "required"))

PhysicalMemory_el = Element("physical_memory",
                            Region_el,
                            name = (str_attr, "required"))

Interrupt_el      = Element("interrupt",
                            name = (str_attr, "required"),
                            number = (long_attr, "required"))

PhysicalDevice_el = Element("phys_device",
                            PhysicalMemory_el,
                            Interrupt_el,
                            name = (str_attr, "required"))

WordSize_el = Element("word_size",
                        size = (size_attr, "required"))

PageSize_el = Element("page_size",
                        size = (size_attr, "required"))

KernelHeapAttrs_el = Element("kernel_heap_attrs",
                             distance = (size_attr, "optional"),
                             align    = (size_attr, "optional"))
                             
CachePolicy_el = Element("cache_policy",
                         name  = (str_attr, "required"),
                         value = (long_attr, "required"))

Machine_el = Element("machine",
                     PhysicalDevice_el,
                     VirtualMemory_el,
                     PhysicalMemory_el,
                     WordSize_el,
                     PageSize_el,
                     KernelHeapAttrs_el,
                     CachePolicy_el,
                     file = (str_attr, "optional"))


def collect_machine_element(parsed, ignore_name, machine, namespace):
    """Collect the attributes of the machine element."""
    machine_el = parsed.find_child("machine")

    assert machine_el is not None

    # If the file attribute is specified, then read the full version
    # of the machine element from that file.
    if hasattr(machine_el, "file"):
        machine_el = Machine_el.parse_xml_file(machine_el.file)

    machine.word_size = machine_el.find_child("word_size").size

    machine.set_page_sizes([el.size for el in machine_el.find_children("page_size")])
    
    machine.add_cache_policies([(el.name, el.value) for el in machine_el.find_children("cache_policy")])
    
    attrs = machine_el.find_child("kernel_heap_attrs")
    if attrs is not None:
        machine.kernel_heap_proximity = getattr(attrs, 'distance', None)
        machine.kernel_heap_align = getattr(attrs, 'align', None)

    def map_mem_type(region_el):
        """Map the string memory type to a memory descriptor type."""
        mem_type = getattr(region_el, "type", "conventional")

        if not MEMDESC_DICT.has_key(mem_type):
            raise MergeError, "Unknown memory type %s" % mem_type

        return MEMDESC_DICT[mem_type]

    for p_el in machine_el.find_children("physical_memory"): 
        if not ignore_name.match(p_el.name):
            mem = [(el.base, el.size, map_mem_type(el))
                   for el in p_el.find_children("region")]
            machine.add_physical_mem(p_el.name, mem)

    for v_el in machine_el.find_children("virtual_memory"): 
        if not ignore_name.match(v_el.name):
            mem = [(el.base, el.size, MEMDESC_UNDEFINED)
                   for el in v_el.find_children("region")]
            machine.add_virtual_mem(v_el.name, mem)

    dev_ns = namespace.root.get_namespace("dev")

    if dev_ns is None:
        raise MergeError, "Device namespace does not exist!"

    for d_el in machine_el.find_children("phys_device"):
        if not ignore_name.match(d_el.name):
            device = machine.add_phys_device(d_el.name)
            create_alias_cap(device, dev_ns)
            for p_el in d_el.find_children("physical_memory"):
                if not ignore_name.match(p_el.name):
                    mem = [(el.base, el.size, MEMDESC_RESERVED)
                           for el in p_el.find_children("region")]
                    device.add_physical_mem(p_el.name, mem)
            for i_el in d_el.find_children("interrupt"):
                if not ignore_name.match(i_el.name):
                    device.add_interrupt(i_el.name, i_el.number)

    # extra_func is set by extensions, so pylint incorrectly thinks
    # that it is not callable.
    #pylint: disable-msg=E1102
    if Machine_el.extra_func is not None:
        Machine_el.extra_func(machine_el, Machine_el, Kernel_el, RootProgram_el)

