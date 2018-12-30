#
# Australian Public Licence B (OZPLB)
# 
# Version 1-0
# 
# Copyright (c) 2004 National ICT Australia
# 
# All rights reserved. 
# 
# Developed by: Embedded Real-time and Operating Systems Group (ERTOS)
#               National ICT Australia
#               http://www.ertos.nicta.com.au
# 
# Permission is granted by National ICT Australia, free of charge, to
# any person obtaining a copy of this software and any associated
# documentation files (the "Software") to deal with the Software without
# restriction, including (without limitation) the rights to use, copy,
# modify, adapt, merge, publish, distribute, communicate to the public,
# sublicense, and/or sell, lend or rent out copies of the Software, and
# to permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
# 
#    # Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimers.
# 
#    # Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimers in the documentation and/or other materials provided
#       with the distribution.
# 
#    # Neither the name of National ICT Australia, nor the names of its
#       contributors, may be used to endorse or promote products derived
#       from this Software without specific prior written permission.
# 
# EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
# PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS", AND
# NATIONAL ICT AUSTRALIA AND ITS CONTRIBUTORS MAKE NO REPRESENTATIONS,
# WARRANTIES OR CONDITIONS OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO ANY REPRESENTATIONS, WARRANTIES OR CONDITIONS
# REGARDING THE CONTENTS OR ACCURACY OF THE SOFTWARE, OR OF TITLE,
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
# THE ABSENCE OF LATENT OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF
# ERRORS, WHETHER OR NOT DISCOVERABLE.
# 
# TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT SHALL
# NATIONAL ICT AUSTRALIA OR ITS CONTRIBUTORS BE LIABLE ON ANY LEGAL
# THEORY (INCLUDING, WITHOUT LIMITATION, IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHERWISE) FOR ANY CLAIM, LOSS, DAMAGES OR OTHER
# LIABILITY, INCLUDING (WITHOUT LIMITATION) LOSS OF PRODUCTION OR
# OPERATION TIME, LOSS, DAMAGE OR CORRUPTION OF DATA OR RECORDS; OR LOSS
# OF ANTICIPATED SAVINGS, OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR
# OTHER ECONOMIC LOSS; OR ANY SPECIAL, INCIDENTAL, INDIRECT,
# CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES, ARISING OUT OF OR IN
# CONNECTION WITH THIS LICENCE, THE SOFTWARE OR THE USE OF OR OTHER
# DEALINGS WITH THE SOFTWARE, EVEN IF NATIONAL ICT AUSTRALIA OR ITS
# CONTRIBUTORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH CLAIM, LOSS,
# DAMAGES OR OTHER LIABILITY.
# 
# If applicable legislation implies representations, warranties, or
# conditions, or imposes obligations or liability on National ICT
# Australia or one of its contributors in respect of the Software that
# cannot be wholly or partly excluded, restricted or modified, the
# liability of National ICT Australia or the contributor is limited, to
# the full extent permitted by the applicable legislation, at its
# option, to:
# a.  in the case of goods, any one or more of the following:
# i.  the replacement of the goods or the supply of equivalent goods;
# ii.  the repair of the goods;
# iii. the payment of the cost of replacing the goods or of acquiring
#  equivalent goods;
# iv.  the payment of the cost of having the goods repaired; or
# b.  in the case of services:
# i.  the supplying of the services again; or
# ii.  the payment of the cost of having the services supplied again.
# 
# The construction, validity and performance of this licence is governed
# by the laws in force in New South Wales, Australia.
#

import unittest
import tempfile
import optparse
import popen2
import shutil
import os
import sys
import re

from snakebag.testmenu import TestMenu

sys.path.append(os.getcwd())
import helper

from magpieparsers.types.builtin import create_basictype_ast
from targets.shared.inputparser import construct as inputparser_construct
from output.generic import GenericOutput
from generator.generator import ASTInfo
from generator.v4generator import V4Generator

HARDWARE_ARCH = 'Generic 32'
GENERATOR_TYPE = 'idl4'

