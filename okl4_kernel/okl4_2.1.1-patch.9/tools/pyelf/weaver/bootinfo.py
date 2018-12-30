
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

"""Generate bootinfo operations for the image."""

from StringIO import StringIO
from os.path import basename
from elf.ByteArray import ByteArray
from elf.constants import PF_R
from elf.util import align_up, align_down
from weaver import MergeError
import weaver.machine
import weaver.image
import weaver.pools
import weaver.bootinfo_elf
from weaver.device import AliasCapObject, VirtualDevice

# Default values for iguana programs.  These can be overridden by
# attributes in the XML file.
DEFAULT_STACK_SIZE    = 0x1000
DEFAULT_HEAP_SIZE     = 0x10000
DEFAULT_CALLBACK_SIZE = 0x1000
DEFAULT_PRIORITY      = 100
DEFAULT_ENV_MS_SIZE   = 0x1000

# A guess at the number of extra ADD_*_MEM ops needed to hold the
# free-list.
BOOTINFO_GUESS_OPS    = 20

class BootInfoObject:
    """Common interfaces for bootinfo objects."""

    def __init__(self):
        self.caps = {}
        self.bi_name = None
        self.is_implicit = False

    def set_implicit(self):
        self.is_implicit = True
        
    def unset_implicit(self):
        self.is_implicit = False

    def add_cap(self, cap):
        """Add a cap to to the object."""
        cap.set_object(self)
        self.caps[cap.get_name()] = cap

    def get_cap(self, name):
        """Return the named cap."""
        return self.caps[name]

    def get_export_type(self):
        raise 'Unimplemented.'

    def get_bootinfo_id(self):
        return self.bi_name

    def need_attach(self):
        return False

class Cap(BootInfoObject):
    """Description of a cap and the rights that cap holds."""

    rights = {
        "read":    weaver.bootinfo_elf.BI_CAP_READ,
        "write":   weaver.bootinfo_elf.BI_CAP_WRITE,
        "execute": weaver.bootinfo_elf.BI_CAP_EXECUTE,
        "master":  weaver.bootinfo_elf.BI_CAP_MASTER,
        }                     

    def __init__(self, name, rights=None):
        BootInfoObject.__init__(self)

        self.name       = name
        self.the_object = None
        self.rights     = []
        self.all_rights = 0

        # Add the default rights.
        if rights is not None:
            for right in rights:
                self.add_right(right)

    def set_object(self, the_object):
        """Set the object what the cap belongs to."""
        self.the_object = the_object

    def get_object(self):
        """Return the object that this Cap refers to."""
        return self.the_object

    def get_name(self):
        """Return the name of the cap."""
        return self.name

    def add_right(self, right):
        """Add a right to the cap."""
        # Need some error checking here.

        if not Cap.rights.has_key(right):
            raise MergeError, "'%s' not a supported right." % right

        self.rights.append(right)
        self.all_rights = self.all_rights | Cap.rights[right]

    def generate_bootinfo(self, obj, bi):
        """Generate the bootinfo instructions to create the cap."""
        self.bi_name = \
                     bi.write_new_cap(obj.get_bootinfo_id(),
                                      self.all_rights)

class AliasCap(Cap):
    """Description of an alias cap."""

    def __init__(self, name, object):
        Cap.__init__(self, name, None)

        self.name       = name
        self.the_object = object

    def add_right(self, right):
        "This doesn't make sense for an AliasCap, throw a warning."
        print "Warning: trying to add %s rights to alias cap %s" % \
              (right, self.name)

    def generate_bootinfo(self, bi):
        """
        Generate the bootinfo instructions to create the cap.

        Since this is an alias cap we defer to the object it is pointing to.
        """
        raise "Error: generate_bootinfo() should not be called on an " \
              "AliasCap (%s)" % self.name 

