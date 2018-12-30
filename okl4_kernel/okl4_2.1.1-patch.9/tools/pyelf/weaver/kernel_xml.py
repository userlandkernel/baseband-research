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

"""Collect data from the kernel element."""

from weaver import MergeError
from weaver.ezxml import Element, long_attr, bool_attr, str_attr
from weaver.segments_xml import collect_elf_segments, collect_patches, \
     Segment_el, Patch_el, Heap_el
from elf.core import UnpreparedElfFile

from elf.constants import ET_EXEC

DEFAULT_KERNEL_HEAP_SIZE = 4 * 1024 * 1024
DEFAULT_KERNEL_MAX_THREADS = 1024

Option_el = Element("option",
                    key   = (str_attr, "required"),
                    value = (long_attr, "required"))

Dynamic_el = Element("dynamic",
                    align   = (long_attr, "optional"),
                    max_threads = (long_attr, "optional"))

Config_el = Element("config", Option_el)

# Compound Elements
Kernel_el = Element("kernel", Segment_el, Patch_el, Heap_el,
                    Config_el, Dynamic_el,
                    file     = (str_attr, "required"),
                    xip      = (bool_attr, "optional"),
                    physpool = (str_attr, "optional"))

Rootserver_el = Element("rootserver", Segment_el,
                        filename = (str_attr, "required"))

def get_symbol(elf, symbol, may_not_exist = False):
    """
    Return the address, and size in bytes, of a symbol from the ELF
    file.

    If may_not_exist is true then failing to find the symbol is not a
    fatal error.
    """
    sym = elf.find_symbol(symbol)

    if not sym:
        if may_not_exist:
            return None
        else:
            raise MergeError, \
                  'Symbol "%s" not found in kernel ELF file.  ' \
                  'Needed for XIP support.' % \
                  (symbol)

    address = sym.value
    bytes   = sym.size

    if bytes == 0:
        bytes = elf.wordsize / 8

    return (address, bytes)
        
def get_tcb_size(elf, image):
    """
    Return the size, in bytes, of a TCB entry.

    This value is fetched from the kernel image.
    """
    (addr, size) = get_symbol(elf, "tcb_size")
    return image.get_value(addr, size, elf.endianess)

def collect_kernel_element(parsed, kernel_heap_size, namespace, image,
                           machine, pools):
    """Collect the attributes of the kernel element."""
    kernel_el = parsed.find_child("kernel")
    if kernel_el is None:
        return

    # New namespace for objects living in the kernel.
    kernel_namespace = namespace.add_namespace('kernel')

    # Will the kernel be run execute in place?
    do_xip = hasattr(kernel_el, 'xip') and kernel_el.xip

    physpool = None
    if hasattr(kernel_el, 'physpool'):
        physpool = kernel_el.physpool

    # Push the overriding physical pool for the kernel.
    image.push_attrs(physical = physpool)

    segment_els = kernel_el.find_children("segment")
    patch_els   = kernel_el.find_children("patch")

    elf = UnpreparedElfFile(filename=kernel_el.file)
    #elf = PreparedElfFile(filename=kernel_el.file)

    if elf.elf_type != ET_EXEC:
        raise MergeError, \
              "All the merged ELF files must be of EXEC type."

    image.set_kernel(elf)
    segs = collect_elf_segments(elf, image.KERNEL, segment_els,
                                'kernel', kernel_namespace, image,
                                machine, pools)

    elf = elf.prepare(elf.wordsize, elf.endianess)
    base_segment = None

    if do_xip:
        (addr, size) = get_symbol(elf, '__phys_addr_ram')
        image.patch(addr, size, segs[1])
        base_segment = segs[1]

        (addr, size) = get_symbol(elf, '__phys_addr_rom')
        image.patch(addr, size, segs[0])
    else:
        sdata = get_symbol(elf, '__phys_addr_ram', may_not_exist=True)

        if sdata is not None:
            (addr, size) = sdata
            image.patch(addr, size, segs[0])

        base_segment = segs[0]

    # The extra_patches attr may be added by a plugin.
    for patch in getattr(Kernel_el, "extra_patches", []):
        addr = get_symbol(elf, patch[0], True)
        if addr != None:
            addr = int(addr[0])+ int(patch[1])
            new_patch = Patch_el(address=hex(addr), bytes=patch[2],
                                 value=patch[3])
            patch_els.append(new_patch)

    collect_patches(elf, patch_els, kernel_el.file, image)

    dynamic_attrs      = image.new_attrs(None)
    dynamic_attrs.align = machine.kernel_heap_align
    max_threads = DEFAULT_KERNEL_MAX_THREADS

    dynamic_el = kernel_el.find_child("dynamic")
    if dynamic_el is not None:
        max_threads = getattr(dynamic_el, 'max_threads', max_threads)
        dynamic_attrs.align = getattr(dynamic_el, 'align',
                                      dynamic_attrs.align)

    dynamic_attrs.size = max_threads * get_tcb_size(elf, image)
    array = image.add_kernel_array(dynamic_attrs, pools)
    image.add_group(machine.kernel_heap_proximity, (base_segment, array))
    image.patch(get_symbol(elf, "tcb_array")[0], machine.word_size / 8, array)
    image.patch(get_symbol(elf, "num_tcbs")[0],
                machine.word_size / 8, max_threads)

    heap_attrs      = image.new_attrs(None)
    heap_attrs.size = DEFAULT_KERNEL_HEAP_SIZE
    heap_attrs.align = machine.kernel_heap_align

    heap_el = kernel_el.find_child("heap")
    if heap_el is not None:
        heap_attrs.phys_addr = getattr(heap_el, 'phys_addr',
                                       heap_attrs.phys_addr)
        heap_attrs.size      = getattr(heap_el, 'size', heap_attrs.size)
        heap_attrs.align     = getattr(heap_el, 'align', heap_attrs.align)

    # Override the size with the command line value, if present.
    if kernel_heap_size != 0:
        heap_attrs.size = kernel_heap_size

    heap = image.set_kernel_heap(heap_attrs, pools)

    image.add_group(machine.kernel_heap_proximity, (base_segment, heap))

    config_el = kernel_el.find_child("config")

    if config_el is not None:
        for option in config_el.find_children("option"):
            image.kconfig.add_config(option.key, option.value)

    image.pop_attrs()