PATH_TO_EXPECTED_OUTPUT = 'test/expected_output/'
FAILED_OUTPUT_PREFIX = 'test.fail.'

WORD_SIZES = ['32', '64']
# FIXME: Currently we do not test arm_biguuid
PLATFORMS = ['generic_biguuid', 'generic'] #, 'arm_biguuid']
OUTPUT_TYPES = ['c', 's', 't']  # Client, service, service template
STANDARD_CPP = 'gcc'

HACK_PREFIX = '../../'

CMD_PREFIX = "./magpidl4.py"
CMD_POSTFIX_TMPL = '-%(output_type)s %(input_filename)s -h - -i %(interface)s -fno-use-malloc -w %(word_size)s -p %(platform)s --with-cpp=%(selected_cpp)s'

INCLUDE_LIBS_IGUANA = """--nfdonly-ugly-includepath-hack=%s -I/usr/local/include -Ilibs/templates/include -Ilibs/l4/include -Ilibs/c/include -Ilibs/c/arch_include/arm -Ilibs/iguana/include -Ibuild/ig_server/libs/iguana/include -Ilibs/iguana/include -Ilibs/elf/include -Ilibs/hash/include -Ilibs/bit_fl/include -Ilibs/ll/include -Ilibs/circular_buffer/include -Ilibs/b_plus_tree/include -Ilibs/b_plus_tree/include -Ilibs/range_fl/include -Ilibs/l4e/include"""

#IGUANA_DEFINES = """-DARCH_IA32 -DMACHINE_PC99 -DENDIAN_LITTLE -D__L4_ARCH__=ia32 -DL4_ARCH_IA32 --cpp-flags="-U__SIZE_TYPE__ -U__PTRDIFF_TYPE__ -U__WCHAR_TYPE__ -E " """

STANDARD_DEFINES = """--cpp-flags="-E -" """
IGUANA_DEFINES = """-DARCH_IA32 -DMACHINE_PC99 -DENDIAN_LITTLE -D__L4_ARCH__=ia32 -DL4_ARCH_IA32 --cpp-flags="-E -" """

#Iguana uses this command line:
#PYTHONPATH=tools/install/lib tools/magpie/magpidl4.py -c libs/iguana/include/interfaces/iguana.idl4 --with-cpp=`which cpp` -Itools/magpie/include -Ilibs/templates/include -Ilibs/l4/include -Ilibs/l4/test -Ilibs/mutex/include -Ilibs/mutex/test -Ilibs/c/include -Ilibs/c/arch_include/ia32 -Ilibs/ll/include -Ilibs/ll/test -Ilibs/range_fl/include -Ilibs/range_fl/test -Ilibs/circular_buffer/include -Ilibs/circular_buffer/test -Ilibs/elf/include -Ilibs/elf/test -Ilibs/l4e/include -Ilibs/l4e/test -Ilibs/iguana/include -Ilibs/iguana/test -Ibuild/ig_server/libs/iguana/include -Ilibs/iguana/include -Ilibs/hash/include -Ilibs/hash/test -Ilibs/bit_fl/include -Ilibs/bit_fl/test -Ilibs/util/include -Ilibs/util/test -Ilibs/obj_alloc/include -Ilibs/obj_alloc/test -Ilibs/queue/include -Ilibs/b_plus_tree/include -Ilibs/b_plus_tree/test -Ilibs/b_plus_tree/include -Ilibs/b_plus_tree/test -DARCH_IA32 -DMACHINE_PC99 -DENDIAN_LITTLE -D__L4_ARCH__=ia32 -DL4_ARCH_IA32 -h build/ig_server/libs/iguana/include/interfaces/iguana_client.h -i v4nicta_n2 -fno-use-malloc -w 32 -p generic_biguuid

INCLUDE_LIBS_TEST = "-Itest/input"

STANDARD_CONFORM_REGEXES = [
	(r'Generated by Magpie \d+, .*', 'Generated by Magpie VERSION_REMOVED'),
	(r'Source file.*', 'Source file REMOVED_FOR_TESTING'),
]

