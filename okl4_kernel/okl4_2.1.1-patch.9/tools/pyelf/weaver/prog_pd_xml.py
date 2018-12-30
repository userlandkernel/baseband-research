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
Define os_bootinfo_el.
"""
from elf.core import PreparedElfFile, UnpreparedElfFile
from elf.constants import ET_EXEC, PF_R, PF_W
from weaver import MergeError
from weaver.ezxml import ParsedElement, Element, long_attr, bool_attr, str_attr
from weaver.segments_xml import collect_patches, collect_elf_segments, \
     attach_to_elf_flags, Segment_el, Heap_el, Patch_el, \
     start_to_value, make_pager_attr
from weaver.memobjs_xml import Stack_el, Memsection_el, \
     collect_memsection_element, create_standard_caps, create_alias_cap

import weaver.bootinfo
from weaver.bootinfo import AliasCap

Argv_el = Element("arg",
                  value = (str_attr, "required"))

CommandLine_el = Element("commandline", Argv_el)

Thread_el = Element("thread", Stack_el, CommandLine_el,
                    name     = (str_attr, "required"),
                    start    = (str_attr, "required"),
                    priority = (long_attr, "optional"),
                    physpool = (str_attr, "optional"),
                    virtpool = (str_attr, "optional"))

Entry_el = Element("entry",
                        key    = (str_attr, "required"),
                        value  = (long_attr, "optional"),
                        cap    = (str_attr, "optional"),
                        attach = (str_attr, "optional"))

Environment_el = Element("environment", Entry_el)

Zone_el = Element("zone", Segment_el, Memsection_el,
                  name     = (str_attr, "required"),
                  physpool = (str_attr, "optional"),
                  virtpool = (str_attr, "optional"))

VirtualDevice_el = Element("virt_device",
                           name = (str_attr, "required"))

Program_el = Element("program",
                     Segment_el, Patch_el, Stack_el, Heap_el,
                     CommandLine_el, Memsection_el, Thread_el,
                     Environment_el, Zone_el, VirtualDevice_el,
                     name     = (str_attr, "required"),
                     file     = (str_attr, "required"),
                     priority = (long_attr, "optional"),
                     physpool = (str_attr, "optional"),
                     virtpool = (str_attr, "optional"),
                     direct   = (bool_attr, "optional"),
                     pager    = (str_attr, "optional"),
                     server   = (str_attr, "optional"),
                     platform_control = (bool_attr, "optional"),
                     )

PD_el = Element("pd",
                Segment_el, Patch_el, Memsection_el, Thread_el,
                Environment_el, Zone_el,
                name     = (str_attr, "required"),
                file     = (str_attr, "optional"),
                physpool = (str_attr, "optional"),
                virtpool = (str_attr, "optional"),
                pager    = (str_attr, "optional"),
                direct   = (bool_attr, "optional"),
                platform_control = (bool_attr, "optional"),
                )


def collect_thread(elf, el, ignore_name, namespace, image, machine,
                   pools, entry, name = None,
                   namespace_thread_name = None):
    """Collect the attributes of a thread element."""
    if entry is None:
        raise MergeError, "No entry point specified for thread %s" % name

    # el can be a program element or a thread element.
    if name is None:
        name = el.name

    user_main = getattr(el, 'start', None)

    # HACK:  There is no easy way for a PD to specify the main thread,
    # so is the user entry point is '_start' (the traditional entry
    # point symbol), then start there rather then the thread entry
    # point.
    if user_main == "_start":
        start = "_start"
        user_main = None

    entry = start_to_value(entry, elf)
    user_main = start_to_value(user_main, elf)

    priority = getattr(el, 'priority', None)
    physpool = getattr(el, 'physpool', None)
    virtpool = getattr(el, 'virtpool', None)

    # New namespace for objects living in the thread.
    if namespace_thread_name is None:
        namespace_thread_name = name
    new_namespace = namespace.add_namespace(namespace_thread_name)

    # Push the overriding pools for the thread.
    image.push_attrs(virtual = virtpool,
                     physical = physpool)

    # Create the thread desription object.
    thread = weaver.bootinfo.Thread(name, entry, user_main, priority)

    # Add the standard caps for the thread.
    create_standard_caps(thread, new_namespace)

    # Collect the stack.  Is there no element, create a fake one for
    # the collection code to use.
    stack_el = el.find_child('stack')

    if stack_el is None:
        stack_el = ParsedElement('stack')

    stack_ms = collect_memsection_element(stack_el, ignore_name,
                                          new_namespace, image,
                                          machine, pools)
    thread.attach_stack(stack_ms)
    image.add_group(0, [stack_ms.get_ms()])

    # Collect any command line arguments.
    commandline_el = el.find_child('commandline')

    if commandline_el is not None:
        for arg_el in commandline_el.find_children("arg"):
            thread.add_argv(arg_el.value)

    image.pop_attrs()

    return thread

def collect_environment_element(env_el, namespace, machine, pools,
                                image, bootinfo):
    """Collect the object environment for a program or PD."""
    env = weaver.bootinfo.Environment(namespace)

    # Collect any custom entries in the environment.
    if env_el is not None:
        for entry_el in env_el.find_children('entry'):
            value       = None
            cap_name    = None
            attach      = None

            if hasattr(entry_el, 'value'):
                value = entry_el.value

                env.add_entry(entry_el.key, value = value,
                              cap_name = cap_name, attach = attach)
            else:
                if not hasattr(entry_el, 'cap'):
                    raise MergeError, 'Value or cap attribute required.'

                cap_name = entry_el.cap

                if hasattr(entry_el, 'attach'):
                    attach = attach_to_elf_flags(entry_el.attach)

                # Check if the cap is an AliasCap.  If it is, ask the cap object
                # to create any implied objects now
                cap = env.scope.lookup(cap_name)

                if cap is None:
                    raise MergeError, "Cap %s not found." % cap_name

                if isinstance(cap, AliasCap):
                    # Always add the AliasCap before any implied object
                    # caps.  Because we will use this alias cap to generate
                    # their bootinfo just in time - nt
                    env.add_entry(entry_el.key, value = value, cap = cap,
                                  cap_name = cap_name, attach = attach)

                    # Add implicit object caps to the environment
                    # These caps are different from the alias cap!
                    cap_list = \
                             cap.get_object().create_implicit_objects(namespace,
                                                                      machine,
                                                                      pools, image,
                                                                      bootinfo)
                    if cap_list is not None:
                        for key, cap in cap_list.iteritems():
                            env.add_entry(key, cap = cap, attach = PF_R | PF_W)

                else:
                    # If the cap is not an AliasCap we add it as before
                    env.add_entry(entry_el.key, value = value,
                                  cap_name = cap_name, attach = attach)

    return env

def collect_zone_element(zone_el, ignore_name, namespace, pools,
                         image, bootinfo, machine):
    """Collect the contents of a zone."""
    non_zone_ms = []
    env = weaver.bootinfo.Environment(namespace)

    # New namespace for objects living in the memsection.
    zone_namespace = namespace.add_namespace(zone_el.name)

    virtpool = getattr(zone_el, 'virtpool', None)
    physpool = getattr(zone_el, 'physpool', None)

    attrs = image.new_attrs(zone_namespace)
    attrs.name = zone_el.name

    zone = weaver.bootinfo.Zone(attrs, pools, image, machine)

    bootinfo.add_zone(zone)

    # Push the overriding attributes for the zone.
    image.push_attrs(virtual  = zone_el.name,
                     physical = physpool)

    master = weaver.bootinfo.Cap("master", ["master"])
    zone.add_cap(master)
    zone_namespace.add(master.get_name(), master)

    for ms_el in zone_el.find_children('memsection'):
        if not ignore_name.match(ms_el.name):
            ms = collect_memsection_element(ms_el, ignore_name,
                                            zone_namespace, image,
                                            machine, pools)

            # Memsections that aren't allocated in the zone belong to the PD.
            if ms.get_attrs().virtpool == zone_el.name:
                zone.add_memsection(ms)
                image.add_group(None, [ms.get_ms()])
            else:
                non_zone_ms.append(ms)

    # All of the segments have already been processed by the program/pd, but
    # record the ones probably should belong to the zone.  These will be added
    # to the zone by PD.add_zone()
    segment_els = zone_el.find_children("segment")
    zone.set_claimed_segments([segment_el.name for segment_el in segment_els])

    image.pop_attrs()

    return (zone, non_zone_ms)

def collect_program_element(program_el, ignore_name, namespace, image,
                            machine, bootinfo, pools):
    """Collect the attributes of a program element."""

    # New namespace for objects living in the program's PD.
    prog_namespace = namespace.add_namespace(program_el.name)

    pd = weaver.bootinfo.PD(program_el.name, prog_namespace, image,
                            machine, pools)

    elf = UnpreparedElfFile(filename=program_el.file)

    if elf.elf_type != ET_EXEC:
        raise MergeError, "All the merged ELF files must be of EXEC type."

    bootinfo.add_elf_info(name = program_el.file,
                          elf_type = image.PROGRAM,
                          entry_point = elf.entry_point)

    pd.elf = elf

    virtpool = getattr(program_el, 'virtpool', None)
    physpool = getattr(program_el, 'physpool', None)
    direct = getattr(program_el, 'direct', None)
    pd.set_platform_control(getattr(program_el, "platform_control", False))

    if hasattr(program_el, 'pager'):
        pager = make_pager_attr(program_el.pager)
    else:
        pager = None

    # Push the overriding attributes for the program.
    image.push_attrs(virtual  = virtpool,
                     physical = physpool,
                     pager    = pager,
                     direct   = direct)

    pd.set_default_pools(image, bootinfo)

    # Collect the object environment
    env = collect_environment_element(program_el.find_child('environment'),
                                      prog_namespace, machine, pools,
                                      image, bootinfo)

    segment_els = program_el.find_all_children("segment")
    segs = collect_elf_segments(elf,
                                image.PROGRAM,
                                segment_els,
                                program_el.name,
                                prog_namespace,
                                image,
                                machine,
                                pools)

    segs_ms = [bootinfo.record_segment_info(program_el.name, seg,
                                            image, machine, pools)
               for seg in segs]
    for seg_ms in segs_ms:
        pd.attach_memsection(seg_ms)

    # Record any patches being made to the program.
    patch_els   = program_el.find_children("patch")
    collect_patches(elf, patch_els, program_el.file, image)

    # Collect the main thread.
    thread = collect_thread(elf, program_el, ignore_name, prog_namespace,
                            image, machine, pools,
                            entry = elf.entry_point,
                            name = program_el.name,
                            namespace_thread_name = "main")
    pd.add_thread(thread)
    pd.set_server_thread(thread)

    # Add the virtual device elements
    # Virtual devices always get added to the global namespace because they
    # should be globally unique
    server_spawn_nvdevs = 0
    dev_ns = namespace.root.get_namespace("dev")

    if dev_ns is None:
        raise MergeError, "Device namespace does not exist!"

    for v_el in program_el.find_children('virt_device'):
        virt_dev = pd.add_virt_dev(v_el.name, program_el.name, pd,
                                   thread, server_spawn_nvdevs)
        create_alias_cap(virt_dev, dev_ns)
        server_spawn_nvdevs += 1

    # Record the main thread and its stack in the environment.
    env.add_entry(key      = "MAIN",
                  cap_name = 'main/master')
    env.add_entry(key      = "MAIN/STACK",
                  cap_name = 'main/stack/master',
                  attach  = thread.get_stack().get_attrs().attach)


    # If marked, sure that the program is exported to every
    # environment so that it can be found by other programs.
    #
    if hasattr(program_el, 'server'):
        bootinfo.add_server(
            key = program_el.server,
            cap_name = prog_namespace.abs_name('main') + '/stack/master')

    # Collect remaining threads.
    elf = pd.elf = pd.elf.prepare(image.wordsize, image.endianess)

    for thread_el in program_el.find_children('thread'):
        if not ignore_name.match(thread_el.name):
            thread = collect_thread(elf, thread_el, ignore_name,
                                    prog_namespace, image, machine, pools,
                                    entry = "_thread_start")
            pd.add_thread(thread)

            # Record the thread and its stack in the environment.
            env.add_entry(key      = thread.get_name(),
                          cap_name = thread.get_name() + '/master')
            env.add_entry(key      = thread.get_name() + "/STACK",
                          cap_name = thread.get_name() + '/stack/master',
                          attach   = thread.get_stack().get_attrs().attach)

    # Collect any other memsections in the program.
    for ms_el in program_el.find_children('memsection'):
        if not ignore_name.match(ms_el.name):
            ms = collect_memsection_element(ms_el, ignore_name,
                                            prog_namespace, image,
                                            machine, pools)

            pd.attach_memsection(ms)
            image.add_group(0, [ms.get_ms()])

            env.add_entry(key      = ms.get_name(),
                          cap_name = ms.get_name() + '/master',
                          attach   = ms.get_attrs().attach)

    # Collect any zones in the program.
    for zone_el in program_el.find_children('zone'):
        (zone, non_zone_ms) = \
               collect_zone_element(zone_el, ignore_name,
                                    prog_namespace, pools, image,
                                    bootinfo, machine)

        pd.attach_zone(zone)

        # Attach memsections that aren't part of the zone to the program.
        for ms in non_zone_ms:
            pd.attach_memsection(ms)

            image.add_group(0, [ms.get_ms()])
            env.add_entry(key      = ms.get_name(),
                          cap_name = ms.get_name() + '/master',
                          attach   = ms.get_attrs().attach)

    # Collect the heap.  Is there no element, create a fake one for
    # the collection code to use.
    heap_el = program_el.find_child('heap')

    if heap_el is None:
        heap_el = ParsedElement('heap')

    heap_ms = collect_memsection_element(heap_el, ignore_name,
                                         prog_namespace, image,
                                         machine, pools)
    pd.attach_heap(heap_ms)
    image.add_group(0, [heap_ms.get_ms()])
    # Fill env with default values.
    env.add_entry(key      = "HEAP",
                  cap_name = 'heap/master',
                  attach   = heap_ms.get_attrs().attach)

    env.add_entry(key      = "HEAP_BASE",
                  base     = heap_ms)
    env.add_entry(key      = "HEAP_SIZE",
                  value    = heap_ms.get_attrs().size)

    pd.add_environment(env)

    bootinfo.add_pd(pd)

    image.pop_attrs()


def collect_pd_element(pd_el, ignore_name, namespace, image, machine,
                       bootinfo, pools):
    """Collect the attributes of a PD element."""

    # New namespace for objects living in the PD.
    pd_namespace = namespace.add_namespace(pd_el.name)

    pd = weaver.bootinfo.PD(pd_el.name, pd_namespace, image, machine, pools)
    virtpool = getattr(pd_el, 'virtpool', None)
    physpool = getattr(pd_el, 'physpool', None)
    direct = getattr(pd_el, 'direct', None)
    pd.set_platform_control(getattr(pd_el, "platform_control", False))

    if hasattr(pd_el, 'pager'):
        pager = make_pager_attr(pd_el.pager)
    else:
        pager = None

    # Push the overriding attributes for the pd.
    image.push_attrs(virtual  = virtpool,
                     physical = physpool,
                     pager    = pager,
                     direct   = direct)

    pd.set_default_pools(image, bootinfo)

    # Collect the object environment
    env = collect_environment_element(pd_el.find_child('environment'),
                                      pd_namespace, machine, pools,
                                      image, bootinfo)

    if hasattr(pd_el, "file"):
        elf = PreparedElfFile(filename=pd_el.file)

        if elf.elf_type != ET_EXEC:
            raise MergeError, "All the merged ELF files must be of EXEC type."

        segment_els = pd_el.find_all_children("segment")
        segs = collect_elf_segments(elf,
                                    image.PROGRAM,
                                    segment_els,
                                    pd_el.name,
                                    pd_namespace,
                                    image,
                                    machine,
                                    pools)

        segs_ms = [bootinfo.record_segment_info(pd_el.name, seg,
                                                image, machine, pools)
                   for seg in segs]
        for seg_ms in segs_ms:
            pd.attach_memsection(seg_ms)

        # Record any patches being made to the program.
        patch_els   = pd_el.find_children("patch")
        collect_patches(elf, patch_els, image)

    # Collect threads in the PD.
    for thread_el in pd_el.find_children('thread'):
        if not ignore_name.match(thread_el.name):
            thread = collect_thread(elf, thread_el, ignore_name,
                                    pd_namespace, image, machine, pools,
                                    entry = "_thread_start")
            pd.add_thread(thread)

            # Record the thread and its stack in the environment.
            env.add_entry(key      = thread.get_name(),
                          cap_name = thread.get_name() + '/master')
            env.add_entry(key      = thread.get_name() + "/STACK",
                          cap_name = thread.get_name() + '/stack/master',
                          attach   = thread.get_stack().get_attrs().attach)

    # Collect memsections in the PD.
    for ms_el in pd_el.find_children('memsection'):
        if not ignore_name.match(ms_el.name):
            ms = collect_memsection_element(ms_el, ignore_name,
                                            pd_namespace, image,
                                            machine, pools)
            pd.attach_memsection(ms)

            image.add_group(0, [ms.get_ms()])

            env.add_entry(key      = ms.get_name(),
                          cap_name = ms.get_name() + '/master',
                          attach   = ms.get_attrs().attach)

    # Collect any zones in the program.
    for zone_el in pd_el.find_children('zone'):
        (zone, non_zone_ms) = \
               collect_zone_element(zone_el, ignore_name,
                                    pd_namespace, pools, image,
                                    bootinfo, machine)

        pd.attach_zone(zone)

        # Attach zones that aren't part of the zone to the program.
        for ms in non_zone_ms:
            pd.attach_memsection(ms)

            image.add_group(0, [ms.get_ms()])

            env.add_entry(key      = ms.get_name(),
                          cap_name = ms.get_name() + '/master',
                          attach   = ms.get_attrs().attach)

    pd.add_environment(env)

    bootinfo.add_pd(pd)
    image.pop_attrs()

def collect_program_pd_elements(parsed, ignore_name, namespace, image,
                                machine, bootinfo, pools):
    """Go through and handle all the iguana program objects that we have."""

    for el in parsed.children:
        if el.tag == "program":
            if not ignore_name.match(el.name):
                collect_program_element(el, ignore_name, namespace,
                                        image, machine, bootinfo, pools)
        elif el.tag == "pd":
            if not ignore_name.match(el.name):
                collect_pd_element(el, ignore_name, namespace, image,
                                   machine, bootinfo, pools)