class Environment:
    """
    Description of a PD's Object Environment.

    Entries are placed in the environment with their caps referred to
    by name.  After the XML file has been processed, the  snap()
    method must be called to make the environment point to the cap
    objects.  This two step process is required because the
    environment may refer to caps that are in PDs that are yet to be
    processed.
    """

    class Entry:
        """An entry in the Object Environment."""
        def __init__(self, key, value, base, cap_name, attach, cap = None):
            self.key      = key
            self.value    = value
            self.base     = base
            self.cap_name = cap_name
            self.cap      = cap
            self.attach   = attach

    class ELFEntry:
        """An Entry for ELF file info."""
        def __init__(self, key, elf_type, entry_point):
            self.key         = key
            self.elf_type    = elf_type
            self.entry_point = entry_point

    class SegmentEntry:
        """An Entry for segment info."""
        def __init__(self, key, segment):
            self.key     = key
            self.segment = segment

    def __init__(self, scope):
        """
        Create a new object environment.  The scope paramater is an
        ObjectNamespace that is used to resolve cap names and acts as
        the 'current directory' for relative names.
        """
        self.scope = scope
        self.entries = []

    def calc_size(self, machine):
        WORD_IN_BYTES  = machine.word_size / 8
        HDR_SIZE       = 2 * WORD_IN_BYTES
        ENV_ITEM_SIZE  = 8 * WORD_IN_BYTES
        ZERO_BYTE      = 1
        # Entries added by iguana_server.
        internal_entries = ("OKL4_DEFAULT_CLIST")

        # Calculate the size of the keys.
        str_len = reduce((lambda sz, x: sz + len(x.key) + ZERO_BYTE),
                         self.entries, 0)

        internal_str_len = reduce((lambda sz, x: sz + len(x) + ZERO_BYTE),
                                  internal_entries, 0)

        # Return the total size
        return (HDR_SIZE +
                (len(self.entries) + len(internal_entries)) * ENV_ITEM_SIZE +
                str_len + internal_str_len)

    def add_entry(self, key, value = None, base = None, cap = None,
                  cap_name = None, attach=None):
        """Add a new entry to the environment."""
        entry = Environment.Entry(key, value = value,
                                  base = base,
                                  cap_name = cap_name,
                                  cap = cap,
                                  attach = attach)

        self.entries.append(entry)

    def add_elf_entry(self, key, elf_type, entry_point):
        """Add a new ELF info entry to the environment."""
        entry = Environment.ELFEntry(key, elf_type, entry_point)
        self.entries.append(entry)

    def add_segment_entry(self, key, segment):
        """Add a new segement info entry to the environment."""
        entry = Environment.SegmentEntry(key, segment)
        self.entries.append(entry)

    def snap(self):
        """Snap the cap names pointers in the environment into cap
        objects."""
        for e in self.entries:
            if isinstance(e, Environment.Entry) and e.cap is None and \
                   e.cap_name is not None:
                cap = self.scope.lookup(e.cap_name)

                if cap is None:
                    # Needs more context.
                    raise MergeError, "Cap %s not found." % e.cap_name
                else:
                    e.cap = cap

    def generate_bootinfo(self, pd, bi):
        """
        Generate the bootinfo instructions to create and fill the
        environment.
        """
        pd_bi = pd.get_bootinfo_id()

        self.snap()

        for e in self.entries:
            if isinstance(e, Environment.Entry):
                if e.value is not None:
                    bi.write_object_export(pd   = pd_bi,
                                           key  = e.key.upper(),
                                           obj  = e.value,
                                           type = weaver.bootinfo_elf.BI_EXPORT_CONST)
                elif e.base is not None:
                    bi.write_object_export(pd   = pd_bi,
                                           key  = e.key.upper(),
                                           obj  = e.base.get_bootinfo_id(),
                                           type = weaver.bootinfo_elf.BI_EXPORT_BASE)
                else:
                    cap = e.cap
                    cap_object = cap.get_object()

                    if (isinstance(cap_object, AliasCapObject)):
                        pass
                    else:
                        bi.write_grant_cap(pd = pd_bi,
                                           cap = cap.get_bootinfo_id())

                        # If exporting a memsection, attach it appropriately
                        if (cap_object.get_export_type() in 
                            (weaver.bootinfo_elf.BI_EXPORT_MEMSECTION_CAP,
                             weaver.bootinfo_elf.BI_EXPORT_ZONE_CAP)):
                            assert e.attach is not None
                            if cap_object.need_attach():
                                bi.write_attach(pd = pd_bi,
                                                ms = cap_object.get_bootinfo_id(),
                                                rights = e.attach)

                        bi.write_object_export(pd   = pd_bi,
                                               key  = e.key.upper(),
                                               obj  = cap.get_bootinfo_id(),
                                               type = cap_object.get_export_type())

            elif isinstance(e, Environment.ELFEntry):
                bi.write_struct_export(pd     = pd_bi,
                                       key    = e.key.upper(),
                                       first  = e.elf_type,
                                       second = e.entry_point,
                                       type   =
                                       weaver.bootinfo_elf.BI_EXPORT_ELF_FILE)
            elif isinstance(e, Environment.SegmentEntry):
                # This can be called with the ELF file in one of two
                # states.  If the file is not prepared, then we can't
                # know the offset, but this only occurs when we're
                # calculating the size of the bootinfo memsection, so
                # the values don't matter.  If the ELF file is
                # prepared, then the offset and size can be fetched
                # from the prepared segment.
                seg    = e.segment
                offset = 0
                paddr  = 0

                if seg.prepared:
                    offset = seg.offset
                    paddr  = seg.paddr

                bi.write_struct_export(pd     = pd_bi,
                                       key    = e.key.upper(),
                                       first  = seg.vaddr,
                                       second = seg.flags,
                                       third  = paddr,
                                       fourth = offset,
                                       fifth  = seg.get_filesz(),
                                       sixth  = seg.get_memsz(),
                                       type   = weaver.bootinfo_elf.BI_EXPORT_ELF_SEGMENT)
            else:
                raise "Unexpected object in environment: %s" % e

class MemoryPool(BootInfoObject):
    def __init__(self, name):
        BootInfoObject.__init__(self)

        self.name = name
        self.pool = None

    def get_name(self):
        return self.name

    def is_virtual(self):
        raise "Unimplemented."

    def set_direct(self, direct):
        self.pool.set_direct(direct)

    def set_bootinfo_id(self, bi_name):
        self.bi_name = bi_name

    def generate_bootinfo(self, bi, skip_if_implicit = False):
        """
        Generate the bootinfo instructions to create and fill the
        memory pool.
        """
        self.bi_name = \
                     bi.write_new_pool(self.is_virtual())

        for cap in self.caps.values():
            cap.generate_bootinfo(self, bi)

        for (base, end, mem_type) in self.pool.get_freelist():
            if self.is_virtual():
                bi.write_add_virt_mem(self.bi_name, base, end)
            else:
                bi.write_add_phys_mem(self.bi_name, base, end)

