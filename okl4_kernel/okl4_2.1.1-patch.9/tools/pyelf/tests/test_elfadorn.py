#
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

import util
import os
import weaver.display as display
import adorn.main as adorn_main
import adorn.arguments as adorn_arguments
import adorn.linkerscript as adorn_linker_script
import subprocess

modules_under_test =  [adorn_main, adorn_arguments, adorn_linker_script]

base = "./elfadorn_tests"
obj_dir = os.path.join(base, "obj_files")
linker_script_dir = os.path.join(base, "linker_scripts")
out_dir = os.path.join(base, "output")


test_args = [ 
             (["ia32_gnu", "i686-unknown-linux-gnu-ld", "-T", "gnu.lds", "ia32_obj"],
                            ('gcc', ['gnu.lds'])),
             (["mips64_gnu", "mips64-elf-ld", "-T", "gnu.lds", "mips64_obj"],
                            ('gcc', ['gnu.lds'])),
             (["arm_gnu", "arm-linux-ld", "-T", "gnu.lds", "arm_obj"],
                            ('gcc', ['gnu.lds'])),
             (["arm_rvct", os.path.join(base, "rvct_link"), "--scatter", "rvct.lds", "rvct_obj"],
                            ('rvct', ['rvct.lds'])),
            ]

def linker_exists(args):
    """Filter to check that a linker exists."""
    # rvct_link, being a wrapper, needs two arguments before it can be
    # used to check the existence of the actual linker.
    linker = args[0][1]
    script_arg = args[0][2]
    try:
        retcode = subprocess.call('%s %s >/dev/null 2>&1' % (linker, script_arg),
                                  shell=True)
        return retcode != 127
    except OSError:
        return False

# Filter out linkers that aren't on this machine.
test_args = filter(linker_exists, test_args)

class TestAdornCmd(util.TestCmd):
    display_cmd = [display.print_cmd]
    cmd = display_cmd

    def tearDown(self):
        for args in test_args:
            try:
                file = os.path.join(out_dir, args[0][0])
                os.remove(file)
            except OSError:
                pass

    def test_help(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["-H"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stdout.capitalize().startswith("Usage"), True)

    def _cons_args(self, args):
        outfile = args[0]
        linker = args[1]
        script_arg = args[2]
        linker_script = args[3]
        obj_file = args[4]


        return ["./run.py",
                "-o", 
                os.path.join(out_dir, outfile), 
#                "-s",
                "--", 
                linker,
                script_arg, 
                os.path.join(linker_script_dir, linker_script), 
                os.path.join(obj_dir, obj_file), 
                "-o", 
                os.path.join(out_dir, outfile)
               ]

    def _run_adorn(self, args):
        args = self._cons_args(args)
        return adorn_main.main(args)

    def test_adorn_command(self):
        expected_output = ['SEGMENT_1', 'SEGMENT_2', 'SEGMENT_3', 'SEGMENT_4', 'SEGMENT_5']
        for args in test_args:
            exit_value = self._run_adorn(args[0])
            self.assertEquals(exit_value, None)
            print_args = [ "-s", os.path.join(out_dir, args[0][0])]
            exit_val, ret_stdout, ret_stderr = self._run_command(print_args)
            self.assertEquals(exit_val, 0) 
            result = ret_stdout.split("\n")
            result = result[1:6]
            self.assertEqual(expected_output, result)

    def test_arguments(self):
        iter = 0
        for args in test_args:
            expected_output = args[1]
            script = adorn_arguments.get_script_names(args[0]) 
            name =   adorn_arguments.get_linker_name(args[0], "")
            ret = (name, script)
            self.assertEquals(ret, expected_output)


