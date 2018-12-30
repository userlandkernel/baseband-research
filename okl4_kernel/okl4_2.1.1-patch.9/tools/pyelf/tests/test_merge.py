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

import weaver.merge
import util
import os
import commands
import sys

modules_under_test = [weaver.merge, weaver.bootinfo, weaver.ezxml,
                      weaver.segments_xml, weaver.image, weaver.kernel,
                      weaver.kernel_elf, weaver.kernel_xml, weaver.machine,
                      weaver.machine_xml, weaver.namespace,
                      weaver.bootinfo_elf, weaver.prog_pd_xml,
                      weaver.memobjs_xml, weaver.pools_xml,
                      weaver.rootprogram_xml,
                      weaver.parse_spec, weaver.pools]

# Test to see if we have readelf installed

status, output = commands.getstatusoutput("readelf -H")
# status is 0 when readelf gave us its help some large nonzero otherwise
have_readelf = not status
if not have_readelf:
    print >> sys.stderr, "test_structures.py: You don't have readelf " \
          "installed. Not all tests will run."

# Note: We test not equal to zero, rather than explicit exit codes
# as it differs across python versions.

class TestMergeCmd(util.TestCmd):
    cmd = [weaver.merge.merge_cmd]
    def tearDown(self):
        try:
            os.remove("test_output")
        except OSError:
            pass
    
    def test_help(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["-H"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stdout.capitalize().startswith("Usage"), True)

    def test_no_output(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["specfoo"])
        self.assertNotEqual(exit_value, 0)
        self.assertEquals(ret_stderr.capitalize().startswith("Usage"), True)

    def test_no_spec(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["-o", "foo"])
        self.assertNotEqual(exit_value, 0)
        self.assertEquals(ret_stderr.capitalize().startswith("Usage"), True)

    def test_empty_spec(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/empty.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 1)
        self.assertEquals(ret_stderr.startswith("Failed to parse"), True)

    def test_simple_spec(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_simple_extension(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_extension.xml", "-o", "test_output"])
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        
        self.assertEquals(exit_value, 0)

    def test_simple_spec_nosections(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple.xml", "-o", "test_output", "--no-section-headers"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        
    
        # These strings should be present in a sectionless ELF ...
        status, output = commands.getstatusoutput("readelf -a test_output|grep 'Size of section headers:           0 (bytes)'")
        self.assertEquals(status, 0)
        status, output = commands.getstatusoutput("readelf -a test_output|grep 'Number of section headers:         0'")
        self.assertEquals(status, 0)
        status, output = commands.getstatusoutput("readelf -a test_output|grep 'There are no sections in this file.'")
        self.assertEquals(status, 0)
        # ... and this one shouldn't
        status, output = commands.getstatusoutput("readelf -a test_output|grep 'Section to Segment mapping'")
        self.assertNotEqual(status, 0)

#     def test_simple_with_offset_spec(self):
#         exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_with_offset.xml", "-o", "test_output"])
#         self.assertEquals(ret_stderr, "")
#         self.assertEquals(ret_stdout, "")        
#         self.assertEquals(exit_value, 0)

    def test_simple_no_exec(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_no_exec.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 1)
        self.assertEquals(ret_stderr, "Error: All the merged ELF files must be of EXEC type.\n")
        self.assertEquals(ret_stdout, "")        

    def test_simple_non_load(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_non_load.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 1)
        self.assertEquals(ret_stderr.startswith("Error: Unable to handle segment"), True)
        self.assertEquals(ret_stdout, "")        

    def test_simple_ignore_segment(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_ignore_segment.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_simple_raw_section(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_raw_section.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_simple_raw_file(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_raw_file.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_simple_noname(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_noname.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_simple_patch(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_with_patch.xml", "-o", "test_output"])
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        
        self.assertEquals(exit_value, 0)

    def test_import_machine(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple_import_machine.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_multiple_pools(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/multiple_pools.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")        

    def test_multiple_elf_file(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/multiple_elf_file.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")
# With dynamic allocation, this check needs to be re-written.
#        exit_val = self.check_segments_and_sections("data/eg_weave/multiple_elf_file.xml", "test_output")
#        self.assertEquals(exit_val, 0)

    def test_multiple_pager(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/multiple_pager.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "")

    def test_multiple_pager2(self):
        """Test various error conditions with custom pagers."""
        template = """
<image>
	<machine file="data/eg_weave/machine.xml" />
	<virtual_pool name="virtual">
		<memory base="0x1000" size="0xcffff000"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory base="0xa0000000" size="0x3800000"/>
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming" %s>
               <memsection name="paged" size="16K" %s />
        </program>
        <pd name="a_pd" %s>
        </pd>
</image>
"""
        inputs = (
            # Defaults.
            { 'values':
              ('', '', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('pager="none"', '', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('pager="default"', '', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('pager="memload"', '', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('', '', 'pager="memload"'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('pager="undefined"', '', ''),
              'exit_value': 1,
              'stdout': "",
              'stderr': """Error: "undefined" is not a recognised pager.  Valid values are ('none', 'default', 'memload').\n""",
              },
            { 'values':
              ('', '', 'pager="undefined"'),
              'exit_value': 1,
              'stdout': "",
              'stderr': """Error: "undefined" is not a recognised pager.  Valid values are ('none', 'default', 'memload').\n""",
              },
            { 'values':
              ('', 'pager="none"', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('', 'pager="default"', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('', 'pager="memload"', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            { 'values':
              ('', 'pager="undefined"', ''),
              'exit_value': 1,
              'stdout': "",
              'stderr': """Error: "undefined" is not a recognised pager.  Valid values are ('none', 'default', 'memload').\n""",
              },
            { 'values':
              ('pager="memload"', 'pager="none"', 'pager="default"'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])


    def test_scatter_load(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/scatter_load.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "") 
# With dynamic allocation, this check needs to be re-written.
#        self.check_segment_addr("data/eg_weave/scatter_load.xml", "test_output")

    def test_object_environment(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/environment.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "") 

    def test_file_inclusion(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/include_common.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "") 

    def test_memory_overlap(self):
        """Test address machine address and pool overlap checks."""
        template = """
<image>
	<machine>
		<word_size size="0x20" />
		<virtual_memory name="virtual">
			<region base="%s" size="%s" />
		</virtual_memory>
		<virtual_memory name="virtual2">
			<region base="%s" size="%s" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="%s" size="%s" />
		</physical_memory>
		<physical_memory name="physical2">
			<region base="%s" size="%s" />
		</physical_memory>
		<page_size size="0x1000" />
	</machine>
	<virtual_pool name="virtual">
		<memory base="%s" size="%s"/>
	</virtual_pool>
	<virtual_pool name="virtual2">
		<memory base="%s" size="%s"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory base="%s" size="%s"/>
	</physical_pool>
	<physical_pool name="physical2">
		<memory base="%s" size="%s"/>
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
</image>
        """

        inputs = (
            # No memory pools overlap
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # One machine virtual range inside another.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x10000000", "0x10000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x10000000", "0x10000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: The machine memory region 0x10000000--0x1fffffff (size 0x10000000) in "virtual2" overlaps with region 0x0--0x7fffffff (size 0x80000000) in "virtual".\n',
              },
            # One machine virtual range base address is in another range.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x10000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x10000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: The machine memory region 0x10000000--0x8fffffff (size 0x80000000) in "virtual2" overlaps with region 0x0--0x7fffffff (size 0x80000000) in "virtual".\n',
              },
            # One machine virtual range end address is in another range.
            { 'values':
              ("0x50000000", "0x80000000",
               "0x10000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x50000000", "0x80000000",
               "0x10000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: The machine memory region 0x50000000--0xcfffffff (size 0x80000000) in "virtual" overlaps with region 0x10000000--0x8fffffff (size 0x80000000) in "virtual2".\n',
              },
            # One machine physical range inside another.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0xa1000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0xa1000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: The machine memory region 0xa1000000--0xc0ffffff (size 0x20000000) in "physical2" overlaps with region 0xa0000000--0xdfffffff (size 0x40000000) in "physical".\n',
              },
            # One machine physical range base address is in another range.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0xa1000000", "0x40000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0xa1000000", "0x40000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: The machine memory region 0xa1000000--0xe0ffffff (size 0x40000000) in "physical2" overlaps with region 0xa0000000--0xdfffffff (size 0x40000000) in "physical".\n',
              },
            # One machine physical range end address is in another range.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0x9f000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0x9f000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: The machine memory region 0xa0000000--0xdfffffff (size 0x40000000) in "physical" overlaps with region 0x9f000000--0xbeffffff (size 0x20000000) in "physical2".\n',
              },
            # One virtual pool range inside another.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x10000000", "0x10000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Virtual Pools: Parts of "virtual2" (0x10000000-0x1fffffff, size 0x10000000) overlap with "virtual" (0x0-0x7fffffff, size 0x80000000).\n',
              },
            # One virtual pool range base address is in another pool.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x10000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Virtual Pools: Parts of "virtual2" (0x10000000-0x8fffffff, size 0x80000000) overlap with "virtual" (0x0-0x7fffffff, size 0x80000000).\n',
              },
            # One virtual pool range end address is in another pool.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x50000000", "0x80000000",
               "0x10000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Virtual Pools: Parts of "virtual2" (0x10000000-0x8fffffff, size 0x80000000) overlap with "virtual" (0x50000000-0xcfffffff, size 0x80000000).\n',
              },
            # One physical pool range inside another.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0xa1000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical Pools: Parts of "physical2" (0xa1000000-0xc0ffffff, size 0x20000000) overlap with "physical" (0xa0000000-0xdfffffff, size 0x40000000).\n',
              },
            # One physical pool range base address inside another pool.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0x40000000", "0x40000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0xa1000000", "0x40000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical Pools: Parts of "physical2" (0xa1000000-0xe0ffffff, size 0x40000000) overlap with "physical" (0xa0000000-0xdfffffff, size 0x40000000).\n',
              },
            # One physical pool range end address inside another pool.
            { 'values':
              ("0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x20000000",
               "0x40000000", "0x20000000",
               "0x00000000", "0x80000000",
               "0x80000000", "0x80000000",
               "0xa0000000", "0x40000000",
               "0x9f000000", "0x20000000"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical Pools: Parts of "physical2" (0x9f000000-0xbeffffff, size 0x20000000) overlap with "physical" (0xa0000000-0xdfffffff, size 0x40000000).\n',
              },
            )
        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])

    def test_segment_direct(self):
        """
        Test various combinations of directly mapping every segment
        in a program.
        """

        template = """
<image>
	<machine>
		<word_size size="0x20" />
		<virtual_memory name="virtual">
			<region base="0x1000" size="0xcffff000" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="%s" size="%s" />
		</physical_memory>
		<page_size size="0x1000" />
	</machine>
	<virtual_pool name="virtual">
		<memory src="virtual"/>
	</virtual_pool>

	<physical_pool name="physical" direct="%s">
		<memory src="physical" />
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming" direct="%s" >
        </program>
</image>
        """

        inputs = (
            # No direct segment mapping.
            { 'values':
              ("0xa0000000", "0x3800000", "false", "false"),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Direct mapping.
            { 'values':
              ("0x80000000", "0x3800000", "true", "true"),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Ask for direct mapping but the physpool doesn't support
            # it.
            { 'values':
              ("0x80000000", "0x3800000", "false", "true"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical pool "physical" does not support direct memory allocation.\n',
              },
            # Ask for direct mapping, but the address range is not in
            # the pool.
            { 'values':
              ("0xa0000000", "0x3800000", "true", "true"),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Segment "/ig_naming/rx": Cannot reserve physical addresses 0x80100000--0x80107e55.\n',
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])

    def test_segment_attrs(self):
        """
        Test reported problems with segment elements referring to
        segments that are not in the ELF file, and tests for virtual
        and physical memory overlap.
        """

        template = """
<image>
	<machine>
		<word_size size="0x20" />
		<virtual_memory name="virtual">
			<region base="0x1000" size="0xcffff000" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="0xa0000000" size="0x3800000" />
		</physical_memory>
		<page_size size="0x1000" />
	</machine>
	<virtual_pool name="virtual">
		<memory src="virtual"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory src="physical" />
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming">
        %s
        </program>
</image>
        """

        inputs = (
            # Refer to a non-existent segment name.
            { 'values':
              ('<segment name="non_existent" />'),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: /ig_naming: Cannot find segment "non_existent" in the ELF file. Valid values are [\'rx\', \'rw\']\n',
              },
            # Overlap the segments in physical memory.
            { 'values':
              ('<segment name="rx" phys_addr="0xa2000000" /> <segment name="rw" phys_addr="0xa2000000" />'),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical Memory: Parts of "/ig_naming/rw" (0xa2000000-0xa2000153, size 0x154) overlap with "/ig_naming/rx" (0xa2000000-0xa2007fff, size 0x8000).\n',
              },
            # Overlap a segment and memsection in virtual memory.
            { 'values':
              ('<memsection name="overlap" size="1M" virt_addr="0x80100000" />'),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Virtual Memory: Parts of "/ig_naming/overlap" (0x80100000-0x801fffff, size 0x100000) overlap with "/ig_naming/rx" (0x80100000-0x80107fff, size 0x8000).\n',
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])

    def test_kernel_proximity(self):
        """Test the kernel heap proximity checking code."""
        template = """
<image>
	<machine>
		<word_size size="0x20" />
		<virtual_memory name="virtual">
			<region base="0x80000000" size="0x50000000" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="0xa0000000" size="0x3800000" />
		</physical_memory>
		<page_size size="0x1000" />
		<page_size size="0x10000" />
		<page_size size="0x100000" />
                %s
	</machine>

	<virtual_pool name="virtual">
		<memory base="0x80000000" size="0x50000000"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory base="0xa0000000" size="0x3800000"/>
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
        %s
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
</image>
        """

        inputs = (
            # Defaults.  Should be within 64M of kernel.
            { 'values':
              ('', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            # Proximity equal to the default location.
            { 'values':
              ('<kernel_heap_attrs distance="1M" />', ''),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            # Very close proximity.
            { 'values':
              ('<kernel_heap_attrs distance="64K" />', ''),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical pool "physical": Could not place /kernel_heap within 0x10000 bytes of /kernel/rwx.\n',
              },

            # Heap outside the default size
            { 'values':
              ('', '<heap phys_addr="0xc0000000" />'),
              'exit_value': 1,
              'stdout': "",
              'stderr': 'Error: Physical pool "physical": Could not place /kernel_heap within 0x4000000 bytes of /kernel/rwx.\n'
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])

    def test_kernel_heap_alignment(self):
        """
        Test various combinations of directly mapping every segment
        in a program.
        """

        template = """
<image>
	<machine>
		<word_size size="0x20" />
		<virtual_memory name="virtual">
			<region base="0x1000" size="0xcffff000" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="0xa0000000" size="0x3800000" />
		</physical_memory>
		<page_size size="0x1000" />
		<page_size size="0x100000" />

                %s
	</machine>
	<virtual_pool name="virtual">
		<memory base="0x1000" size="0xcffff000"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory base="0xa0000000" size="0x3800000"/>
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming">
        </program>
</image>
        """

        inputs = (
            # Default alignment.  Heap 1M from the kernel.
            { 'values': (""),
              'exit_value': 0,
              'stdout': """\
VIRTUAL:
  <00001000:000013d3> /rootprogram/environment
  <00002000:00002fff> /rootprogram/main/stack
  <00003000:000034d3> /ig_naming/environment
  <00004000:00004fff> /ig_naming/callback
  <00005000:00005fff> /ig_naming/main/stack
  <00006000:00006fff> /bootinfo
  <00010000:0001ffff> /rootprogram/heap
  <00020000:0002ffff> /ig_naming/heap
  <00100000:0010b02c> /rootprogram/0
  <00113030:00116253> /rootprogram/1
  <80100000:80107e55> /ig_naming/rx
  <8010fe58:8010ffab> /ig_naming/rw
  <f0000000:f0028ad3> /kernel/rwx
  <f002c000:f002ffff> /kernel/rw
PHYSICAL:
  <a0000000:a0028ad3> /kernel/rwx
  <a0029000:a00293d3> /rootprogram/environment
  <a002a000:a002afff> /rootprogram/main/stack
  <a002b000:a002b4d3> /ig_naming/environment
  <a002c000:a002ffff> /kernel/rw
  <a0030000:a0083fff> /kernel_array
  <a0084000:a0084fff> /ig_naming/callback
  <a0085e58:a0085fab> /ig_naming/rw
  <a0086000:a0086fff> /ig_naming/main/stack
  <a0087000:a0087fff> /bootinfo
  <a0088000:a009302c> /rootprogram/0
  <a0095030:a0098253> /rootprogram/1
  <a00a0000:a00affff> /rootprogram/heap
  <a00b0000:a00b7e55> /ig_naming/rx
  <a00c0000:a00cffff> /ig_naming/heap
  <a0100000:a04fffff> /kernel_heap
""",
              'stderr': "",
              },
            # 4K alignment.  Heap next to the kernel.
            { 'values': ('<kernel_heap_attrs align="4K" />'),
              'exit_value': 0,
              'stdout': """\
VIRTUAL:
  <00001000:000013d3> /rootprogram/environment
  <00002000:00002fff> /rootprogram/main/stack
  <00003000:000034d3> /ig_naming/environment
  <00004000:00004fff> /ig_naming/callback
  <00005000:00005fff> /ig_naming/main/stack
  <00006000:00006fff> /bootinfo
  <00010000:0001ffff> /rootprogram/heap
  <00020000:0002ffff> /ig_naming/heap
  <00100000:0010b02c> /rootprogram/0
  <00113030:00116253> /rootprogram/1
  <80100000:80107e55> /ig_naming/rx
  <8010fe58:8010ffab> /ig_naming/rw
  <f0000000:f0028ad3> /kernel/rwx
  <f002c000:f002ffff> /kernel/rw
PHYSICAL:
  <a0000000:a0028ad3> /kernel/rwx
  <a0029000:a00293d3> /rootprogram/environment
  <a002a000:a002afff> /rootprogram/main/stack
  <a002b000:a002b4d3> /ig_naming/environment
  <a002c000:a002ffff> /kernel/rw
  <a0030000:a0083fff> /kernel_array
  <a0084000:a0483fff> /kernel_heap
  <a0484000:a0484fff> /ig_naming/callback
  <a0485e58:a0485fab> /ig_naming/rw
  <a0486000:a0486fff> /ig_naming/main/stack
  <a0487000:a0487fff> /bootinfo
  <a0488000:a049302c> /rootprogram/0
  <a0495030:a0498253> /rootprogram/1
  <a04a0000:a04affff> /rootprogram/heap
  <a04b0000:a04b7e55> /ig_naming/rx
  <a04c0000:a04cffff> /ig_naming/heap
""",
              'stderr': "",
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "--map", "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])

    def test_ignore_prog(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/ignore_program_test.xml", "-i", "(vmlinu.)|(ig_serial)|(ig_timer)", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")

        if not have_readelf:
            return

        # check excluded programs are not in the elf
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'vmlinu'")
        self.assertNotEquals(status, 0)
        self.assertEquals(output, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'ig_serial'")
        self.assertNotEquals(status, 0)
        self.assertEquals(output, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'ig_timer'")
        self.assertNotEquals(status, 0)
        self.assertEquals(output, "")

        #check the other stuff is still included
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'ig_nam'")
        self.assertEquals(status, 0)
        self.assertNotEquals(output, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'kernel'")
        self.assertEquals(status, 0)
        self.assertNotEquals(output, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'server'")
        self.assertEquals(status, 0)
        self.assertNotEquals(output, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'bootinfo'")
        self.assertEquals(status, 0)
        self.assertNotEquals(output, "")
#         status, output = commands.getstatusoutput("readelf -e test_output|grep 'segment_info'")
#         self.assertEquals(status, 0)
#         self.assertNotEquals(output, "")

    def test_program_header_offset(self):
        """Test the the ---program-header-offset option."""
        if not have_readelf:
            return

        # Check that a default merge puts the headers at the minimum offset,
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple.xml", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'Start of program headers'")
        self.assertEquals(status, 0)
        self.assertEquals(output, "  Start of program headers:          52 (bytes into file)")

        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/simple.xml", "--program-header-offset=4096", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        status, output = commands.getstatusoutput("readelf -e test_output|grep 'Start of program headers'")
        self.assertEquals(status, 0)
        self.assertEquals(output, "  Start of program headers:          4096 (bytes into file)")

    def test_dump_layout(self):
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/multiple_elf_file.xml", "--map", "-o", "test_output"])
        output = """\
VIRTUAL:
  <00001000:000013d3> /rootprogram/environment
  <00002000:00002fff> /rootprogram/main/stack
  <00003000:00012fff> /rootprogram/heap
  <00013000:000134d3> /ig_naming/environment
  <00014000:00014fff> /ig_naming/callback
  <00015000:00015fff> /ig_naming/main/stack
  <00016000:00025fff> /ig_naming/heap
  <00026000:00026fff> /bootinfo
  <00100000:0010b02c> /rootprogram/0
  <00113030:00116253> /rootprogram/1
  <80100000:80107e55> /ig_naming/rx
  <8010fe58:8010ffab> /ig_naming/rw
  <f0000000:f0028ad3> /kernel/rwx
  <f002c000:f002ffff> /kernel/rw
PHYSICAL:
  <a0000000:a0028ad3> /kernel/rwx
  <a0029000:a00293d3> /rootprogram/environment
  <a002a000:a002afff> /rootprogram/main/stack
  <a002b000:a002b4d3> /ig_naming/environment
  <a002c000:a002ffff> /kernel/rw
  <a0030000:a0083fff> /kernel_array
  <a0084000:a0483fff> /kernel_heap
  <a0484000:a048f02c> /rootprogram/0
  <a0490030:a0493253> /rootprogram/1
  <a0494000:a04a3fff> /rootprogram/heap
  <a04a4000:a04a4fff> /ig_naming/callback
  <a04a5000:a04ace55> /ig_naming/rx
  <a04ade58:a04adfab> /ig_naming/rw
  <a04ae000:a04aefff> /ig_naming/main/stack
  <a04af000:a04befff> /ig_naming/heap
  <a04bf000:a04bffff> /bootinfo
"""
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, output)

    def test_last_phys(self):
        "Test the -l option that prints the last physical address."
        exit_value, ret_stdout, ret_stderr = self._run_command(["data/eg_weave/multiple_elf_file.xml", "-l", "-o", "test_output"])
        self.assertEquals(exit_value, 0)
        self.assertEquals(ret_stderr, "")
        self.assertEquals(ret_stdout, "physical: 0xa04c0000\n")

    def test_zones(self):
        """Simple tests for memory zones."""
        template = """
<image>
        <machine>
                <word_size size="0x20" />
                <virtual_memory name="virtual">
                        <region base="0x1000" size="0xcffff000" />
                </virtual_memory>
                <physical_memory name="physical">
                        <region base="0xa0000000" size="0x3800000" />
                </physical_memory>
                <page_size size="0x1000" />
        </machine>
        <virtual_pool name="virtual">
                <memory base="0x1000" size="0xcffff000"/>
        </virtual_pool>

        <physical_pool name="physical">
                <memory base="0xa0000000" size="0x3800000"/>
        </physical_pool>

        <kernel file="data/eg_weave/l4kernel">
        </kernel>

        <rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
        </rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming">
               %s
        </program>
</image>
        """

        inputs = (
            # Simple zone, occupying the whole of a window.
            { 'values':
"""
                <zone name="z1" >
                        <memsection name="zoned_ms" size="0x100000" virt_addr="0x80600000" align="1048576"  />
                </zone>
""",
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Small memsection occupying a fixed address at the base
            # of a zone.
            { 'values':
"""
                <zone name="z1" >
                        <memsection name="zoned_ms" size="0x1000" virt_addr="0x80600000" align="1048576"  />
                </zone>
""",
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Small memsection to be allocated dynamically.
            { 'values':
"""
                <zone name="z1" >
                        <memsection name="zoned_ms" size="0x1000"  />
                </zone>
""",
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Multiple dynamically allocated memsections.
            { 'values':
"""
                <zone name="z1" >
                        <memsection name="zoned_ms" size="0x1000"  />
                        <memsection name="zoned_ms2" size="0x1000"  />
                        <memsection name="zoned_ms3" size="0x1000"  />
                </zone>
""",
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Multiple fixed zones in the same window.
            # of a zone.
            { 'values':
"""
                <zone name="z1" >
                        <memsection name="zoned_ms" size="0x1000" virt_addr="0x80600000"  />
                        <memsection name="zoned_ms1" size="0x1000" virt_addr="0x80604000"  />
                        <memsection name="zoned_ms2" size="0x1000" virt_addr="0x80608000"  />
                        <memsection name="zoned_ms3" size="0x1000" virt_addr="0x80680000"  />
                </zone>
""",
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Multiple zones in the same program
            { 'values':
"""
                <zone name="z1" >
                        <memsection name="zoned_ms" size="0x1000"  />
                </zone>
                <zone name="z2" >
                        <memsection name="zoned_ms" size="0x1000"  />
                </zone>
""",
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])

    def test_cache_policies(self):
        """Test the cache policy code."""
        template = """
<image>
	<machine>
		<word_size size="0x20" />

                <cache_policy name="user1" value="50" />
                <cache_policy name="user2" value="100" />
                
		<virtual_memory name="virtual">
			<region base="0x1000" size="0xcffff000" />
		</virtual_memory>
		<physical_memory name="physical">
			<region base="0xa0000000" size="0x3800000" />
		</physical_memory>
		<page_size size="0x1000" />
	</machine>
	<virtual_pool name="virtual">
		<memory base="0x1000" size="0xcffff000"/>
	</virtual_pool>

	<physical_pool name="physical">
		<memory base="0xa0000000" size="0x3800000"/>
	</physical_pool>

	<kernel file="data/eg_weave/l4kernel">
	</kernel>

	<rootprogram file="data/eg_weave/ig_server" virtpool="virtual" physpool="physical">
	</rootprogram>
        <program name="ig_naming" file="data/eg_weave/ig_naming">
        	<segment name="rx" cache_policy="%s" />
        	<memsection name="demo" size="4K" cache_policy="%s" />
        </program>
</image>
        """

        inputs = (
            # Iterate through the standard cache policies.  All should
            # work.
            { 'values': ('default', 'default'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('cached', 'cached'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('uncached', 'uncached'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('writeback', 'writeback'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('writethrough', 'writethrough'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('coherent', 'coherent'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('device', 'device'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('writecombining', 'writecombining'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Test the user specified cache policies.
            { 'values': ('user1', 'user1'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },

            { 'values': ('user2', 'user2'),
              'exit_value': 0,
              'stdout': "",
              'stderr': "",
              },
            # Test a non-existent policy name.
            { 'values': ('rubbish', 'rubbish'),
              'exit_value': 1,
              'stdout': "",
              'stderr': "Error: Unknown cache policy: 'rubbish'.\n",
              },
            )

        for value in inputs:
            exit_value, ret_stdout, ret_stderr = \
                        self._run_command(["-S", template % value['values'], "-o", "test_output"])
            self.assertEquals(ret_stderr, value['stderr'])
            self.assertEquals(ret_stdout, value['stdout'])        
            self.assertEquals(exit_value, value['exit_value'])
