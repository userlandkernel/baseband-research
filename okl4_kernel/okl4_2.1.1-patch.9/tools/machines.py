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

from simulators import *
from toolchains import *
import SCons.Script

class Region(object):
    """
    Describes a region of memory.

    The properties of a region are:
    base - The base address of the region.
    end  - The address one byte past the end of the region.
    mem_type - The type of memory as a string.  Supported types are
               'conventional' and 'dedicated'.  The default is
               'conventional'.
    """
    def __init__(self, base, end,
                 mem_type = "conventional"):
        self.base     = base
        self.end      = end
        self.size     = end - base
        self.mem_type = mem_type

    def get_base(self):
        return base

    def get_end(self):
        return self.end

    def get_size(self):
        return self.size

    def get_mem_type(self):
        return mem_type

class Machine(object):
    virtual = True   # this is a virtual target - ie class, not instance

    default_toolchain = None

    as_flags = []
    c_flags = []
    cpp_flags = []
    cpp_defines = []
    cxx_flags = []
    link_flags = []
    drivers = []
    device_core = None

    wordsize = None
    endian = None

    timer_driver = None
    serial_driver = None

    cpu = ""

    # Arrays of valid address ranges for the different types of memory
    # in the machine.
    memory = {
        'virtual'  : [],
        'physical' : [],
        'rom'      : [],
        'tcm'      : [],
        }

    base_vaddr = 0x100000
    # Preferred alignment for this machine.  The preferred alignment
    # produces the best performance at the cost of memory
    # fragmentation.
    preferred_alignment = 0x10000
    min_alignment=0x1000
    smp = False # Multiple CPUs/threads?
    zero_bss = False # Zero the bss of the boot image?
    boot_binary = False # Create a binary of the boot image?
    copy_elf = False # Copy the elf image to the build directory?
    cust = "okl" # Default customer
    default_feature_profile = "EXTRA"

############################################################################
# Import customer specific machines
############################################################################


import os
import imp
dirs = ["."] + [os.path.join("cust", x) for x in os.listdir("cust")]
for top_dir in dirs:
    for middle_dir in ["arch", "platform"]:
        path = os.path.join(top_dir, middle_dir)
        if not os.path.exists(path):
            continue
        for bottom_dir in os.listdir(path):
            file_name = os.path.join(path, bottom_dir, "tools", "machines.py")
            if os.path.exists(file_name):
                execfile(file_name)