class PhysPool(MemoryPool):
    def __init__(self, name, machine, pools):
        MemoryPool.__init__(self, name)

        self.pool = pools.new_physical_pool(name, machine)

    def add_memory(self, src, base, size, machine, pools):
        """Add memory to the pool."""
        pools.add_physical_memory(self.name, machine,
                                               src, base, size)

    def is_virtual(self):
        return False

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_PHYSPOOL_CAP

class VirtPool(MemoryPool):
    def __init__(self, name, machine, pools):
        MemoryPool.__init__(self, name)

        self.pool = pools.new_virtual_pool(name, machine)

    def add_memory(self, src, base, size, machine, pools):
        """Add memory to the pool."""
        pools.add_virtual_memory(self.name, machine,
                                              src, base, size)
    def is_virtual(self):
        return True

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_VIRTPOOL_CAP


class Thread(BootInfoObject):
    def __init__(self, name, start, user_main, priority = None):
        BootInfoObject.__init__(self)

        self.name      = name
        self.start     = start
        self.args      = []
        self.caps      = {}
        self.stack_ms  = None
        self.owner     = None
        self.spawn_vdevs = 0

        if user_main is None:
            self.user_main = 0
        else:
            self.user_main = user_main

        if priority is None:
            self.priority = DEFAULT_PRIORITY
        else:
            self.priority = priority

    def set_owner(self, pd):
        self.owner   = pd

        pd.attach_memsection(self.stack_ms)

    def get_priority(self):
        return self.priority

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_THREAD_CAP

    def add_argv(self, arg):
        self.args.append(arg)

    def attach_stack(self, stack_ms):
        """Link a thread to it's stack."""

        if self.stack_ms is not None:
            raise MergeError, "Thread %s already has a stack." % self.name

        # Set the defaults for the stack if they have not yet been
        # overridden.
        attrs = stack_ms.get_attrs()

        attrs.file = None
        attrs.pager = None # Iguana writes to the stack.

        if attrs.name is None:
            attrs.name = "stack"

        if attrs.size is None:
            attrs.size = DEFAULT_STACK_SIZE

        self.stack_ms = stack_ms

    def get_stack(self):
        return self.stack_ms

    def get_name(self):
        return self.name

    def generate_create_thread(self, bi):
        n = bi.write_new_thread(owner     = self.owner.get_bootinfo_id(),
                                ip        = self.start,
                                user_main = self.user_main,
                                priority  = self.priority,
                                name      = self.name)
        self.bi_name = n

    def generate_bootinfo(self, bi, heap, callback):
        """Generate the bootinfo instructions to create the thread."""
        # Stack.

        bi.write_register_stack(thread = self.bi_name,
                                ms     = self.stack_ms.get_bootinfo_id())

        # Dump the caps.
        for cap in self.caps.values():
            cap.generate_bootinfo(self, bi)
    
        # Write out the command line arguments.
        for arg in self.args:
            bi.write_argv(self.bi_name, arg)

    def generate_run_thread(self, bi):
        bi.write_run_thread(self.bi_name)