# Automatically update test results.
# Set to True if you are 100% CONFIDENT that your changes are correct
ALWAYS_UPDATE_NEW = False
# PS: 100% CONFIDENT

# Automatically update test results if the file is new.
ALWAYS_UPDATE_IF_NOT_EXISTS = False

if ALWAYS_UPDATE_NEW is True:
	print "WARNING! You are going to UPDATE all tests even if they fail."
	print "If you don't want this, set ALWAYS_UPDATE_NEW in test/test.py to False."
	print "Type 'yes' to continue."
	yes = raw_input()
	if yes != 'yes':
		sys.exit(0)

if ALWAYS_UPDATE_IF_NOT_EXISTS is True:
	print "WARNING! You are going to UPDATE all tests when the target does not exist.."
	print "Type 'yes' to continue."
	yes = raw_input()
	if yes != 'yes':
		sys.exit(0)

class Skelly(object):
	def get_output(self, cmd, conform_regexes = None):
		""" Run 'cmd', return the output. """
		if conform_regexes is None:
			conform_regexes = STANDARD_CONFORM_REGEXES

		child_stdout_stderr, child_stdin = popen2.popen4(cmd)
		child_stdin.close()

		result = child_stdout_stderr.read()
		result = self.regex_process(conform_regexes, result)
		return result

	def regex_process(self, regexlist, text):
		for matcher, replacement_string in regexlist:
			text = re.sub(matcher, replacement_string, text)
		return text

class FullTests(Skelly):
	def front_to_back(self, cmdline, expected_output_name, conform_regexes = None):
		new_output = self.get_output(cmdline, conform_regexes)
		full_expected_output_name = os.path.join(PATH_TO_EXPECTED_OUTPUT, expected_output_name)

		menu = TestMenu()
		menu.go(cmdline, full_expected_output_name, new_output)

	def front_to_back_multiple(self, input_filename, include_libs_type = None,
			conform_regexes = STANDARD_CONFORM_REGEXES, selected_cpp = STANDARD_CPP,
			interface = 'v4', all_word_sizes = True, all_platforms = True,
			defines = None):
		cmd_head = CMD_PREFIX
		if include_libs_type is None or include_libs_type == 'TEST':
			include_libs = INCLUDE_LIBS_TEST
			include_libs_type = 'TEST'
			defines = STANDARD_DEFINES
		elif include_libs_type == 'IGUANA':
			include_libs = INCLUDE_LIBS_IGUANA % (TEST_IGUANA_PREFIX)
			if not defines:
				defines = IGUANA_DEFINES
		else:
			defines = STANDARD_DEFINES
		cmd_head += ' %s' % (include_libs)
		cmd_head += ' %s' % (defines)
		use_include_libs_text = 'includes_%s' % (include_libs_type)
		if all_platforms:
			platforms = PLATFORMS
		else:
			platforms = [PLATFORMS[0]]
		if all_word_sizes:
			word_sizes = WORD_SIZES
		else:
			word_sizes = [WORD_SIZES[0]]
		for output_type in OUTPUT_TYPES:
			for word_size in word_sizes:
				for platform in platforms:
					cmd_postfix = CMD_POSTFIX_TMPL % locals()
					cmd = '%s %s' % (cmd_head, cmd_postfix)
					output_filename = 'test_%s_%s_%s_%s_%s_%s' % (os.path.basename(input_filename), output_type, interface, word_size, platform, use_include_libs_text)
					self.front_to_back(cmd, output_filename, conform_regexes)

class KengeIguana(unittest.TestCase, FullTests):
	def test_full_iguana(self):
		self.front_to_back_multiple('test/input/iguana.idl4', include_libs_type = 'IGUANA',
				interface = 'v4nicta_n2')

