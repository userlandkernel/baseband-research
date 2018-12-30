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

from weaver import MergeError
import weaver.bootinfo_elf
from weaver.bootinfo_elf import BI_EXPORT_CONST, BI_EXPORT_THREAD_ID, BI_IO_MEMORY

class AliasCapObject:
    """
    Common interfaces for objects that can have an alias cap defined.

    Not subclassing BootInfoObject is intentional, because these objects are
    not supposed to appear in bootinfo at all.
    """
    def __init__(self):
        self.name = "Error: Unnamed AliasCapObject"

    def create_implicit_objects(self, namespace, machine=None, pools=None, image=None, bootinfo=None):
        pass # do nothing

    def generate_implicit_bootinfo(self, pd, bi, image=None, machine=None, bootinfo=None):
        pass # do nothing

class VirtualDevice(AliasCapObject):
    """
    A virtual device object. Contains enough information for consumers
    to communicate to and reference the virtual device.
    """

    def __init__(self, name, server_name, server_pd, server_thread, index):
        self.name           = name
        self.index          = index
        self.server_name    = server_name
        self.server_pd      = server_pd
        self.server_thread  = server_thread
        self.client_thread  = None

#     def get_server(self):
#         return self.server

    def get_index(self):
        return self.index

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_VIRTDEV_CAP

    def create_implicit_objects(self, namespace, machine=None, pools=None, image=None, bootinfo=None):
        """
        Create memory objects that are implicitly defined by this
        virtual device.

        For virtual devices, nothing needs to be done for now.  See
        comments in PhysicalDevice.create_implicit_objects.
        """
        pass

    def generate_implicit_bootinfo(self, pd, bi, image=None, machine=None, bootinfo=None):
        # BEWARE!  This pd.server_thread is NOT refering to the device server,
        # but the server thread of the client program.
        #
        # In other words:
        #   - pd (arg to this method) = device client PD
        #   - self.server_pd          = device server PD
        #   - self.server_thread     != pd.server_thread
        #
        # Don't be confused! - nt
        #
        if pd.server_thread is not None:
            self.client_tid = pd.server_thread
        else:
            raise MergeError, "Receiving PD must have a server thread to get virtdev handles."

        # Patch server variables to tell it about client thread ids
        elf = self.server_pd.elf
        if not elf:
            raise MergeError, "You cannot give device control to PDs without an executable ELF."
        sym  = elf.find_symbol("virtual_device_instance")
        if not sym:
            raise MergeError, "Cannot find symbol virtual_device_instance[]"

        # XXX: The number of elements within the granted_physmem, interrupt and
        # valid device instances is hardcoded to 4 in the code.  get_size()
        # returns the total size of the array while we want the size of an
        # element, so we divide by 4 here.  A better method might be to define
        # the size of the array in the build system and use that value there
        # in addition pass a -DDEVICES_ARRAY=num to gcc and use that #define
        # instead of the hardcoded values.
        size = sym.size / 4
        addr = sym.value + size * self.index

        image.patch(addr, 4, 1)

        # Add device handle and device server tid to client environment
        bi.write_object_export(pd   = pd.get_bootinfo_id(),
                               key  = self.server_name.upper() + "_TID",
                               obj  = self.server_thread.get_bootinfo_id(),
                               type = BI_EXPORT_THREAD_ID)
        bi.write_object_export(pd   = pd.get_bootinfo_id(),
                               key  = self.server_name.upper() + "_HANDLE",
                               obj  = self.index,
                               type = BI_EXPORT_CONST)