class MemSection(BootInfoObject):
    """Bootinfo related data for a memsection."""

    def __init__(self, image, machine, pools, attrs = None, segment = None):
        """
        Initialise the memsection with either the given attributes or
        the attributes of the given segment.

        If a segment is passed, then this memsection covers the range
        of the segment.
        """
        BootInfoObject.__init__(self)

        self.owner = None
        self.zone  = None
        self.vbase = 0

        if segment is not None:
            self.ms = segment
        else:
            self.ms = image.add_memsection(attrs, machine, pools)

    def get_ms(self):
        return self.ms

    def set_owner(self, owner):
        """Set the owning PD of the memsection."""
        self.owner = owner

    def set_zone(self, zone):
        """Set the owning zone of the memsection."""
        self.zone = zone

    def get_zone(self):
        """
        Return the owning zone of the memsection, or None if the
        memsection is not in a zone.
        """
        return self.zone

    def get_image_ms(self):
        return self.ms

    def get_name(self):
        """Return the name of the memsection."""
        return self.ms.get_attrs().name

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_MEMSECTION_CAP

    def get_attrs(self):
        """Return the attributes of the memsection."""
        return self.ms.get_attrs()

    def need_attach(self):
        """
        Return whether or not the memsection needs to be attached
        to a PD.

        Memsections with custom pagers, and memsections in zones, to
        not need to be attached.
        """
        return self.zone is None and self.get_attrs().pager is None

    def generate_bootinfo(self, bi, machine, bootinfo, skip_if_implicit = False):
        """
        Generate the bootinfo operations to setup the memsection.
        """
        # If bootinfo has already been written implicitly, skip this step
        if self.is_implicit and skip_if_implicit:
            return

        attrs = self.get_attrs()

        # Set defaults for values calculated from attributes generated
        # by layout().  This method can be called from
        # BootInfo.create_dynamic_segments(), which called prior to
        # Image.layout() in which case addresses and default pools may
        # not be known.  Consequently it doesn't really matter what
        # default values are used because the bootinfo ops will be
        # thrown away once the total size is calculated.
        vbase       = 0
        pbase       = 0
        size        = 0
        physpool_id = 0
        virtpool_id = 0

        # Calculate the ids of the memsections's pools.

        if attrs.direct:
            virtpool_id = \
                          bootinfo.get_virtpool('direct').get_bootinfo_id()
        elif attrs.virtpool is not None:
            virtpool_id = \
                          bootinfo.get_virtpool(attrs.virtpool).get_bootinfo_id()
        
        if attrs.physpool is not None:
            physpool_id = \
                        bootinfo.get_physpool(attrs.physpool).get_bootinfo_id()
        
        # Align the addresses to page boundaries.  The pool allocators
        # will have taken this into account, but kept non-aligned
        # values to be compatible with the ELF contents.
        if attrs.phys_addr is not None:
            pbase = align_down(attrs.phys_addr,
                               machine.min_page_size())
        if attrs.virt_addr is not None:
            vbase = align_down(attrs.virt_addr,
                               machine.min_page_size())
            size  = align_up(attrs.size + (attrs.virt_addr - vbase),
                             machine.min_page_size())

        flags = (weaver.bootinfo_elf.BI_MEM_USER |
                 weaver.bootinfo_elf.BI_MEM_FIXED)

        if attrs.pager == "memload":
            flags |= weaver.bootinfo_elf.BI_MEM_LOAD

        if attrs.protected:
            flags |= weaver.bootinfo_elf.BI_MEM_PROTECTED

        # Memsections in zones are initialised differently to
        # memsections in PDs.
        if self.zone is not None:
            self.bi_name = \
                         bi.write_new_ms(owner    = self.owner.get_bootinfo_id(),
                                         base     = vbase,
                                         size     = size,
                                         flags    = flags,
                                         attr     = attrs.cache_policy,
                                         physpool = physpool_id,
                                         virtpool = None,
                                         zone     = self.zone.get_bootinfo_id())
            for cap in self.caps.values():
                cap.generate_bootinfo(self, bi)

        else:
            self.bi_name = \
                         bi.write_new_ms(owner    = self.owner.get_bootinfo_id(),
                                         base     = vbase,
                                         size     = size,
                                         flags    = flags,
                                         attr     = attrs.cache_policy,
                                         physpool = physpool_id,
                                         virtpool = virtpool_id,
                                         zone     = None)

            for cap in self.caps.values():
                cap.generate_bootinfo(self, bi)

            if self.need_attach():
                bi.write_attach(pd = self.owner.get_bootinfo_id(),
                                ms = self.bi_name,
                                rights = self.ms.attrs.attach)

            bi.write_grant_cap(pd = self.owner.get_bootinfo_id(),
                               cap = self.caps['master'].get_bootinfo_id())

        # Common operations.
    
        bi.write_map(vaddr = vbase,
                     size  = size,
                     paddr = pbase,
                     scrub = self.get_attrs().should_scrub(),
                     mode  = self.get_attrs().cache_policy)

        # Save the virtual base
        self.vbase = vbase
                     
class Zone(VirtPool):
    """A collection of memsections shared by PDs."""
    def __init__(self, attrs, pools, image, machine):
        # Do *not* call VirtPool.__init__, otherwise a pool and a zone
        # will be created.
        MemoryPool.__init__(self, attrs.name)

        self.pool       = pools.new_zone(attrs.name, machine)
        self.image_zone = image.add_zone(attrs, self.pool)

        self.memsections = []
        self.attach      = 0
        self.claimed_segments = []

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_ZONE_CAP

    def get_attrs(self):
        """Return the attributes of the zone."""
        return self.image_zone.get_attrs()

    def get_name(self):
        return self.get_attrs().name

    def set_claimed_segments(self, segments):
        self.claimed_segments = segments

    def get_claimed_segments(self):
        return self.claimed_segments

    def add_memsection(self, ms):
        ms.set_zone(self)
        self.memsections.append(ms)
        self.image_zone.add_memsection(ms.get_image_ms())

        # A zone is attached with the most permissive access rights of
        # all of the memsections within the zone.
        self.attach |= ms.get_attrs().attach

    def get_attach(self):
        return self.attach

    def get_memsections(self):
        return self.memsections

    def need_attach(self):
        return True

    def generate_bootinfo(self, bi, machine, bootinfo):
        attrs = self.get_attrs()

        if attrs.virtpool is not None:
            virtpool_id = \
                          bootinfo.get_virtpool(attrs.virtpool).get_bootinfo_id()
        else:
            virtpool_id = 0

        self.bi_name = bi.write_new_zone(virtpool_id)

        for cap in self.caps.values():
            cap.generate_bootinfo(self, bi)
    
        for (window_base, window_end) in self.pool.get_windows():
            bi.write_add_zone_window(self.bi_name, window_base)

        for (base, end, mem_type) in self.pool.get_freelist():
            bi.write_add_virt_mem(self.bi_name, base, end)

        for ms in self.memsections:
            ms.generate_bootinfo(bi, machine, bootinfo)

