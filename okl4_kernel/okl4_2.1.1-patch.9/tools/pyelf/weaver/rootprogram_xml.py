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
Processing of the rootprogram XML tag.
"""
from elf.core import PreparedElfFile, UnpreparedElfFile
from elf.constants import ET_EXEC
from weaver import MergeError
from weaver.ezxml import ParsedElement, Element, bool_attr, str_attr
from weaver.segments_xml import collect_patches, collect_elf_segments, \
     Segment_el, Patch_el, Heap_el, start_to_value
from weaver.prog_pd_xml import collect_environment_element, \
     collect_thread, Environment_el 
from weaver.memobjs_xml import Stack_el, collect_memsection_element
import weaver.image
import weaver.bootinfo

Extension_el = Element("extension", Segment_el, Patch_el,
                       name     = (str_attr, "required"),
                       file     = (str_attr, "optional"),
                       start    = (str_attr, "optional"),
                       direct   = (bool_attr, "optional"),
                       pager    = (str_attr, "optional"),
                       physpool = (str_attr, "optional"))

RootProgram_el = Element("rootprogram", Segment_el, Patch_el,
                         Extension_el, Environment_el,
                         Stack_el, Heap_el,
                         file     = (str_attr, "required"),
                         physpool = (str_attr, "required"),
                         virtpool = (str_attr, "required"),
                         pager    = (str_attr, "optional"),
                         direct   = (bool_attr, "optional"))
def get_symbol(elf, symbol, may_not_exist = False):
    sym = elf.find_symbol(symbol)

    if not sym:
        if may_not_exist:
            return None
        else:
            print "warn: cannot find symbol ", symbol
            return None

    address = sym.get_value()
    bytes   = sym.get_size()

    if bytes == 0:
        bytes = elf.wordsize / 8

    return (address, bytes)

def collect_rootprogram_element(parsed, ignore_name, namespace, image, machine, bootinfo, pools):
    """Handle an Iguana Server Compound Object"""

    # Find the tag
    root_program_el = parsed.find_child("rootprogram")

    assert(root_program_el is not None)

    # New namespace for objects living in the root program's PD.
    rp_namespace = namespace.add_namespace('rootprogram')

    pd = weaver.bootinfo.RootServerPD('rootserver', rp_namespace,
                                      image, machine, pools)

    # Declare the default memory pools.
    def_virtpool = getattr(root_program_el, "virtpool", None)
    def_physpool = getattr(root_program_el, "physpool", None)
    def_pager    = getattr(root_program_el, "pager", None)
    def_direct   = getattr(root_program_el, "direct", None)

    bootinfo.set_system_default_attrs(def_virtpool,
                                      def_physpool,
                                      image,
                                      def_pager,
                                      def_direct)

    elf = UnpreparedElfFile(filename=root_program_el.file)

    if elf.elf_type != ET_EXEC:
        raise MergeError, "All the merged ELF files must be of EXEC type."

    # Record the entry point of the root program so that the kernel
    # can start it.
    image.kconfig.set_rootserver_entry(elf.entry_point)

    pd.set_default_pools(image, bootinfo)

    # Collect the object environment
    env = collect_environment_element(root_program_el.find_child('environment'),
                                      rp_namespace, machine, pools, image, bootinfo)

    segment_els = root_program_el.find_children("segment")
    collect_elf_segments(elf,
                         image.ROOT_PROGRAM,
                         segment_els,
                         root_program_el.file,
                         rp_namespace,
                         image,
                         machine,
                         pools)

    # Record any patches being made to the program.
    patch_els   = root_program_el.find_children("patch")
    for p in getattr(RootProgram_el, "extra_patches", []):
        addr = get_symbol(elf, p[0], True)
        if addr == None:
            continue
        addr = int(addr[0])+ int(p[1])
        new_patch = Patch_el(address=hex(addr), bytes=p[2], value=p[3])
        patch_els.append(new_patch)
    collect_patches(elf, patch_els, root_program_el.file, image)

    for extension_el in root_program_el.find_children("extension"):
        if not ignore_name.match(extension_el.name):
            collect_extension_element(extension_el,
                                      pd,
                                      rp_namespace,
                                      elf,
                                      image,
                                      machine,
                                      bootinfo,
                                      pools)

    # Collect the main thread.  The root program can only have one
    # thread, so this call chiefly is used to collect information
    # about the stack.
    #
    # The stack is not set up as a memsection, so it is not put in the
    # object environment.
    thread = collect_thread(elf, root_program_el, ignore_name, rp_namespace,
                            image, machine, pools,
                            entry = elf.entry_point,
                            name = 'rootprogram',
                            namespace_thread_name = "main")
    pd.add_thread(thread)

    # Collect the heap.  Is there no element, create a fake one for
    # the collection code to use.
    #
    # The heap is not set up as a memsection, so it is not put in the
    # object environment.

    heap_el = root_program_el.find_child('heap')

    if heap_el is None:
        heap_el = ParsedElement('heap')
    
    heap_ms = collect_memsection_element(heap_el, ignore_name,
                                         rp_namespace, image, machine,
                                         pools)
    pd.attach_heap(heap_ms)
    image.add_group(0, [heap_ms.get_ms()])

    pd.add_environment(env)
    bootinfo.add_rootserver_pd(pd)


def collect_extension_element(extension_el, pd, namespace, rp_elf, image, machine, bootinfo, pools):
    # New namespace for objects living in the extension.
    extn_namespace = namespace.add_namespace(extension_el.name)

    elf   = None
    start = None
    name  = None
    physpool = getattr(extension_el, 'physpool', None)
    pager    = getattr(extension_el, 'pager', None)
    direct   = getattr(extension_el, 'direct', None)

    # Push the overriding pools for the extension.
    image.push_attrs(physical = physpool,
                                  pager    = pager,
                                  direct   = direct)

    if hasattr(extension_el, "file"):
        elf = UnpreparedElfFile(filename=extension_el.file)

        if elf.elf_type != ET_EXEC:
            raise MergeError, "All the merged ELF files must be of EXEC type."

        segment_els = extension_el.find_children("segment")
        segs = collect_elf_segments(elf,
                                    image.EXTENSION,
                                    segment_els,
                                    extension_el.name,
                                    extn_namespace,
                                    image,
                                    machine,
                                    pools)

        segs_ms = [bootinfo.record_segment_info(extension_el.name,
                                                seg, image, machine, pools) for seg in segs]
        for seg_ms in segs_ms:
            pd.attach_memsection(seg_ms)

        # Record any patches being made to the program.
        patch_els   = extension_el.find_children("patch")
        collect_patches(elf, patch_els, extension_el.file, image)

        start = elf.entry_point
        name  = extension_el.file

    if hasattr(extension_el, "start"):
        start = extension_el.start
        name  = extension_el.name

        # If no file is supplied, look for symbols in the root
        # program.
        if elf is None:
            elf = rp_elf

    elf = elf.prepare(elf.wordsize, elf.endianess)
    start = start_to_value(start, elf)

    bootinfo.add_elf_info(name = name,
                          elf_type = image.EXTENSION,
                          entry_point = start)

    image.pop_attrs()