class IDL4Basic(unittest.TestCase, FullTests):

	# Only use setUp() and tearDown() if necessary

	#def setUp(self):
	#	... code to execute in preparation for tests ...

	#def tearDown(self):
	#	... code to execute to clean up after tests ...
	
	
	#def test_structsimple(self):
	#	self.front_to_back_multiple('test/input/struct.idl')

	def test_inheritance(self):
		self.front_to_back_multiple('test/input/inheritance.idl')

	def test_no_include_dirs(self):
		self.front_to_back_multiple('test/input/simple.idl')

	def test_char_ptr_arg(self):
		self.front_to_back_multiple('test/input/char_ptr_arg.idl')

	def test_char_ptr_arg_2(self):
		self.front_to_back_multiple('test/input/char_ptr_arg_2.idl')

	def test_struct_params(self):
		self.front_to_back_multiple('test/input/struct_params.idl')

		# Test the old interface too. FIXME: Have to do this better.
		self.front_to_back('./magpidl4.py -i v4nicta_q1 -p generic_biguuid  -I test/input test/input/struct_params.idl', 'test_struct_params_nicta_q1api')

	def test_diverse_types(self):
		self.front_to_back_multiple('test/input/diverse_types.idl')
	
	def test_basic_c_types(self):
		self.front_to_back_multiple('test/input/idl_c_basic_types.idl')

#	def test_diverse_types_2(self):
#		self.front_to_back_multiple('test/input/diverse_types_2.idl')

	def test_const_propagation(self):
		self.front_to_back_multiple('test/input/const_propagation.idl')

	def test_const_propagation_fromheaders(self):
		self.front_to_back_multiple('test/input/const_propagation_fromheaders.idl')
	
	def test_const_propagation_extended(self):
		self.front_to_back('./magpidl4.py --constants test/input/const_propagation_extended.idl', 'test_const_propagation_extended')
	
	def test_default_output(self):
		self.front_to_back('./magpidl4.py test/input/simple.idl', 'test_default_output')
	
	def test_bitfields(self):
		self.front_to_back_multiple('test/input/bitfields.idl')
	
	def test_void_param(self):
		self.front_to_back_multiple('test/input/void_param.idl',
				all_word_sizes = False, all_platforms = False)
	
	def test_ppc64asm(self):
		self.front_to_back('./magpidl4.py test/input/ppc64asm.idl -I test/input', 'test_ppc64_asm')
	
	def test_annotations(self):
		self.front_to_back('./magpie.py --target=magpie/corba_tester --output=annotations test/input/annotations.idl', 'test_function_annotations')
	
	def test_byref_nicta(self):
		"""
		Ensure that the "time" parameter is passed by value from the service stub
		"""
		self.front_to_back('./magpidl4.py -w 32 -s -i v4nicta_n2 --platform generic test/input/timer.idl', 'test_byref_nicta')
	
	def test_byref_normal(self):
		"""
		Ensure that the "time" parameter is passed by reference from the service stub
		"""
		self.front_to_back('./magpidl4.py -w 32 -s -i v4 --platform generic test/input/timer.idl', 'test_byref_normal')
	
	def test_generic_iface(self):
		# Make sure all the command-line options work.
		self.front_to_back('./magpie.py --help', 'generic_iface_cmdline_help')
		self.front_to_back('./magpie.py --list-targets', 'generic_iface_cmdline_list_targets')
		self.front_to_back('./magpie.py --target=magpie/corba_tester --list-outputs', 'generic_iface_cmdline_list_outputs')
		self.front_to_back('./magpie.py --target=magpie/corba_tester --output=annotations --templates=test/test_templates test/input/annotations.idl', 'generic_iface_cmdline_templates_dir')
	
	def test_syntax_error(self):
		self.front_to_back('./magpidl4.py test/input/syntax_error.idl', 'syntax_error_idl')
		self.front_to_back('./magpidl4.py -Itest/input test/input/cpp_syntax_error.idl', 'syntax_error_cpp')
	
	def test_kernelmsg(self):
		self.front_to_back('./magpidl4.py --platform generic test/input/kernelmsg.idl', 'kernelmsg')

class MIGFrontendTest(unittest.TestCase, FullTests):
	def test_sample_types_pt_ast(self):
		self.front_to_back('python migtester.py --pt --ast test/input_mig/migsample.defs', 'test_migsample_pt_ast')