class PD(BootInfoObject):
    """Description of a PD."""
    ROOTSERVER_PD_BI_NAME = 0

    def __init__(self, name, namespace, image, machine, pools, owner =
                 None, is_rootserver_pd = False):
        BootInfoObject.__init__(self)

        self.name        = name
        self.owner       = owner
        self.threads     = []
        self.memsections = []
        self.segments    = []
        self.heap_ms     = None
        self.environment = None
        self.added_std_env = False
        self.default_virtpool = None
        self.default_physpool = None
        self.default_directpool = None
        self.server_thread = None
        self.zones         = []
        self.virtual_devs = {}
        self.pd_namespace = namespace
        self.elf          = None
        self.user_map_ms  = None
        self.platform_ctrl = False

        env_ms_attrs       = image.new_attrs(namespace)
        env_ms_attrs.name  = "environment"
        env_ms_attrs.size  = DEFAULT_ENV_MS_SIZE
        env_ms_attrs.pager = None

        self.env_ms = MemSection(image, machine, pools, env_ms_attrs)
        # Create the master cap for the env_ms memsection.
        master = Cap("master", ["master"])
        self.env_ms.add_cap(master)

        self.attach_memsection(self.env_ms)
        image.add_group(None, [self.env_ms.get_ms()])

        if not is_rootserver_pd:
            callback_attrs      = image.new_attrs(namespace)
            callback_attrs.name = "callback"
            callback_attrs.size = DEFAULT_CALLBACK_SIZE
            callback_attrs.pager = None

            self.callback = MemSection(image, machine, pools,
                                       attrs = callback_attrs) 
            # Create the master cap for the callback memsection.
            master = Cap("master", ["master"])
            self.callback.add_cap(master)

            self.attach_memsection(self.callback)
            image.add_group(None, [self.callback.get_ms()])

    def set_default_pools(self, image, bootinfo):
        """
        Record the current default pools so that they can be passed
        on to the PD's runtime allocators.
        """
        (virt_name, phys_name) = image.current_pools()
        self.default_virtpool  = bootinfo.get_virtpool(virt_name)
        self.default_physpool  = bootinfo.get_physpool(phys_name)
        self.default_directpool = bootinfo.get_virtpool("direct")

    def get_default_pools(self):
        """
        Get the default allocation pools for this PD.
        """
        return (self.default_virtpool, self.default_physpool,
                self.default_directpool)

    def set_platform_control(self, control):
        """Set whether or not the PD can call L4_PlatformControl."""
        self.platform_ctrl = control

    def add_environment(self, env):
        """Set the PD's object environment."""
        self.environment = env

    def add_thread(self, thread):
        thread.set_owner(self)
        self.threads.append(thread)

    def add_virt_dev(self, name, server_name, server_pd, server_thread, index):
        """Add a named virtual device."""
        # to be implemented
        # create vdev object and cap
        self.virtual_devs[name] = VirtualDevice(name, server_name,
                                                server_pd,
                                                server_thread, index)
        return self.virtual_devs[name]

    def set_server_thread(self, thread):
        self.server_thread = thread

    def get_threads(self):
        """Return a list of threads in the PD."""
        return self.threads

    def attach_heap(self, heap_ms):
        if self.heap_ms is not None:
            raise MergeError, "PD %s already has a heap." % self.name

        # Set the defaults for the heap if they have not yet been
        # overridden.
        attrs = heap_ms.get_attrs()

        attrs.file = None

        if attrs.name is None:
            attrs.name = "heap"

        if attrs.size is None:
            attrs.size = DEFAULT_HEAP_SIZE

        self.heap_ms = heap_ms
        self.attach_memsection(self.heap_ms)

    def attach_memsection(self, ms):
        ms.set_owner(self)
