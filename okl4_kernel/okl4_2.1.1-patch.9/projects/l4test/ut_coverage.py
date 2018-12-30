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
# Authour: Guillaume Mulko <gmulko@ok-labs.com>
# Date   : Thu 19 July 2007
#

import sys
sys.path.append("tools/unittest")
from ut_config import And, Or, Variable
from ut_core import main, TestRun

class CoverageTestRun(TestRun):
    def build_dir(self):
        res = '#build.coverage'
        if self.config.dict.has_key('machine'):
            res += '_' + self.config['machine']
        return res

    def command_line(self):
        build_dir = self.build_dir()
        self.config['BUILD_DIR'] = build_dir
        cmd = 'tools/build.py ' + self.command_options()
        if self.options.execute and not self.options.build:
                cmd += ' coverage'
        return cmd


# Projects
project = Variable('project', ['l4test'])

# Toolchains 

# Tests
l4test_timeout = Variable('TESTING.TIMEOUT', ['4500'])
print_log = Variable('TESTING.PRINT_LOG', ['True'])
disable_trace = Variable('ENABLE_TRACEPOINTS', ['False'])

# Machines
gumstix = Variable('machine', ['gumstix'])

# Tree by project
gumstix_tree = And([project, gumstix, Variable('run', ['skyeye']), l4test_timeout, print_log, disable_trace])

# Complete tree
tree = gumstix_tree


if __name__ == '__main__':
    main(tree, CoverageTestRun)