class CAmkESTest(unittest.TestCase, FullTests):
	pass

class BrokenTest(unittest.TestCase, FullTests):
	def test_complex_initialisers(self):
		"""
		C++ parser test. Complex initialisers are broken. Refer init_declarator
		and friends in parsers/cplusplus/antlr/astgen.py
		"""
		self.front_to_back('./ctester.py --ast test/input/complex_initialisers.h')
	
class MagpieTestResults(unittest.TestResult):
	def startTest(self, test):
		test_name = test.id().split('.')[-1]
		print test_name
	
	#def stopTest(self, test):
	#	print " Finished."

class SemanticChecksTest(unittest.TestCase, Skelly):
	def _expect_failure(self, cmdline, expected_string):
		result = self.get_output(cmdline)
		# FIXME: Ensure returncode is nonzero
		if expected_string not in result:
			print "* FAIL *"
		assert expected_string in result

	def test_unknown_type(self):
		self._expect_failure('./magpidl4.py test/input/semantic_checks/unknown_type.idl',
				"Semantic error: Unknown type 'bogus'")
	
	def test_name_clashe(self):
		self._expect_failure('./magpidl4.py test/input/semantic_checks/name_clash.idl',
				"Semantic error: Duplicate name in scope 'foo'")

CANNED_INTERFACE = """
[uuid(37)]
interface test {
	%s func (%s);
};
"""
class ParameterTests(unittest.TestCase):
	"""
	Generator-only tests.
	"""
	def _rewrite_grid(self, function, directions):
		result = []
		for grid_dict in function.grid_parameters(directions):
			text = "box %d:%d " % (grid_dict['box_number'], grid_dict['start_of_box'])
			text += "param %s:%d-%d" % (grid_dict['param_name'], grid_dict['start_of_parameter'], grid_dict['bits_in_box'])
			result.append(text)

		return result

	def test_gridparameters(self):
		GRIDPARAMS = [
			('void', 'in int arg', ['box 0:0 param arg:0-32'], []),
			('void', 'in int arg1, in int arg2', ['box 0:0 param arg1:0-32', 'box 1:0 param arg2:0-32'], []),
			('short', 'in short arg1, in int arg2, out short arg3', ['box 0:0 param arg1:0-16', 'box 1:0 param arg2:0-32'], ['box 0:0 param arg3:0-16', 'box 0:16 param __retval:0-16']),
			('int', 'in int y', ['box 0:0 param y:0-32'], ['box 0:0 param __retval:0-32']),
		]
		inputparser = inputparser_construct()
		for retval, testparams, output_dirin, output_dirout in GRIDPARAMS:
			# Create the test file, so the rest of the system can survive...
			# We use mkstemp because Windows NT can't reopen regular temp files
			test_string = CANNED_INTERFACE % (retval, testparams)
			handle, pathname = tempfile.mkstemp()
			try:
				os.write(handle, test_string)
				os.close(handle)

				# Now to create the AST
				basicast = create_basictype_ast(HARDWARE_ARCH, GENERATOR_TYPE)
				ast = inputparser.produce_ast(pathname, 'CORBA', basicast)
				astinfo = ASTInfo(ast)

				# Get a generator
				generator = V4Generator(GenericOutput(), astinfo)

				# And walk to the function.
				interface = list(generator.get_interfaces())[0]
				function = list(interface.get_functions())[0]
				marshal = self._rewrite_grid(function, ['in', 'inout'])
				unmarshal = self._rewrite_grid(function, ['inout', 'out', 'return'])

				fail = False

				if marshal != output_dirin:
					print "\nmarshal_c: %s\nvs %s\n" % (marshal, output_dirin)
					fail = True

				if unmarshal != output_dirout:
					print "\nunmarshal_c: %s\nvs %s\n" % (unmarshal, output_dirout)
					fail = True

				self.assert_(marshal == output_dirin and unmarshal == output_dirout)

			finally:
				os.unlink(pathname)

