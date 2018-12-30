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
Module for generating Os Bootinfo segment.
"""

from struct import *
from sys import stdout
from elf.ByteArray import ByteArray
import struct
from weaver import MergeError

BI_MAGIC = (0x1960021d)
BI_VERSION  = 8

# Memsection flags.
BI_MEM_NORMAL = 0x1
BI_MEM_FIXED  = 0x2
BI_MEM_DIRECT = 0x4
BI_MEM_UTCB   = 0x8
BI_MEM_USER   = 0x10
BI_MEM_INTERNAL = 0x20
BI_MEM_LOAD   = 0x40
BI_MEM_PROTECTED = 0x80

# Access rights.
BI_READABLE        = (0x04)
BI_WRITABLE        = (0x02)
BI_EXECUTABLE      = (0x01)
BI_FULLYACCESSIBLE = (0x07)
BI_READWRITEONLY   = (0x06)
BI_READEXECONLY    = (0x05)
BI_NOACCESS        = (0X00)

# Memory attributes
BI_DEFAULT_MEMORY       = 0
BI_CACHED_MEMORY        = 1
BI_UNCACHED_MEMORY      = 2
BI_WRITE_BACK_MEMORY    = 3
BI_WRITE_THROUGH_MEMORY = 4
BI_COHERENT_MEMORY      = 5
BI_IO_MEMORY            = 6
BI_IO_COMBINED_MEMORY   = 7

# Cap rights.  Sent as a bitmask.
BI_CAP_EXECUTE = (1 << 0)
BI_CAP_WRITE   = (1 << 1)
BI_CAP_READ    = (1 << 2)
BI_CAP_MASTER  = (BI_CAP_EXECUTE | BI_CAP_WRITE | BI_CAP_READ)

# Operation Types.
BI_OP_HEADER            = 1
BI_OP_END               = 2
BI_OP_NEW_PD            = 3
BI_OP_NEW_MS            = 4
BI_OP_ADD_VIRT_MEM      = 5
BI_OP_ADD_PHYS_MEM      = 6
BI_OP_NEW_THREAD        = 7
BI_OP_RUN_THREAD        = 8
BI_OP_MAP               = 9
BI_OP_ATTACH            = 10
BI_OP_GRANT             = 11
#BI_OP_EXPORT           = 12 # Retired.
BI_OP_ARGV              = 13
BI_OP_REGISTER_SERVER   = 14
BI_OP_REGISTER_CALLBACK = 15
BI_OP_REGISTER_STACK    = 16
BI_OP_INIT_MEM          = 17
#BI_OP_NEW_VM_POOL       = 18 # Retired
#BI_OP_NEW_PHYS_POOL     = 19 # Retired
BI_OP_NEW_CAP           = 20
BI_OP_GRANT_CAP         = 21
BI_OP_OBJECT_EXPORT     = 22
BI_OP_STRUCT_EXPORT     = 23
BI_OP_REGISTER_ENV      = 24
BI_OP_NEW_POOL          = 25
BI_OP_GRANT_INTERRUPT   = 26
BI_OP_SECURITY_CONTROL  = 27
BI_OP_NEW_ZONE          = 28
BI_OP_ADD_ZONE_WINDOW   = 29
BI_OP_KERNEL_INFO       = 30

# Common Local Names
OS_GETENV_CLIST_SLOT =          (0x1)
OS_GETENV_CLIST_USED =          (0x2)
OS_GETENV_CLIST_BASE =          (0x3)
OS_GETENV_STACK      =          (0x4)
OS_GETENV_HEAP_BASE  =          (0x5)
OS_GETENV_HEAP_SIZE  =          (0x6)
OS_GETENV_CALLBACK   =          (0x7)
OS_GETENV_DEFAULT_POOL  =       (0x8)
OS_GETENV_DEFAULT_VIRT_POOL =   (0x9)
OS_GETENV_TCM_POOL =            (0xa)
OS_GETENV_SEGMENT_INFO =        (0xb)

# Export types
BI_EXPORT_CONST          = 1
BI_EXPORT_OBJECT         = 2
BI_EXPORT_BASE           = 3
BI_EXPORT_THREAD_ID      = 4
BI_EXPORT_MEMSECTION_CAP = 5
BI_EXPORT_THREAD_CAP     = 6
BI_EXPORT_VIRTPOOL_CAP   = 7
BI_EXPORT_PHYSPOOL_CAP   = 8
BI_EXPORT_CLIST_CAP      = 9
BI_EXPORT_ELF_SEGMENT    = 10
BI_EXPORT_ELF_FILE       = 11
BI_EXPORT_PD_CAP         = 12
BI_EXPORT_PHYSDEV_CAP    = 13
BI_EXPORT_VIRTDEV_CAP    = 14
BI_EXPORT_ZONE_CAP       = 15

# Security Control types.
BI_SECURITY_PLATFORM  = 1 # Platform Control
BI_SECURITY_SPACE     = 2 # Space Switch
BI_SECURITY_INTERRUPT = 3 # Interrupt Control
BI_SECURITY_MAPPING   = 4 # User Mapping

# Class for writing out writing out a bootinfo file.
class BootInfo:

    def __init__(self, output, bits_per_word, endianess):
        def encode_hdr_word(op, size):
            if bits_per_word == 64:
                return op << 32 | size
            else:
                return op << 16 | size

        self.enc_func = encode_hdr_word

        if bits_per_word == 64:
            self.word_char = 'Q'
        else:
            self.word_char = 'I'

        self.BI_IGNORE = (2L ** bits_per_word) - 1

        # FIXME: Horrible code
        if endianess == '>':
            self._endianess = '>'
        elif endianess == '<':
            self._endianess = '<'
        else:
            raise MergeError, "Bad endianess"
        
        self.output = output
        self.format_word = self._endianess + self.word_char
        self.bytes_per_word = bits_per_word / 8
        self.object_index = 0

    def encode_word(self, word):
        return pack(self.format_word, word)

    def encode_hdr(self, op, size):
        real_size = (size + 1) * self.bytes_per_word
        word = self.enc_func(op, real_size)
        return self.encode_word(word)

    def padded_length(self, string):
        size = len(string) + 1
        padding = 0
        if size % self.bytes_per_word != 0:
            padding = self.bytes_per_word - (size % self.bytes_per_word)

        return size + padding
        
    def string_words(self, string):
        """Return the number of words occupied by a string."""
        return self.padded_length(string) / self.bytes_per_word

    def encode_string(self, string):
        size = len(string) + 1
        string += '\0'
        padding = 0
        if size % self.bytes_per_word != 0:
            padding = self.bytes_per_word - (size % self.bytes_per_word)
        return pack('%s%ds%dx' % (self._endianess, size, padding), string)
        

    # Public functions, that write out each type of record.
    def write_header(self, debug, stack_base, stack_end, heap_base, heap_end):
        header = self.encode_hdr(BI_OP_HEADER, 7)
        header += self.encode_word(BI_MAGIC)
        header += self.encode_word(BI_VERSION)
        header += self.encode_word(debug)
        header += self.encode_word(stack_base)
        header += self.encode_word(stack_end)
        header += self.encode_word(heap_base)
        header += self.encode_word(heap_end)
        self.output.write(header)

    def write_end(self):
        header = self.encode_hdr(BI_OP_END, 0)
        self.output.write(header)

    def write_new_pd(self, owner):
        header = self.encode_hdr(BI_OP_NEW_PD, 1)
        header = header + self.encode_word(owner)
        self.output.write(header)

        self.object_index = self.object_index + 1
        return self.object_index

    def write_new_ms(self, owner, base, size, flags, attr, physpool,
                     virtpool, zone):
        if zone is None:
            zone = self.BI_IGNORE

        if virtpool is None:
            virtpool = self.BI_IGNORE

        header = self.encode_hdr(BI_OP_NEW_MS, 8)
        header = header + self.encode_word(owner)
        header = header + self.encode_word(base)
        header = header + self.encode_word(size)
        header = header + self.encode_word(flags)
        header = header + self.encode_word(attr)
        header = header + self.encode_word(physpool)
        header = header + self.encode_word(virtpool)
        header = header + self.encode_word(zone)
        self.output.write(header)

        self.object_index = self.object_index + 1
        return self.object_index

    def write_new_init_mem(self, virt_base, virt_end, phys_base, phys_end):
        header = self.encode_hdr(BI_OP_INIT_MEM, 4)
        header = header + self.encode_word(virt_base)
        header = header + self.encode_word(virt_end)
        header = header + self.encode_word(phys_base)
        header = header + self.encode_word(phys_end)
        self.output.write(header)

        self.object_index = self.object_index + 3
        return (self.object_index - 2, self.object_index - 1, self.object_index)

    def write_new_pool(self, is_virtual):
        header = self.encode_hdr(BI_OP_NEW_POOL, 1)
        header = header + self.encode_word(is_virtual)
        self.output.write(header)

        self.object_index = self.object_index + 1
        return self.object_index

    def write_new_zone(self, pool):
        header = self.encode_hdr(BI_OP_NEW_ZONE, 1)
        header = header + self.encode_word(pool)
        self.output.write(header)

        self.object_index = self.object_index + 1
        return self.object_index

    def write_add_zone_window(self, zone, base):
        header = self.encode_hdr(BI_OP_ADD_ZONE_WINDOW, 2)
        header = header + self.encode_word(zone)
        header = header + self.encode_word(base)
        self.output.write(header)

    def write_add_virt_mem(self, pool, base, end):
        header = self.encode_hdr(BI_OP_ADD_VIRT_MEM, 3)
        header = header + self.encode_word(pool)
        header = header + self.encode_word(base)
        header = header + self.encode_word(end)
        self.output.write(header)

    def write_add_phys_mem(self, pool, base, end):
        header = self.encode_hdr(BI_OP_ADD_PHYS_MEM, 3)
        header = header + self.encode_word(pool)
        header = header + self.encode_word(base)
        header = header + self.encode_word(end)
        self.output.write(header)

    def write_new_thread(self, owner, ip, user_main, priority, name):
        header = self.encode_hdr(BI_OP_NEW_THREAD,
                                 4 + self.string_words(name))
        header = header + self.encode_word(owner)
        header = header + self.encode_word(ip)
        header = header + self.encode_word(user_main)
        header = header + self.encode_word(priority)
        header = header + self.encode_string(name)
        self.output.write(header)

        self.object_index = self.object_index + 1
        return self.object_index

    def write_register_stack(self, thread, ms):
        header = self.encode_hdr(BI_OP_REGISTER_STACK, 2)
        header = header + self.encode_word(thread)
        header = header + self.encode_word(ms)
        self.output.write(header)

    def write_run_thread(self, name):
        header = self.encode_hdr(BI_OP_RUN_THREAD, 1)
        header = header + self.encode_word(name)
        self.output.write(header)

    def write_map(self, vaddr, size, paddr, scrub, mode):
        header = self.encode_hdr(BI_OP_MAP, 5)
        header = header + self.encode_word(vaddr)
        header = header + self.encode_word(size)
        header = header + self.encode_word(paddr)
        header = header + self.encode_word(scrub)
        header = header + self.encode_word(mode)
        self.output.write(header)

    def write_attach(self, pd, ms, rights):
        header = self.encode_hdr(BI_OP_ATTACH, 3)
        header = header + self.encode_word(pd)
        header = header + self.encode_word(ms)
        header = header + self.encode_word(rights)
        self.output.write(header)

    def write_grant(self, pd, obj, rights):
        header = self.encode_hdr(BI_OP_GRANT, 3)
        header = header + self.encode_word(pd)
        header = header + self.encode_word(obj)
        header = header + self.encode_word(rights)
        self.output.write(header)

    def write_argv(self, thread, argv):
        header = self.encode_hdr(BI_OP_ARGV,
                                 1 + self.string_words(argv))
        header = header + self.encode_word(thread)
        header = header + self.encode_string(argv)
        self.output.write(header)

    def write_register_server(self, thread, ms):
        header = self.encode_hdr(BI_OP_REGISTER_SERVER, 2)
        header = header + self.encode_word(thread)
        header = header + self.encode_word(ms)
        self.output.write(header)

    def write_register_callback(self, pd, ms):
        header = self.encode_hdr(BI_OP_REGISTER_CALLBACK, 2)
        header = header + self.encode_word(pd)
        header = header + self.encode_word(ms)
        self.output.write(header)

    def write_security_control(self, pd, obj, type):
        header = self.encode_hdr(BI_OP_SECURITY_CONTROL, 3)
        header += self.encode_word(pd)
        header += self.encode_word(obj)
        header += self.encode_word(type)
        self.output.write(header)

    def write_new_cap(self, obj, rights):
        header = self.encode_hdr(BI_OP_NEW_CAP, 2)
        header = header + self.encode_word(obj)
        header = header + self.encode_word(rights)
        self.output.write(header)

        self.object_index = self.object_index + 1
        return self.object_index

    def write_grant_cap(self, pd, cap):
        header = self.encode_hdr(BI_OP_GRANT_CAP, 2)
        header = header + self.encode_word(pd)
        header = header + self.encode_word(cap)
        self.output.write(header)

    def write_object_export(self, pd, key, obj, type):
        header = self.encode_hdr(BI_OP_OBJECT_EXPORT,
                                 3 + self.string_words(key))
        header = header + self.encode_word(pd)
        header = header + self.encode_word(obj)
        header = header + self.encode_word(type)
        header = header + self.encode_string(key)
        self.output.write(header)

    def write_struct_export(self, pd, key, first, second,
                            third = 0L, fourth = 0, fifth = 0,
                            sixth = 0, type = BI_EXPORT_CONST):
        header = self.encode_hdr(BI_OP_STRUCT_EXPORT,
                                 8 + self.string_words(key))
        header = header + self.encode_word(pd)
        header = header + self.encode_word(first)
        header = header + self.encode_word(second)
        header = header + self.encode_word(third)
        header = header + self.encode_word(fourth)
        header = header + self.encode_word(fifth)
        header = header + self.encode_word(sixth)
        header = header + self.encode_word(type)
        header = header + self.encode_string(key)
        self.output.write(header)

    def write_register_env(self, pd, ms):
        header = self.encode_hdr(BI_OP_REGISTER_ENV, 2)
        header = header + self.encode_word(pd)
        header = header + self.encode_word(ms)
        self.output.write(header)

    def write_grant_interrupt(self, thread, irq):
        header = self.encode_hdr(BI_OP_GRANT_INTERRUPT, 2)
        header = header + self.encode_word(thread)
        header = header + self.encode_word(irq)
        self.output.write(header)

    def write_kernel_info(self, max_spaces, max_mutexes, max_caps):
        header = self.encode_hdr(BI_OP_KERNEL_INFO, 3)
        header += self.encode_word(max_spaces)
        header += self.encode_word(max_mutexes)
        header += self.encode_word(max_caps)
        self.output.write(header)

def _words2str(words, fmt):
    """
    Convert a list of longs, as generated by struct.unpack(), back
    into a string, and removing any trailing NULs
    """
    text = ""
    for i in words:
        text += struct.pack(fmt, i)

    # Strings that don't end on a word boundary will have trailing NUL
    # chars, so strip them off.
    end_null = text.find('\0')
    if end_null != -1:
        text = text[:end_null]

    return text


class IgBootInfoSection(object):
    class BootInfoHeader:
        def __init__(self, section, words):
            self.magic      = words[0]
            self.version    = words[1]
            self.debug      = words[2]
            self.stack_base = words[3]
            self.stack_end  = words[4]
            self.heap_base  = words[5]
            self.heap_end   = words[6]

        def __repr__(self):
            return "HEADER (magic: %#x, version: %#x, debug: %#x,  " \
                   "stack_base: %#x, stack_end: %#x, heap_base: %#x, " \
                   "heap_end: %#x)" % \
                   (self.magic, self.version, self.debug,
                   self.stack_base, self.stack_end, self.heap_base,
                   self.heap_end)

    class BootInfoEnd:
        def __init__(self, section, words):
            pass

        def __repr__(self):
            return "END"

    class BootInfoNewPD:
        def __init__(self, section, words):
            self.owner = words[0]
            self.name  = section.alloc_name()

        def __repr__(self):
            return "NEW PD (owner: %d) = %d" % (self.owner, self.name)

    class BootInfoNewMS:
        def __init__(self, section, words):
            self.owner    = words[0]
            self.base     = words[1]
            self.size     = words[2]
            self.flags    = words[3]
            self.attr     = words[4]
            self.physpool = words[5]
            self.virtpool = words[6]
            self.zone     = words[7]
            self.name     = section.alloc_name()

        def __repr__(self):
            return "NEW MS (owner: %d, base: 0x%x, size: 0x%x, flags " \
                   "0x%x, attr: 0x%x, physpool: 0x%x, " \
                   "virtpool: 0x%x, zone:  %d) = %d" % \
                   (self.owner, self.base, self.size, self.flags,
                   self.attr, self.physpool, self.virtpool, self.zone,
                   self.name)

    class BootInfoInitMem:
        def __init__(self, section, words):
            self.virt_base = words[0]
            self.virt_end  = words[1]
            self.phys_base = words[2]
            self.phys_end  = words[3]
            self.virt_name = section.alloc_name()
            self.phys_name = section.alloc_name()
            self.dir_name = section.alloc_name()

        def __repr__(self):
            return "MEM INIT (virt_base: 0x%x, virt_end: 0x%x, " \
                   "phys_base: 0x%x, phys_end: 0x%x) = (%d, %d, %d)" % \
                   (self.virt_base, self.virt_end, self.phys_base,
                    self.phys_end, self.virt_name, self.phys_name,
                    self.dir_name)

    class BootInfoNewPool:
        def __init__(self, section, words):
            self.is_virtual = words[0]
            self.name       = section.alloc_name()

        def __repr__(self):
            return "NEW POOL (is_virtual: %d) = %d" % \
                   (self.is_virtual, self.name)

    class BootInfoAddVirtMem:
        def __init__(self, section, words):
            self.pool = words[0]
            self.base = words[1]
            self.end  = words[2]

        def __repr__(self):
            return "ADD VIRT MEM (pool: 0x%x, base: 0x%x, end: 0x%x)" % \
                   (self.pool, self.base, self.end)

    class BootInfoAddPhysMem:
        def __init__(self, section, words):
            self.pool = words[0]
            self.base = words[1]
            self.end  = words[2]

        def __repr__(self):
            return "ADD PHYS MEM (pool: 0x%x, base: 0x%x, end: 0x%x)" % \
                   (self.pool, self.base, self.end)

    class BootInfoNewThread:
        def __init__(self, section, words):
            self.owner     = words[0]
            self.ip        = words[1]
            self.user_main = words[2]
            # get priority (signed)
            self.priority  = struct.unpack(section.format_word.lower(),
                                          struct.pack(section.format_word, words[3]))[0]

            self.kdb_name  = _words2str(words[4:], section.format_word)
            self.name      = section.alloc_name()

        def __repr__(self):
            return 'NEW THREAD (owner: %d, ip: 0x%x, user_main: 0x%x priority: %d, name: "%s") = %d' % \
                   (self.owner, self.ip, self.user_main,
                    self.priority, self.kdb_name, self.name)

    class BootInfoRegisterStack:
        def __init__(self, section, words):
            self.thread = words[0]
            self.ms     = words[1]

        def __repr__(self):
            return "REGISTER STACK (thread: %d, ms: %d)" % \
                   (self.thread, self.ms)

    class BootInfoRunThread:
        def __init__(self, section, words):
            self.name = words[0]

        def __repr__(self):
            return "RUN THREAD (name: %d)" % self.name

    class BootInfoMap:
        def __init__(self, section, words):
            self.vaddr = words[0]
            self.size  = words[1]
            self.paddr = words[2]
            self.scrub = words[3]
            self.mode  = words[4]

        def __repr__(self):
            return "MAP (vaddr: 0x%x, size: 0x%x, paddr: 0x%x, scrub: 0x%x, mode: 0x%x)" % \
                   (self.vaddr, self.size, self.paddr, self.scrub, self.mode)

    class BootInfoAttach:
        def __init__(self, section, words):
            self.pd     = words[0]
            self.ms     = words[1]
            self.rights = words[2]

        def __repr__(self):
            return "ATTACH (pd: %d, ms: %d, rights: 0x%x)" % \
                   (self.pd, self.ms, self.rights)

    class BootInfoGrant:
        def __init__(self, section, words):
            self.pd     = words[0]
            self.obj    = words[1]
            self.rights = words[2]

        def __repr__(self):
            return "GRANT (pd: %d, obj: %d, rights: 0x%x)" % \
                   (self.pd, self.obj, self.rights)

    class BootInfoArgv:
        def __init__(self, section, words):
            self.thread = words[0]
            self.arg    = _words2str(words[1:], section.format_word)

        def __repr__(self):
            return 'ARGV (thread: %d, arg: "%s")' % \
                   (self.thread, self.arg)

    class BootInfoRegisterServer:
        def __init__(self, section, words):
            self.thread = words[0]
            self.ms     = words[1]

        def __repr__(self):
            return "REGISTER SERVER (thread: %d, ms: %s)" % \
                   (self.thread, self.ms)

    class BootInfoRegisterCallback:
        def __init__(self, section, words):
            self.pd = words[0]
            self.ms = words[1]

        def __repr__(self):
            return "REGISTER CALLBACK (pd: %d, ms: %s)" % \
                   (self.pd, self.ms)

    class BootInfoSecurityControl:
        security_types = {
            BI_SECURITY_PLATFORM  : 'Platform Control',
            BI_SECURITY_SPACE     : 'Space Switch',
            BI_SECURITY_INTERRUPT : 'Interrupt Control',
            BI_SECURITY_MAPPING   : 'User Mapping',
            }

        def __init__(self, section, words):
            self.pd   = words[0]
            self.obj  = words[1]
            self.type = words[2]

        def __repr__(self):
            str_type = self.security_types.get(self.type, "Unknown")

            return "SECURITY CONTROL (pd: %d, obj: %s, type: %s (%d))" % \
                   (self.pd, self.obj, str_type, self.type)

    class BootInfoNewCap:
        def __init__(self, section, words):
            self.obj    = words[0]
            self.rights = words[1]
            self.name   = section.alloc_name()

        def __repr__(self):
            return "NEW CAP (object: %d, rights: 0x%x) = %d" % \
                   (self.obj, self.rights, self.name)

    class BootInfoGrantCap:
        def __init__(self, section, words):
            self.pd     = words[0]
            self.cap    = words[1]

        def __repr__(self):
            return "GRANT_CAP (pd: %d, cap: %d)" % \
                   (self.pd, self.cap)

    class BootInfoObjectExport:
        export_types = {
            BI_EXPORT_CONST         : 'Constant',
            BI_EXPORT_OBJECT        : 'Object Reference',
            BI_EXPORT_BASE          : 'Memsection Base Addr',
            BI_EXPORT_THREAD_ID     : 'Thread Id',
            BI_EXPORT_MEMSECTION_CAP: 'Memsection Cap',
            BI_EXPORT_THREAD_CAP    : 'Thread Cap',
            BI_EXPORT_VIRTPOOL_CAP  : 'VirtPool Cap',
            BI_EXPORT_PHYSPOOL_CAP  : 'PhysPool Cap',
            BI_EXPORT_CLIST_CAP     : 'Clist Cap',
            BI_EXPORT_ZONE_CAP      : 'Zone Cap',
            }

        def __init__(self, section, words):
            self.pd   = words[0]
            self.obj  = words[1]
            self.type = words[2]
            self.key  = _words2str(words[3:], section.format_word)

        def __repr__(self):
            str_type = self.export_types.get(self.type, "Unknown")

            return 'OBJECT_EXPORT (pd: %d, key: "%s", obj: %d, type: %s (%d))' % \
                   (self.pd, self.key, self.obj, str_type, self.type)

    class BootInfoStructExport:
        export_types = {
            BI_EXPORT_ELF_SEGMENT: 'ELF Segment Info',
            BI_EXPORT_ELF_FILE   : 'ELF File Info',
            }

        def __init__(self, section, words):
            self.pd     = words[0]
            self.first  = words[1]
            self.second = words[2]
            self.third  = words[3]
            self.fourth = words[4]
            self.fifth  = words[5]
            self.sixth  = words[6]
            self.type   = words[7]
            self.key    = _words2str(words[8:], section.format_word)

        def __repr__(self):
            str_type = self.export_types.get(self.type, "Unknown")

            return 'STRUCT_EXPORT (pd: %d, key: "%s", first: 0x%x, second: 0x%x, third: 0x%x, fourth: 0x%x, fifth: 0x%x, sixth: 0x%x, type: %s (%d))' % \
                   (self.pd, self.key, self.first, self.second,
                   self.third, self.fourth, self.fifth, self.sixth,
                   str_type, self.type)

    class BootInfoRegisterEnvironment:
        def __init__(self, section, words):
            self.pd = words[0]
            self.ms = words[1]

        def __repr__(self):
            return "REGISTER ENV (pd: %d, ms: %s)" % \
                   (self.pd, self.ms)

    class BootInfoNewZone:
        def __init__(self, section, words):
            self.pool = words[0]
            self.name = section.alloc_name()

        def __repr__(self):
            return "NEW ZONE (pool: %d) = %d" % \
                   (self.pool, self.name)

    class BootInfoAddZoneWindow:
        def __init__(self, section, words):
            self.zone = words[0]
            self.base = words[1]

        def __repr__(self):
            return "ADD ZONE WINDOW (zone: %d, base: 0x%x)" % \
                   (self.zone, self.base)

    class BootInfoGrantInterrupt:
        def __init__(self, section, words):
            self.thread = words[0]
            self.irq    = words[1]

        def __repr__(self):
            return "GRANT INTERRUPT (thread: %d, irq: %d) " % (self.thread, self.irq)

    class BootInfoKernelInfo:
        def __init__(self, section, words):
            self.max_spaces  = words[0]
            self.max_mutexes = words[1]
            self.max_caps    = words[2]

        def __repr__(self):
            return "KERNEL INFO (spaces: %d, mutexes: %d, caps: %d) " % \
                   (self.max_spaces, self.max_mutexes, self.max_caps)

    op_classes = {
        BI_OP_HEADER            : BootInfoHeader,
        BI_OP_END               : BootInfoEnd,
        BI_OP_NEW_PD            : BootInfoNewPD,
        BI_OP_NEW_MS            : BootInfoNewMS,
        BI_OP_ADD_VIRT_MEM      : BootInfoAddVirtMem,
        BI_OP_ADD_PHYS_MEM      : BootInfoAddPhysMem,
        BI_OP_NEW_THREAD        : BootInfoNewThread,
        BI_OP_RUN_THREAD        : BootInfoRunThread,
        BI_OP_MAP               : BootInfoMap,
        BI_OP_ATTACH            : BootInfoAttach,
        BI_OP_GRANT             : BootInfoGrant,
        BI_OP_ARGV              : BootInfoArgv,
        BI_OP_REGISTER_SERVER   : BootInfoRegisterServer,
        BI_OP_REGISTER_CALLBACK : BootInfoRegisterCallback,
        BI_OP_REGISTER_STACK    : BootInfoRegisterStack,
        BI_OP_INIT_MEM          : BootInfoInitMem,
        BI_OP_NEW_POOL          : BootInfoNewPool,
        BI_OP_NEW_CAP           : BootInfoNewCap,
        BI_OP_GRANT_CAP         : BootInfoGrantCap,
        BI_OP_OBJECT_EXPORT     : BootInfoObjectExport,
        BI_OP_STRUCT_EXPORT     : BootInfoStructExport,
        BI_OP_REGISTER_ENV      : BootInfoRegisterEnvironment,
        BI_OP_NEW_ZONE          : BootInfoNewZone,
        BI_OP_ADD_ZONE_WINDOW   : BootInfoAddZoneWindow,
        BI_OP_GRANT_INTERRUPT   : BootInfoGrantInterrupt,
        BI_OP_KERNEL_INFO       : BootInfoKernelInfo,
        BI_OP_SECURITY_CONTROL  : BootInfoSecurityControl,
        }

    def __init__(self, section):
        """Create a new bootinfosection. This is not normally called
        directly, as bootinfo instances usually are created first as
        ELfSection objects and then transformed."""
        self.section = section
        self._bytes_per_word = 0
        self.obj_count = 1
        data = self.section._data
        self._items = []

        # The first thing we need to do is determine the word size and
        # the endianness.  This is quite a dodgy way of working this
        # out but unfortunately the elf flags are unavailable.

        # Work out bootinfo type / endianness + wordsize.
        magic = struct.unpack("<I", data[4:8])[0]
        if magic == BI_MAGIC:
            self._wordsize = 4
            self._word_char = 'I'
            self._endianess = '<'

        magic = struct.unpack(">I", data[4:8])[0]
        if magic == BI_MAGIC:
            self._wordsize = 4
            self._word_char = 'I'
            self._endianess = '>'

        magic = struct.unpack("<Q", data[8:16])[0]
        if magic == BI_MAGIC:
            self._wordsize = 8
            self._word_char = 'Q'
            self._endianess = '<'

        magic = struct.unpack(">Q", data[8:16])[0]
        if magic == BI_MAGIC:
            self._wordsize = 8
            self._word_char = 'Q'
            self._endianess = '>'

        self._bytes_per_word = self._wordsize
        self.format_word = self._endianess + self._word_char

        idx = 0

        # Transform the operations into classes.
        while idx < len(data):
            # Read the header.
            hdr = struct.unpack(self.format_word, data[idx:idx + self._bytes_per_word])[0]
            # Calculate the size of the instruction in bytes and
            # words, and the op.
            halfword_bits = self._bytes_per_word * 4
            size = hdr & ((2 ** halfword_bits)-1)
            words = size / self._bytes_per_word
            op = hdr >> halfword_bits

            # Turn into a class.  Pass the constructor the list of
            # words in the op for further processing.
            format = self._endianess + self._word_char * (words - 1)
            start  = idx + self._bytes_per_word
            end    = idx + size
            if not IgBootInfoSection.op_classes.has_key(op):
                raise Exception, "Unknown opcode: %d" % op

            inst   = \
                 IgBootInfoSection.op_classes[op](self, struct.unpack(format,
                                                                      data[start : end]))
            self._items.append(inst)

            # Move to the next op.
            idx += size
            
            if op == BI_OP_END:
                self.total_size = idx
                break


    def alloc_name(self):
        name = self.obj_count
        self.obj_count += 1

        return name


    def output(self, f=stdout):
        """Print an ASCII representation of the OS bootinfo to the file f."""
        print >> f, "OS BootInfo (%s, %s) Operations:" % \
                (self._endianess == "<" and "Little endian" or "Big endian", \
                 str(self._wordsize) + " bit")
        rec_no = 0
        for op in self._items:
            print >> f, "%d: %s" % (rec_no, str(op))
            rec_no += 1

        print >> f, "Total size: 0x%x bytes" % self.total_size

def find_bootinfo(elf):
    """Find the bootinfo data structure in an ELF. Return None if it
    can't be found."""
    bootinfo = elf.find_section_named("bootinfo")
    if bootinfo:
        bootinfo = IgBootInfoSection(bootinfo)
    return bootinfo