class PhysicalDevice(AliasCapObject):
    """
    A physical device object.  Encompasses device information.
    """

    def __init__(self, name):
        self.name            = name
        self.physical_mem    = {}
        self.interrupt       = {}

        self.mappings        = {} # mappings[name] = (physpool, virtpool, memsection)

    def add_physical_mem(self, name, mem):
        """Add a named list of physical memory ranges."""
        self.physical_mem[name] = mem
        # _assert_no_overlaps(self.physical_mem)

    def add_interrupt(self, name, number):
        """Add a named interrupts."""
        # print "New interrupt %d added" % number
        self.interrupt[name] = number

    def get_physical_mem(self, name):
        """Get the named list of physical memory ranges."""
        if self.physical_mem.has_key(name):
            return self.physical_mem[name]

    def get_interrupt(self, name):
        """Get the named interrupt."""
        if not self.interrupt.has_key(name):
            raise MergeError, "Interrupt called %s not found." % name

        return self.interrupt[name]

    def get_export_type(self):
        return weaver.bootinfo_elf.BI_EXPORT_PHYSDEV_CAP

    def create_implicit_objects(self, namespace, machine, pools, image, bootinfo):
        """
        Create memory objects that are implicitly defined by this
        physical device.

        The reason we have to do this now, rather than earlier or later,
        is because when the PhysicalDevice is created, we don't know
        who owns it, so we can't do memsection allocation yet.

        If we leave it until create_ops() when bootinfo instructions are
        generated, layout() is already determined, so that's too late.

        The only solution is to create the objects when you do
        collect_environment_element().  This guarantees that this
        function will only be called once.

        For physical devices, for each physmem we create a phys pool,
        and allocate memsections out of it.  We grant a cap to the memsection
        to the containing PD.

        No objects are created for interrupts.
        """
        caps = {}

        for (name, physmem) in self.physical_mem.iteritems():

            # Create the physpool
            pp = weaver.bootinfo.PhysPool("%s_pool" % name, machine, pools)
            bootinfo.add_physpool(pp)
            # This cap is probably never used.
            master = weaver.bootinfo.Cap("master", ["master"])
            pp.add_cap(master)
            namespace.add(master.get_name(), master)
            cap_name = "%s_pool" % name
            caps[cap_name.upper()] = master

            for (base, size, mem_type) in physmem:
                pp.add_memory(None, base, size, machine, pools)

            # Create the memsection
            attrs = image.new_attrs(namespace)
            attrs.name  = name
            attrs.size  = size
            attrs.pager = None
            attrs.physpool = pp.name
            attrs.scrub = False
            attrs.cache_policy = BI_IO_MEMORY
            # attrs.virtpool can only be determined when we know the owner PD
            ms = weaver.bootinfo.MemSection(image, machine, pools, attrs)
            # This cap is probably never used.
            master = weaver.bootinfo.Cap("master", ["master"])
            ms.add_cap(master)
            image.add_group(None, [ms.get_ms()])

            # Return the list of caps, the XML parser should add it
            cap_name = "%s_ms" % name
            caps[cap_name.upper()] = master

            # Record the mapping
            self.mappings[name] = (pp, None, ms)

        return caps

    def generate_interrupt_bootinfo(self, pd, bi, image):
        """
        Generate bootinfo instructions to grant interrupts to the PD.
        """
        elf = pd.elf
        if not elf:
            raise MergeError, "You cannot give device control to PDs without an executable ELF."
        sym  = elf.find_symbol("iguana_granted_interrupt")
        if not sym:
            raise MergeError, "Cannot find symbol iguana_granted_interrupt[]"

        addr = sym.value
        offset = 0
        size = sym.size

        for irq in self.interrupt.itervalues():
            if pd.server_thread is not None:
                thread = pd.server_thread
            elif len(pd.get_threads()) > 0:
                thread = pd.get_threads()[0]
            else:
                raise MergeError, "Cannot grant interrupt to PD with no threads"

            # Instruct iguana server to grant interrupt
            bi.write_grant_interrupt(thread = thread.bi_name, irq = irq)

            # Patch the virtual address bases in the elf image
            image.patch(addr + offset, 4, irq)
            offset += 4
        
        # We write -1 unused interrupt entries to denote invalidity
        while offset < size:
            image.patch(addr + offset, 4, 0xffffffff)
            offset += 4

    def generate_mapping_bootinfo(self, pd, bi, machine, bootinfo, image):
        """
        Generate bootinfo instructions to grant device memory mappings to the PD.
        """
        elf = pd.elf
        if not elf:
            raise MergeError, "You cannot give device control to PDs without an executable ELF."
        sym  = elf.find_symbol("iguana_granted_physmem")
        if not sym:
            raise MergeError, "Cannot find symbol iguana_granted_physmem[]"

        addr = sym.value
        offset = 0
        size = sym.size

        for (name, mapping) in self.mappings.iteritems():
            if not offset < size:
                raise MergeError, "The physmem array has overflowed.  Increase its size."

            (pp, none, ms) = mapping

            # We can only attach the memsection now because we don't
            # know the owner PD until this point in time
            pd.attach_memsection(ms)

            # The memsection should use the PD's default virtpool, but
            # the previously generated physpool
            (virtpool, foo, directpool) = pd.get_default_pools()
            ms.get_attrs().virtpool = virtpool.name

            # Generate the mapping bootinfo ops
            # No need to tell Iguana to create the memsection
            ms.generate_bootinfo(bi, machine, bootinfo, skip_if_implicit = True)
            ms.set_implicit()

            # Update the mapping with the correct virtpool
            self.mappings[name] = (pp, virtpool, ms)

            # Patch the virtual address bases in the elf image
            # We only want to apply the patch only when the vbase
            # is known.  This happens on the second pass to
            # generate_bootinfo()
            if ms.vbase is not 0:
                image.patch(addr + offset, 4, ms.vbase)
            offset += 4

    def generate_implicit_bootinfo(self, pd, bi, image, machine, bootinfo):
        self.generate_mapping_bootinfo(pd, bi, machine, bootinfo, image)
        self.generate_interrupt_bootinfo(pd, bi, image)