#        assert ms.ms.attrs.name is not None
        self.memsections.append(ms)
        if ms.get_attrs().usermap:
            self.user_map_ms = ms

    def detach_memsection(self, ms):
        del self.memsections[self.memsections.index(ms)]

    def attach_zone(self, zone):
        self.zones.append(zone)

        claimed_ms = zone.get_claimed_segments()

        # Move memsections claimed by the zone into the zone.
        for ms in [ms for ms in self.memsections
                   if ms.get_name() in claimed_ms]:
            self.detach_memsection(ms)
            ms.get_attrs().virtpool = zone.get_name()
            zone.add_memsection(ms)

        for ms in zone.get_memsections():
            ms.set_owner(self)

    def _generate_bi_name(self, bi, owner):
        """Return the bootinfo name of the PD."""
        return  bi.write_new_pd(owner)
        
    def generate_contents_bootinfo(self, bi, default_virtpool, default_physpool, default_directpool, machine, bootinfo):
        if self.owner is not None:
            owner = self.owner.get_bootinfo_id()
        else:
            owner = PD.ROOTSERVER_PD_BI_NAME

        self.bi_name = self._generate_bi_name(bi, owner)

        for thread in self.threads:
            thread.generate_create_thread(bi)

        for zone in self.zones:
            zone.generate_bootinfo(bi, machine, bootinfo)

        for ms in self.memsections:
            ms.generate_bootinfo(bi, machine, bootinfo)

        if self.server_thread is not None:
            for ms in self.memsections:
                bi.write_register_server(self.server_thread.get_bootinfo_id(),
                                         ms.get_bootinfo_id())

        if hasattr(self, 'callback'):
            bi.write_register_callback(self.bi_name,
                                       self.callback.get_bootinfo_id())

        if hasattr(self, 'env_ms'):
            bi.write_register_env(self.bi_name,
                                  self.env_ms.get_bootinfo_id())

        if self.user_map_ms is not None:
            bi.write_security_control(self.bi_name,
                                      self.user_map_ms.get_bootinfo_id(),
                                      weaver.bootinfo_elf.BI_SECURITY_MAPPING)

        if self.platform_ctrl:
            bi.write_security_control(self.bi_name,
                                      0, # Unused
                                      weaver.bootinfo_elf.BI_SECURITY_PLATFORM)

        for thread in self.threads:
            thread.generate_bootinfo(bi, self.heap_ms, self.callback)

        # for (key, virt_dev) in self.virtual_devs.iteritems():
        #     virt_dev.generate_bootinfo(bi)

    def add_std_env(self, elf_info, segment_info, servers, virtpools,
                    physpools):
        """
        Add the standard entries to the environment.

        The standard entries are the callback buffer, default pools
        and the elf and segment info tables. 
        """
        if not self.added_std_env:
            if hasattr(self, 'callback'):
                self.environment.add_entry("__OKL4_CALLBACK_BUFFER",
                                           cap=self.callback.get_cap('master'),
                                           attach = 0x7)

            assert(self.default_virtpool is not None)
            self.environment.add_entry("OKL4_DEFAULT_VIRTPOOL",
                                       cap=self.default_virtpool.get_cap('master'))

            assert(self.default_physpool is not None)
            self.environment.add_entry("OKL4_DEFAULT_PHYSPOOL",
                                       cap=self.default_physpool.get_cap('master'))

            for (name, p) in [(name, p) for (name, p) in virtpools.items()
                              if not isinstance(p, Zone)]:
                self.environment.add_entry(name,
                                           cap=p.get_cap('master'))

            for (name, p) in physpools.items():
                self.environment.add_entry(name,
                                           cap=p.get_cap('master'))

            for z in self.zones:
                self.environment.add_entry(key      = z.get_name(),
                                           cap      = z.get_cap('master'),
                                           attach   = z.get_attach())

                for ms in z.get_memsections():
                    self.environment.add_entry(key      = z.get_name() + \
                                                          '/' + ms.get_name(),
                                               cap      = ms.get_cap('master'),
                                               attach   = 0)
                    

            for s in servers:
                self.environment.add_entry(key      = s.get_key(),
                                           cap_name = s.get_cap_name(),
                                           attach   = 0)

            for ei in elf_info:
                self.environment.add_elf_entry(
                    key         = basename(ei.name).upper(),
                    elf_type    = ei.type,
                    entry_point = ei.entry_point)

            for si in segment_info:
                self.environment.add_segment_entry(
                    key   = si.name.upper(),
                    segment = si.segment)

            self.added_std_env = True

    def calc_environment_size(self, machine):
        assert(self.environment is not None)
        assert(self.added_std_env)

        size = self.environment.calc_size(machine)

        self.env_ms.get_attrs().size = size

    def generate_environment_bootinfo(self, bi, elf_info,
                                      segment_info, servers,
                                      virtpools, physpools, machine,
                                      bootinfo, image):
        # If there is an environment, then fill it with it with the
        # standard items and write it out.
        assert(self.environment is not None)

        self.add_std_env(elf_info, segment_info, servers, virtpools,
                         physpools)

        # Find objects that are linked by alias caps and do
        # generate_bootinfo() on them now before it's too late
        # print "***** Environment %s *****" % self.name
        # print ""
        for e in self.environment.entries:

        #     print "----- Entry %s -----" % e.key

        #     if isinstance(e, Environment.Entry):
        #         print "isEntry = %s" % isinstance(e, Environment.Entry)

        #         if e.cap is not None:
        #             print "isCap = %s" % (e.cap is not None)

        #             if isinstance(e.cap, AliasCap):
        #                 print "isAliasCap = %s" % isinstance(e.cap, AliasCap)

        #             else:
        #                 print "Cap name = %s" % e.cap.name
        #         else:
        #             print "isCap = false"
        #     else:
        #         print "isEntry = false"

        #     print ""

            if isinstance(e, Environment.Entry) and \
               e.cap is not None and \
               isinstance(e.cap, AliasCap):
                e.cap.get_object().generate_implicit_bootinfo(self,
                                                              bi,
                                                              image,
                                                              machine,
                                                              bootinfo)

        self.environment.generate_bootinfo(self, bi)

class RootServerPD(PD):
    """
    Description of the RootServer PD.

    Rootserver PDs are special because they have a fixed bootinfo name
    and their stack and heap are handled differently.
    """
    
    def __init__(self, name, namespace, image, machine, pools):
        PD.__init__(self, name, namespace, image, machine, pools,
                    is_rootserver_pd = True)

        self.stack_ms = None

    def add_thread(self, thread):
        """Record the stack details of the main thread."""
        # The root server doesn't really care about threads, it just
        # wants the details of the stack.  The stack is not recorded
        # as a memsection it does not need all of the associated
        # processing.  The stack will have already been mapped by the
        # kernel at rootserver startup.
        #
        # This method can only be called once
        assert self.stack_ms is None

        self.stack_ms = thread.get_stack()
        self.stack_ms.ms.set_root_server_ownership(True)

    def attach_heap(self, heap_ms):
        """Record the details of the rootserver heap."""
        PD.attach_heap(self, heap_ms)
        self.heap_ms.ms.set_root_server_ownership(True)

        # The heap is not recorded as a memsection it does not need
        # all of the associated processing.  The heap will have
        # already been mapped by the kernel at rootserver startup.
        self.memsections.remove(heap_ms)

    def _generate_bi_name(self, bi, owner):
        """Return the bootinfo name of the PD."""
        return PD.ROOTSERVER_PD_BI_NAME

    def get_boot_details(self):
        """
        Return the details of the rootserver stack and heap that will
        be placed in the bootinfo header.

        The stack and heap of the rootserver are mapped in by the
        kernel and the details are passed in the bootinfo header for
        the rootserver to setup.
        """
        # This function will be called before and after
        # image.layout(), so the stack and heap virt addrs may be
        # None.  Before image.layout() the function can return any
        # value, but must not error!
        stack_attrs = self.stack_ms.get_attrs()
        heap_attrs  = self.heap_ms.get_attrs()

        stack_virt = stack_attrs.virt_addr
        if stack_virt is None:
            stack_virt = 0

        heap_virt = heap_attrs.virt_addr
        if heap_virt is None:
            heap_virt = 0

        return (stack_virt,
                stack_virt + stack_attrs.size - 1,
                heap_virt,
                heap_virt + heap_attrs.size - 1)

