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

import os
import commands
import re

###################
# Simulator targets
###################

def qemu_sim(target, source, env):
    status, output = commands.getstatusoutput("qemu")
    version = re.search(r'QEMU PC emulator version ([0-9\.]+)', output)
    if version == None:
        raise "No simulator found?"
    version = version.group(1)
    tap_device = commands.getstatusoutput("test -r /dev/net/tun && test -w /dev/net/tun")
    if version <= '0.7.0':
        return "qemu %s -hda %s  -nics 0" % (" ".join(env.qemu_args), source[0].abspath)
    elif env.wombat_net and tap_device == 0:
        return "qemu %s -hda %s -net nic -net tap -nographic -m 164" % (" ".join(env.qemu_args), source[0].abspath)
    else:
        return "qemu %s -hda %s -net none  -nographic -m 164" % (" ".join(env.qemu_args), source[0].abspath)


def qemu_gta_sim(target, source, machine):
    os.chdir("/opt/okl/Linux-i386/usr/share/qemu-neo")
    return "/opt/okl/Linux-i386/bin/qemu-system-arm -M neo -m 130 -mtdblock  /opt/okl/Linux-i386/usr/share/qemu-neo/openmoko-flash.base -serial stdio -nographic -kernel %s" % (source[0].abspath)

def qemu_versatile_sim(target, source, machine):
    return "/opt/okl/Linux-i386/bin/qemu-system-arm -system-arm  -M versatilepb -nographic %s" % (source[0].abspath)

def skyeye_sim(target, source, machine):
    return "skyeye -c tools/sim_config/%s -e %s" % (machine.skyeye, source[0].abspath)

def sulima_sim(target, source, machine):
    return "u4600 %s" % source[0].abspath

def m5_sim(target, source, env):
    return "m5.opt %s" % source[0].abspath

def gxemul_sim(target, source, machine):
    return "gxemul -C %s -E %s %s" % (machine.sim_cpu, machine.sim_plat, source[0].abspath)

def gumstix_boot(target, source, env):
    return "sh tools/unittest/gumstix-boot.sh %s" % source[0].abspath

def kzm_boot(target, source, env):
    return "sh tools/unittest/kzm-boot.sh %s" % source[0].abspath

def mac_boot(target, source, env):
    return "sh tools/unittest/mac-boot.sh %s" % source[0].abspath


############################################################################
# Import customer specific simulator targets
############################################################################
import os
for thedir in ["arch", "platform", "cust"]:
    if os.path.exists(thedir):
        for dir_ in os.listdir(os.path.join(thedir)):
            file_name = os.path.join(thedir, dir_, "tools", "simulators.py")
            if os.path.exists(file_name):
                execfile(file_name)

for thedir in ["tools/cust"]:
    if os.path.exists(thedir):
        for dir_ in os.listdir(os.path.join(thedir)):
            file_name = os.path.join(thedir, dir_, "simulators.py")
            if os.path.exists(file_name):
                execfile(file_name)

