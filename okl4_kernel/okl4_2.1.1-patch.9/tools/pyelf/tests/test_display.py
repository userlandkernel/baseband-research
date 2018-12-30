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

import unittest
import weaver.display
from StringIO import StringIO
from elf.core import PreparedElfFile
import util

modules_under_test = [weaver.display]

expected_output = {
    "data/null_elf" : (0,),
    "data/arm_exec" : (0, ),
    "data/arm_exec_nosect" : (0, ),
    "data/arm_object" : (0, ),
    "data/arm_scatter_load" : (0, ),
    "data/amd64_exec" : (0, ),
    "data/ia32_exec" : (0, ),
#    "data/ia64_exec" : (0, ),
    "data/mips64_exec" : (0, )
    }


class TestPrintPheader(unittest.TestCase):
    def test_print_headers_summary(self):
        for fn in expected_output:
            output = StringIO()
            elf = PreparedElfFile(filename=fn)
            weaver.display.print_pheaders(elf, True, output)
            expected_data = open("%s.readelf.l" % fn).read()
            self.assertEquals(output.getvalue(), expected_data,
                              "Failed on: %s Got: \n%s \nExpected: \n%s" % (fn, output.getvalue(), expected_data))
            output.close()


class TestPrintSheader(unittest.TestCase):
    def test_print_headers_summary(self):
        for fn in expected_output:
            output = StringIO()
            elf = PreparedElfFile(filename=fn)
            weaver.display.print_sheaders(elf, True, output)
            expected_data = open("%s.readelf.S" % fn).read()
            #self.assertEquals(output.getvalue(), expected_data,
            #                  "Failed on: %s Got: \n%s \nExpected: \n%s" % (fn, output.getvalue(), expected_data))
            output.close()

class TestPrintCmd(util.TestCmd):
    cmd = [weaver.display.print_cmd]
    def test_help(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["-H"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stdout.capitalize().startswith("Usage"), True)

    def test_wrong_args(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["-a"])
        self.assertNotEqual(exit_value, 0)
        self.assertEquals(ret_stderr.capitalize().startswith("Usage"), True)

    def test_all_with_kip(self):
        for fn in expected_output:
            exit_value, ret_stdout, ret_stderr = self._run_command(["-a", fn])
            self.assertEquals(exit_value, 0)
    
    def test_all_no_kip(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["-a", "data/null_elf"])
        self.assertEquals(exit_value, 0)

    def test_header(self):
        for fn in expected_output:
            exit_value, ret_stdout, ret_stderr = self._run_command(["-h", fn])
            self.assertEquals(exit_value, 0)
            
    def test_bootinfo(self):
        for fn in expected_output:
            exit_value, ret_stdout, ret_stderr = self._run_command(["-B", fn])
            self.assertEquals(exit_value, 0)
                
        exit_value, ret_stdout, ret_stderr = self._run_command(["-B", "data/bootinfo_elf"])
        self.assertEquals(exit_value, 0)
                                
    def test_segnames(self):
        for fn in expected_output:
            exit_value, ret_stdout, ret_stderr = self._run_command(["-s", fn])
            self.assertEquals(exit_value, 0)
            
        exit_value, ret_stdout, ret_stderr = self._run_command(["-s", "data/eg_weave/l4kernel"])
        self.assertEquals(exit_value, 0)
                        
    def test_kconfig(self):
        for fn in expected_output:
            exit_value, ret_stdout, ret_stderr = self._run_command(["-k", fn])
            self.assertEquals(exit_value, 0)
            
        exit_value, ret_stdout, ret_stderr = self._run_command(["-k", "data/eg_weave/l4kernel"])
        self.assertEquals(exit_value, 0)
                                
