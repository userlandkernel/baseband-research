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

"""The merge command is the main purpose of elf weaver. It provides a
way for merging many ELF files into one for the purpose of creating a
bootable image.

The merge_cmd function is the input from weaver.main. It parses command
line arguments and calls the merge() function which does most of the work.

The merge() command can also be called from another python library.
"""
import sys
from optparse import OptionParser
from weaver import MergeError
from weaver.parse_spec import parse_spec_xml, parse_spec_xml_string
from weaver.ezxml import EzXMLError, size_attr
from weaver.pools_xml import collect_memory_pools_elements
from weaver.rootprogram_xml import collect_rootprogram_element
from weaver.prog_pd_xml import collect_program_pd_elements
from weaver.kernel_xml import collect_kernel_element
from weaver.machine_xml import collect_machine_element
import weaver.image
import weaver.bootinfo
import weaver.pools
import weaver.namespace
import weaver.machine
import re


def collect_image_objects(parsed, ignore_name, kernel_heap_size,
                          namespace, image, machine, pools, bootinfo):
    """
    Extract the information in the XML elements and shove them into
    various data structures for later memory layout and image
    processing.
    """
    collect_machine_element(parsed, ignore_name, machine, namespace)

    collect_memory_pools_elements(parsed, ignore_name, namespace,
                                  machine, bootinfo, pools)
    collect_kernel_element(parsed, kernel_heap_size, namespace, image,
                           machine, pools)
    collect_rootprogram_element(parsed, ignore_name, namespace, image,
                                machine, bootinfo, pools)
    collect_program_pd_elements(parsed, ignore_name, namespace, image,
                                machine, bootinfo, pools)

def merge(spec_file, options):
    """Based on a given spec file, process it and merge into
    the output_file."""
    pools    = weaver.pools.Pools()
    machine  = weaver.machine.Machine()
    bootinfo = weaver.bootinfo.BootInfo()
    image    = weaver.image.Image(options.program_header_offset)

    namespace = weaver.namespace.ObjectNameSpace(None)
    device_namespace = namespace.add_namespace("dev")

    # parsed is an in memory tree of Element Objects
    # which is created from spec_file
    if options.spec_is_string:
        parsed = parse_spec_xml_string(spec_file)
    else:
        parsed = parse_spec_xml(spec_file)

    collect_image_objects(parsed, options.ignore_name,
                          options.kernel_heap_size,
                          namespace, image, machine, pools, bootinfo)
    bootinfo.create_dynamic_segments(namespace, image, machine, pools, bootinfo)
    image.layout(machine, pools)

    if options.remove_section_headers:
        image.remove_section_headers()


    # The ELF file must be prepared to calculate the offset value of
    # the segment info records in the environment.
    image.prepare(machine)

    bootinfo.generate(image, machine, bootinfo)

    image.apply_patches()
    image.write_out_image(options.output_file, machine)

    # If wanted, print out the final tree.
    if options.dump_layout:
        image.dump()

    if options.last_phys:
        pools.print_last_phys()

def merge_cmd(args):
    """Merge command call from main. This parses command line
    arguments and calls merge, which does all the main work."""
    parser = OptionParser("%prog merge [options] specfile", add_help_option=0)
    parser.add_option("-H", "--help", action="help")
    parser.add_option("-o", "--output", dest="output_file", metavar="FILE",
                      help="Destination filename")
    parser.add_option("-l", "--lastphys", dest="last_phys",
                      action="store_true", default=False,
                      help="After merging, print the next available " \
                      "physical address in each pool.") 
    parser.add_option('-k', "--kernel-heap-size", dest="kernel_heap_size",
                      action="store", default="0x0L",
                      help="Specify the size of the kernel heap, " \
                      "overridding the value in the specfile.") 
    parser.add_option('-i', "--ignore", dest="ignore_name",
                      action="store", default="^$",
                      help="A regex specifying programs to be ignored.")
    parser.add_option('-S', "--string", dest="spec_is_string",
                      action="store_true",
                      help="Treat the specfile argument as an XML string.")
    parser.add_option('-m', "--map", dest="dump_layout",
                      action="store_true",
                      help="Dump a memory map of the final image.")
    parser.add_option('-p', "--program-header-offset",
                      dest="program_header_offset",
                      action="store", type="long", default=None,
                      help="Set the program header offset in the ELF file.")
    parser.add_option('-y', "--verify", dest="verify",
                      action="store_true", default=False,
                      help="Verify that the specfile conforms to the DTD.")
    parser.add_option("--no-section-headers", dest="remove_section_headers",
                      action="store_true",
                      help="Remove section headers from the final image")

    (options, args) = parser.parse_args(args)

    options.kernel_heap_size = size_attr(options.kernel_heap_size)
    options.ignore_name = re.compile(options.ignore_name)

    if options.output_file is None and not options.last_phys: 
        parser.error("Output must be specified")
    if len(args) != 1:
        parser.error("Expecting a spec file.")
    try:
        # During elfweaver-extensions development, verify the document.
        if options.verify and not options.spec_is_string:
            import os
            xmllint = os.system('xmllint --path "tools/pyelf/dtd ../../dtd ../dtd" -o /dev/null --valid %s' % args[0])
            if xmllint != 0:
                return xmllint

        spec_file = args[0]
        merge(spec_file, options)
    except EzXMLError, text:
        print >> sys.stderr, text
        sys.exit(1)
    except MergeError, text:
        print >> sys.stderr, 'Error: %s' % text
        sys.exit(1)

    return 0