def _hack_add_options(options):
	__builtins__.TEST_IGUANA_PREFIX = options.iguana_path # FIXME: giant hack
	__builtins__.TEST_NONINTERACTIVE = options.noninteractive

SUITES = {'idl4basic': {'name': 'IDL4Basic', 'class': IDL4Basic},
	'iguana': {'name': 'Iguana', 'class': KengeIguana},
	'mig': {'name': 'MIG', 'class': MIGFrontendTest},
	'camkes': {'name': 'CAmkES', 'class': CAmkESTest},
	'broken': {'name': 'Broken', 'class': BrokenTest},
	'params': {'name': 'Parameters', 'class': ParameterTests},
	'semchecks': {'name': 'Semantic checks', 'class': SemanticChecksTest},
}

def add_all_tests(options, suite):
	for opt_name in SUITES.keys():
		if getattr(options, 'test_' + opt_name):
			info = SUITES[opt_name]
			print 'Adding test suite %s' % (info['name'])
			suite.addTest(unittest.makeSuite(info['class']))

def add_specified_test(options, suite):
	testonly = 'test_' + options.testonly
	for testclasses in SUITES.values():
		testcase = testclasses['class']
		func = getattr(testcase, testonly, None)
		if func:
			suite.addTest(testcase(testonly))
			break

def remove_caches():
	# Remove .pyc files
	deleted = 0
	for root, dirs, files in os.walk('.'):
		if "{arch}" in root or "_darcs" in root:
			continue # stupid blah
		for afile in files:
			pathname = os.path.join(root, afile)
			if pathname.endswith('.pyc'):
				deleted += 1
				os.unlink(pathname)
	if deleted:
		print "Deleted .pyc files (%d total)" % (deleted)
	
	cache_dir = helper.get_magpie_dir('cache_dir', None)
	template_dir = helper.get_magpie_dir('template_cache_dir', None)
	cache_files_deleted = 0
	for some_dir in (cache_dir, template_dir):
		if os.path.exists(some_dir):
			for dir_path, dir_names, file_names in os.walk(some_dir, topdown = False):
				for file_name in file_names:
					path_name = os.path.join(dir_path, file_name)
					os.remove(path_name)
					cache_files_deleted += 1
	print "Deleted cache files (%d total)" % (cache_files_deleted)
	
def main():
	parser = optparse.OptionParser()
	parser.add_option('-4', '--test-idl4', dest = 'test_idl4basic',
			action = 'store_true', default = False, help = 'Test suite: IDL4-style basic tests')
	parser.add_option('-i', '--test-iguana', dest = 'test_iguana',
			action = 'store_true', default = False, help = 'Test suite: Iguana')
	parser.add_option('-m', '--test-mig', dest = 'test_mig',
			action = 'store_true', default = False, help = 'Test suite: MiG')
	parser.add_option('-c', '--test-camkes', dest = 'test_camkes',
			action = 'store_true', default = False, help = 'Test suite: CAmkES')
	parser.add_option('-b', '--test-broken', dest = 'test_broken',
			action = 'store_true', default = False, help = 'Test suite: currently-broken tests')
	parser.add_option('--semchecks', dest = 'test_semchecks',
			action = 'store_true', default = False, help = 'Test suite: Semantic checks')
	parser.add_option('--params', dest = 'test_params',
			action = 'store_true', default = False, help = 'Test suite: Parameters')
	parser.add_option('--iguana-path', dest = 'iguana_path', action = 'store', default = HACK_PREFIX)
	parser.add_option('--test', dest = 'testonly', action = 'store', default = None)
	parser.add_option('--noninteractive', dest = 'noninteractive', action = 'store_true', default = False)
	options, args = parser.parse_args()
	
	suite = unittest.TestSuite()
	if options.testonly is None:
		add_all_tests(options, suite)
	else:
		add_specified_test(options, suite)
	_hack_add_options(options)

	remove_caches()

	results = MagpieTestResults()
	suite.run(results)
	for error in results.errors:
		print '\n'.join(error[1:])
	print "Errors:", len(results.errors)
	print "Failures:", len(results.failures)

if __name__ == '__main__':
	main()
