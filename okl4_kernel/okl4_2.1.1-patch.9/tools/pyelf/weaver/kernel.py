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

"""Collection of items that will initialise the kernel."""

from weaver import MergeError
from weaver import kernel_elf

MemoryDescriptor = kernel_elf.MemoryDescriptor

class KernelInit:
    """
    Kernel initialisation parameters built from the XML file.
    These parameters are then merged into the kconfig section in the
    ELF file.
    """
    def __init__(self, section = "kernel.roinit"):
        self.section = section
        self.mem_info    = []
        self.configs     = []
        self.rs_entry    = None
        self.rs_stack    = None
        self.rs_mappings = []
        self.bi_mapping  = None
        self.max_spaces  = 256
        self.max_mutexes = 256
        self.max_caps    = 1024

    def set_section(self, section):
        """
        Set the name of the ELF section containing the kernen
        configuration.
        """
        self.section = section

    def set_rootserver_entry(self, entry):
        """Record the entry point of the root server."""
        self.rs_entry = entry

    def set_rootserver_stack(self, stack):
        """Record the stack pointer of the root server."""
        self.rs_stack = stack

    def set_rootserver_mappings(self, mappings):
        """Set the rootserver segment mappings."""
        self.rs_mappings = mappings

    def set_bootinfo(self, virt, phys, size):
        """Set the bootinfo details."""
        self.bi_mapping = (virt, phys, size)

    def add_mem_descriptor(self, desc):
        """Record a memory descriptor."""
        self.mem_info.append(desc)

    def add_config(self, key, value):
        """Record a configuration property."""
        self.configs.append((key, value))

        if key == "spaces":
            self.max_spaces = value
        elif key == "mutexes":
            self.max_mutexes = value
        elif key == "root_caps":
            self.max_caps = value
    
    def get_max_spaces(self):
        return self.max_spaces

    def get_max_mutexes(self):
        return self.max_mutexes

    def get_max_caps(self):
        return self.max_caps

    def _update_memory_descriptors(self, kconfig_sect, machine):
        """Write the memory descriptors to the kernel config."""
        for (base, size, mem_type) in machine.physical_memory:
            kconfig_sect.add_meminfo(mem_type, 0, 0, base, base + size - 1)

        self.mem_info.sort(key=lambda x: x._low)

        for mem_desc in self.mem_info:
            kconfig_sect.add_mem_descriptor(mem_desc)

    def _update_entry(self, kconfig_sect):
        """Write the root-servers entry point to the kernel config."""
        kconfig_sect.set_entry(self.rs_entry)

    def _update_stack(self, kconfig_sect):
        """Write the root-server's stack pointer to the kernel config."""
        kconfig_sect.set_stack(self.rs_stack)

    def _update_mappings(self, kconfig_sect):
        """Write the segment properties to the kconfig if they have been set."""
        # The bootinfo section is *always* the first mapping.
        if self.bi_mapping is not None:
            kconfig_sect.add_mapping(self.bi_mapping)

        for mapping in self.rs_mappings:
            kconfig_sect.add_mapping(mapping)

    def _update_configs(self, kconfig_sect):
        """Write config data to the kconfig structure."""
        for (key, value) in self.configs:
            kconfig_sect.set_config(key, value)


    def update_elf(self, elf, machine):
        """Update the kernel configuration page in the ELF file."""
        kconfig_sect = elf.find_section_named(self.section)

        if kconfig_sect is None:
            raise MergeError, "Couldn't find roinit section"

        kconfig_sect = kernel_elf.KernelConfigurationSection(kconfig_sect)

        self._update_entry(kconfig_sect)
        self._update_stack(kconfig_sect)
        self._update_mappings(kconfig_sect)
        self._update_memory_descriptors(kconfig_sect, machine)
        self._update_configs(kconfig_sect)
        kconfig_sect.update_data()        