class BootInfo:
    class ElfInfo:
        def __init__(self, name, _type, entry_point):
            self.name        = name
            self.type        = _type
            self.entry_point = entry_point

    class SegmentInfo:
        def __init__(self, name, segment):
            self.name    = name
            self.segment = segment

    class Servers:
        """
        Class for recording info about servers that must appear in all
        environments.
        """
        def __init__(self, key, cap_name):
            self.key      = key
            self.cap_name = cap_name

        def get_key(self):
            """Return the key of the server."""
            return self.key

        def get_cap_name(self):
            """Return the cap to be placed in the environment."""
            return self.cap_name

    def __init__(self):
        self.pds              = []
        self.elf_info         = []
        self.segment_info     = []
        self.servers          = []
        self.virtpools        = {}
        self.physpools        = {}
        self.zones            = {}
        self.default_virtpool = None
        self.default_physpool = None
        self.default_directpool = None
        self.ms               = None

    def add_pd(self, pd):
        self.pds.append(pd)

    def add_rootserver_pd(self, pd):
        self.root_server_pd = pd
        self.add_pd(pd)
        
    def add_elf_info(self, name, elf_type, entry_point):
        self.elf_info.append(BootInfo.ElfInfo(name, elf_type, entry_point))

    def add_segment_info(self, name, segment):
        self.segment_info.append(BootInfo.SegmentInfo(name, segment))

    def add_server(self, key, cap_name):
        self.servers.append(BootInfo.Servers(key, cap_name))

    def record_segment_info(self, segment_prefix, seg,
                            image, machine, pools):
        """
        Record details about the segment for later inclusion in the
        object environment.
        """
        # Remove any pathname components from the prefix.
        segment_prefix = basename(segment_prefix)

        attrs = seg.get_attrs()
        self.add_segment_info("%s.%s" % (segment_prefix, attrs.name),
                              seg.get_elf_segment())

        # Return a memsection describing the segment.
        seg.get_attrs().scrub = False
        ms = MemSection(image, machine, pools, segment = seg)
        master = Cap("master", ["master"])
        ms.add_cap(master)

        return ms

    def add_virtpool(self, virtpool):
        self.virtpools[virtpool.get_name()] = virtpool

    def add_physpool(self, physpool):
        self.physpools[physpool.get_name()] = physpool

    def get_virtpool(self, name):
        return self.virtpools[name]

    def get_physpool(self, name):
        return self.physpools[name]

    def add_zone(self, zone):
        self.zones[zone.get_name()] = zone
        self.virtpools[zone.get_name()] = zone

    def get_zone(self, zone_name):
        return self.zones[zone_name]

    def set_system_default_attrs(self, virtpool_name, physpool_name, image,
                                 def_pager = None, def_direct = None):
        """
        Set the default attributes for the image.  These are specified
        in the rootprogram element.
        """
        if not self.virtpools.has_key(virtpool_name):
            raise MergeError, 'Virtual pool "%s" not found.' % virtpool_name

        if not self.physpools.has_key(physpool_name):
            raise MergeError, 'Physical pool "%s" not found.' % physpool_name
        
        self.default_virtpool = self.virtpools[virtpool_name]
        self.default_physpool = self.physpools[physpool_name]
        self.default_directpool = self.virtpools["direct"]

        image.set_attrs_stack(virtpool_name,
                              physpool_name,
                              def_pager,
                              def_direct)

    def get_system_default_pools(self):
        return (self.default_virtpool, self.default_physpool,
                self.default_directpool)

    def create_dynamic_segments(self, namespace, image, machine,
                                pools, bootinfo):
        """
        Calculate the size of the bootinfo and environment
        memsections, all of which depend on the items read in from the
        XML file.

        This function can only be called before the image memory
        layout occurs.
        """
        # Calculate the size of the bootinfo segment.
        f = self.create_ops(image, machine, bootinfo)

        attrs = image.new_attrs(namespace)

        # Since this is called before objects are placed in memory,
        # the bootinfo size will be incorrect because the freelist
        # does not reflect the final freelist.  To offset this, allow
        # some extra space for additional ADD_*_MEM ops.
        slop = BOOTINFO_GUESS_OPS * 4 * \
               (machine.word_size / 8)

        attrs.attach = PF_R
        attrs.name   = "bootinfo"
        attrs.pager  = None
        attrs.size   = align_up(len(f.getvalue()) + slop,
                                machine.min_page_size())
        attrs.data   = ByteArray()

        self.ms = image.add_memsection(attrs, machine, pools)
        image.add_group(0, [self.ms])

        f.close()

        # Adjust the size of the environment memsections.
        for pd in self.pds:
            pd.calc_environment_size(machine)

    def create_ops(self, image, machine, bootinfo):
        """
        Create the bootinfo image.

        This function will be called twice, once before memory layout
        and once after and, apart from the freelist, the size of the
        contents should be the same.  All code in called by this
        function must cope with this.
        """
        f = StringIO()

        bi = weaver.bootinfo_elf.BootInfo(f, machine.word_size,
                                          image.endianess)

        # Write the header, including the details of where the
        # rootserver can find it's stack and heap.
        rs_details = self.root_server_pd.get_boot_details()
        bi.write_header(debug      = 0,
                        stack_base = rs_details[0],
                        stack_end  = rs_details[1],
                        heap_base  = rs_details[2],
                        heap_end   = rs_details[3])
        # Record root-server's stack pointer.
        image.set_rootserver_stack(rs_details[1] + 1)

        # Generate the free memory list.

        # This is really hairy.  First a big block of free memory from
        # the default virtual and physical pools needs to be passed to
        # initialise iguana's memory subsystem.  Then everything else
        # in the default pools, except the initial memory, needs to be
        # passed in.  Just in case the biggest blocks aren't big
        # enough it is best if the virtual and physical memory ops for
        # the default pools are interleaved so that additional memory
        # can be mapped.  Once that's done the remaining pools can be
        # dumped in a sensible order.

        (def_virt, def_phys, def_direct) = self.get_system_default_pools()

        virt_free = def_virt.pool.get_freelist()[:]
        phys_free = def_phys.pool.get_freelist()[:]
        direct_free = def_direct.pool.get_freelist()[:]

        # Sort biggest to smallest
        virt_free.sort(key=lambda x: x[0] - x[1])
        phys_free.sort(key=lambda x: x[0] - x[1])
        direct_free.sort(key=lambda x: x[0] - x[1])

        # Extract the biggest regions and remove them from the lists.
        (first_virt_base, first_virt_end, mem_type) = virt_free[0]
        del virt_free[0]
        (first_phys_base, first_phys_end, mem_type) = phys_free[0]
        del phys_free[0]

        # Initialise the memory system
        (virt_name, phys_name, dir_name) = \
                    bi.write_new_init_mem(first_virt_base, first_virt_end,
                                          first_phys_base, first_phys_end)

        # Record the names of the created system default pools.
        def_virt.set_bootinfo_id(virt_name)
        def_phys.set_bootinfo_id(phys_name)
        def_direct.set_bootinfo_id(dir_name)

        # Mark out the rest of the default pools.  Interleave virtual
        # and physical memory record to ensure that there is always
        # virtual and physical to use.
        for n in range(max(len(virt_free), len(phys_free), len(direct_free))):
            if  n < len(virt_free):
                (base, end, mem_type) = virt_free[n]
                bi.write_add_virt_mem(def_virt.get_bootinfo_id(), base, end)

            if  n < len(phys_free):
                (base, end, mem_type) = phys_free[n]
                bi.write_add_phys_mem(def_phys.get_bootinfo_id(), base, end)

            if  n < len(direct_free):
                (base, end, mem_type) = direct_free[n]
                bi.write_add_virt_mem(def_direct.get_bootinfo_id(), base, end)

        # Now create the remaining virtual and physical pools and
        # write out their memory ranges.  In addition, create the caps
        # for *all* pools.  Order does not matter here.

        for pool in [p for p in self.virtpools.itervalues()
                     if not isinstance(p, Zone)]:
            if pool.pool.get_name() != def_virt.pool.get_name() and \
               pool.pool.get_name() != def_direct.pool.get_name():
                pool.generate_bootinfo(bi)

            for cap in pool.caps.itervalues():
                cap.generate_bootinfo(pool, bi)

        for pool in self.physpools.itervalues():
            if pool.pool.get_name() != def_phys.pool.get_name():
                pool.generate_bootinfo(bi)

            for cap in pool.caps.itervalues():
                cap.generate_bootinfo(pool, bi)

        bi.write_kernel_info(image.kconfig.get_max_spaces(),
                             image.kconfig.get_max_mutexes(),
                             image.kconfig.get_max_caps())

        # Write out the ops to create memsections and interrupts for
        # physical devices 
        # for (key, phys_dev) in \
        # weaver.machine.machine.physical_device.iteritems():
        #     phys_dev.generate_bootinfo(bi)

        # Write out the ops to create all memsections, threads, etc.
        for pd in self.pds:
            pd.generate_contents_bootinfo(bi, def_virt, def_phys,
                                          def_direct, machine,
                                          bootinfo)

        # Write out the environments.  These may refer to objects in
        # any PD.
        for pd in self.pds:
            pd.generate_environment_bootinfo(bi, self.elf_info,
                                             self.segment_info,
                                             self.servers,
                                             self.virtpools,
                                             self.physpools, machine,
                                             bootinfo, image)

        # Get a list of all threads
        all_threads = [thread for pd in self.pds for thread in pd.get_threads()]

        # Start the threads, sorted by decreasing priority.
        for thread in sorted(all_threads, key=lambda x: x.get_priority()):
            thread.generate_run_thread(bi)
 
        bi.write_end()
        return f

    def generate(self, image, machine, bootinfo):
        """Generate and write out the bootinfo section."""
        f = self.create_ops(image, machine, bootinfo)
        dlen = len(f.getvalue())

        assert(dlen <= self.ms.get_attrs().size)

        self.ms.segment.sections[0]._data[:dlen] = ByteArray(f.getvalue())
        image.kconfig.set_bootinfo(phys = self.ms.segment.paddr,
                                   size = self.ms.segment.get_memsz(),
                                   virt = self.ms.segment.vaddr)
        f.close()

