##############################################################################
# Copyright (c) 2006, National ICT Australia
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

############################################################################
# arm based machines
############################################################################
class arm(Machine):
    page_sizes = [0x1000, 0x10000, 0x100000]
    preferred_alignment = 0x100000L
    wordsize = 32
    arch = "arm"
    endian = "little"
    run_methods = {
        'skyeye': skyeye_sim
    }
    default_method = 'skyeye'
    default_toolchain = gnu_arm_toolchain
    rvct_c_flags = []
    ads_c_flags = []

# ARM CPU Classes

class armv4(arm):
    memory = arm.memory.copy()
    memory['virtual'] = [Region(0x1000, 0xd0000000)] # Trap NULL pointer derefs.
    cpp_defines = [("ARM_PID_RELOC", 1)]
    pid_reloc = True

class armv5(arm):
    memory = arm.memory.copy()
    base_vaddr = 0x80000000
    memory['virtual'] = [Region(0x80000000, 0xd0000000)] # Trap NULL pointer derefs.
    cpp_defines = [("ARM_PID_RELOC", 1), ("ARM_SHARED_DOMAINS", 1)]
    pid_reloc = True
    shared_domains = True

class armv6(arm):
    memory = arm.memory.copy()
    memory['virtual'] = [Region(0x1000, 0xe0000000)] # Trap NULL pointer derefs.

class armv4t(armv4):
    c_flags = armv4.c_flags + ["-march=armv4t"]
    cpp_defines = armv4.cpp_defines + [("__ARMv__", 4), "__ARMv4T__"]
    arch_version = 5
    
class armv5te(armv5):
    c_flags = armv5.c_flags + ["-march=armv5te"]
    cpp_defines = armv5.cpp_defines + [("__ARMv__", 5), "__ARMv5TE__"]
    arch_version = 5

class armv5tej(armv5):
    c_flags = armv5.c_flags + ["-march=armv5te"]
    cpp_defines = armv5.cpp_defines + [("__ARMv__", 5), "__ARMv5TEJ__"]
    arch_version = 5

class sa1100(arm):
    c_flags = arm.c_flags + ["-march=armv4", "-mtune=strongarm1100"]
    cpp_defines = armv5.cpp_defines + [("__ARMv__", 4), "__ARMv4__"]
    arch_version = 5
    memory = arm.memory.copy()
    memory['virtual'] = [Region(0x1000, 0xd0000000)] # Trap NULL pointer derefs.
    cpp_defines = [("ARM_PID_RELOC", 1)]
    pid_reloc = True
    cpu = "sa1100"
    timer_driver = "sa1100_timer"
    drivers = [timer_driver]

class pleb(sa1100):
    virtual = False
    serial_driver = "sa1100_uart"
    skyeye = "pleb.skyeye"
    platform = "pleb"
    drivers = [serial_driver] + sa1100.drivers
    memory = sa1100.memory.copy()
    memory['physical'] = [Region(0xc0000000L, 0xc1000000L)]
    cpp_defines = sa1100.cpp_defines + [("PLATFORM_PLEB", 1)]

class ipaq_h3800(sa1100):
    virtual = False
    skyeye = "pleb.skyeye"
    serial_driver = "sa1100_uart"
    elfloader = True
    elf_entry = 0xC0008000L
    platform = "pleb"
    drivers = [serial_driver] + sa1100.drivers
    memory = sa1100.memory.copy()
    memory['physical'] = [Region(0xc0000000L, 0xc1000000L)]
    cpp_defines = sa1100.cpp_defines + [("PLATFORM_PLEB", 1)]

class xscale(armv5te):
    c_flags = armv5te.c_flags + ["-mtune=xscale"]
    rvct_c_flags = armv5te.rvct_c_flags + ["--cpu", "XScale"]
    ads_c_flags = armv5te.ads_c_flags + ["-cpu", "XScale"]
    serial_driver = "uart_8250"
    cpu = "xscale"
    drivers = [serial_driver]

class armv6j(armv6):
    c_flags = armv6.c_flags + ["-march=armv5te"]
    cpp_defines = armv6.cpp_defines + [("__ARMv__", 6), "__ARMv5TE__"]
    arch_version = 6

class arm920t(armv4t):
    c_flags = armv4t.c_flags + ["-mtune=arm920t"]
    rvct_c_flags = armv4t.rvct_c_flags + ["--cpu", "arm920t"]
    ads_c_flags = armv4t.ads_c_flags + ["-cpu", "arm920t"]
    ads_as_flags = armv4t.ads_c_flags + ["-cpu", "arm920t"]
    cpu = "arm920t"

# with floating point
class armv6jf(armv6):
    c_flags = armv6.c_flags + ["-march=armv5te"]
    cpp_defines = armv6.cpp_defines + [("__ARMv__", 6), "__ARMv5TE__"]
    arch_version = 6

class arm1136js(armv6j):
    c_flags = armv6j.c_flags + ["-Wa,-mcpu=arm1136"]
    cpu = "arm1136js"

class arm1176jzs(armv6jf):
    cpu = "arm1176jzs"


class arm926ejs(armv5tej):
    c_flags = armv5tej.c_flags + ["-mtune=arm926ejs"]
    rvct_c_flags = armv5tej.rvct_c_flags + ["--cpu", "arm926ej-s"]
    ads_c_flags = armv5tej.ads_c_flags + ["-cpu", "arm926ej-s"]
    ads_as_flags = armv5tej.ads_c_flags + ["-cpu", "arm926ej-s"]
    cpu = "arm926ejs"

