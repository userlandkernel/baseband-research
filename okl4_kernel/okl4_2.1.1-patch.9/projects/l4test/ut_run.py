#!/usr/bin/python
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

#
# Authour: Philip O'Sullivan <philipo@ok-labs.com>
# Date   : Wed 21 Jun 2006
#

import sys
sys.path.append('tools/unittest')
from ut_config import Variable, And, Or
from ut_core import main, TestRun

class L4TestRun(TestRun):
    def build_dir(self):
        res = '#build.l4'
        if self.config.dict.has_key('machine'):
            res += '_' + self.config['machine']
        return res

project = Variable('project', ['l4test'])
timeout = Variable('TESTING.TIMEOUT', ['500'])
pistachio_toolchains = Variable('pistachio.TOOLCHAIN', ['gnu_arm_toolchain', 'rvct_gnu_toolchain'])
l4test_toolchains = Variable('l4_rootserver.TOOLCHAIN', ['gnu_arm_toolchain', 'rvct_toolchain', 'ads_toolchain'])
arm_toolchains = And([pistachio_toolchains, l4test_toolchains])
print_log = Variable('TESTING.PRINT_LOG', ['True'])
mutex_type = Variable('mutex_type', ['user'])
options = And([print_log, timeout, mutex_type])

# Various configurations of scheduling configs
sched_strict = Variable('SCHEDULING_ALGORITHM', ['strict']);
sched_inheritance = Variable('SCHEDULING_ALGORITHM', ['inheritance']);
scheduler_configs = Or([sched_strict, sched_inheritance])
sched_default = sched_inheritance

# Standard tests for the gumstix
gumstix_run_targets = Variable('run', ['skyeye', 'hardware'])
gumstix_machine_name = Variable('machine', ['gumstix'])
gumstix_standard_tests = And([project, gumstix_machine_name, gumstix_run_targets, arm_toolchains, options, sched_default]) 

# Test various scheduler configurations on the gumstix
gumstix_scheduler_tests = \
        And([project, gumstix_machine_name, gumstix_run_targets, pistachio_toolchains, \
        sched_strict, \
        Variable('l4_rootserver.TOOLCHAIN', ['gnu_arm_toolchain']), options])

gumstix_tree = Or([gumstix_standard_tests, gumstix_scheduler_tests])

openmoko_tree = And([project, Variable('machine', ['gta01']), Variable('run', ['qemu']), options, scheduler_configs])

kzm_arm11_tree = And([project, Variable('machine', ['kzm_arm11']), Variable('run', ['hardware']), options, scheduler_configs])

ia32efi = Variable('machine', ['ia32efiapple'])
ia32efi_options = And([print_log, mutex_type, Variable('VERBOSE_INIT', ['True']), Variable('ENABLE_KDB_CONS_COM', ['True']), Variable('TESTING.TIMEOUT', ['240'])])
ia32efi_tree = And([project, ia32efi, ia32efi_options])

gxmips64 = Variable('machine', ['gxmips64'])
gxmips64_options = And([print_log, mutex_type, Variable('TESTING.TIMEOUT', ['120'])])
gxmips64_tree = And([project, gxmips64, gxmips64_options, scheduler_configs])

gxmips32 = Variable('machine', ['gxmips32'])
gxmips32_options = And([print_log, mutex_type, Variable('TESTING.TIMEOUT', ['120'])])
gxmips32_tree = And([project, gxmips32, gxmips32_options, scheduler_configs])

u4600 = Variable('machine', ['u4600'])
# @todo FIXME: reduce to 400 - davidg.
u4600_options = And([Variable('TESTING.TIMEOUT', ['600']), print_log, mutex_type])
u4600_tree = And([project, u4600, u4600_options, scheduler_configs])

ia32_pc99 = Variable('machine', ['ia32_pc99'])
ia32_pc99_options = And([print_log, mutex_type, Variable('toolprefix', ['i686-unknown-linux-gnu-']), Variable('TESTING.TIMEOUT', ['240'])])
ia32_pc99_options_profiling = And([print_log, mutex_type, Variable('toolprefix', ['i686-unknown-linux-gnu-']), Variable('TESTING.TIMEOUT', ['240']), Variable('enable_profiling', ['True'])])
ia32_pc99_tree = Or([And([project, ia32_pc99, ia32_pc99_options, scheduler_configs]), And([project, ia32_pc99, ia32_pc99_options_profiling])])

tree = Or([gumstix_tree, openmoko_tree, kzm_arm11_tree, ia32efi_tree, ia32_pc99_tree, u4600_tree, gxmips32_tree, gxmips64_tree])

if __name__ == '__main__':
    main(tree, L4TestRun)
