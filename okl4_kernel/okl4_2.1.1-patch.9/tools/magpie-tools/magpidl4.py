#!/usr/bin/env python
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
import sanity
# And now for our feature attraction...

import os.path, sys, traceback
from version import VERSION_STRING
from optparse import OptionParser

# Don't include anything else from this script's directory.
del sys.path[0]
from magpie.targets.shared.target import Target, CommandlineError
from magpie.targets.shared.options import options

from magpieparsers.error import SemanticError, RecognitionError

# Basic IDL parser.
# Parse IDL4-style options and send them along in a native format.

TARGET_MAP = {
	('idl4', 'okl4', 'v4nicta_n2', 'c'): ['okl/generic'],

	# The following targets are all obsolete are maintained to varying degrees.
	('idl4', 'generic', 'v4', 'c'): ['idl4/generic_l4v4'],
	('idl4', 'generic', 'v4nicta_n2', 'c'): ['nicta/generic_n2'],
	('idl4', 'generic_biguuid', 'v4', 'c'): ['idl4/generic_biguuid_l4v4'],
	('idl4', 'generic_biguuid', 'v4nicta_n2', 'c'): ['nicta/generic_biguuid_n2'],
	('idl4', 'nicta', 'v4nicta_n2', 'c'): ['nicta/generic_biguuid_n2_noabi'],
	('idl4', 'nicta_oldstyle', 'v4nicta_n2', 'c'): ['nicta/generic_biguuid_n2_noabi_compat'],
	('idl4', 'generic_biguuid', 'v4nicta_q1', 'c'): ['nicta/generic_biguuid_q1'],
}

def _unique(iterable):
	found = {}
	for key in iterable:
		if key not in found:
			found[key] = 1
			yield key

def _help_target(index):
	""" Return a list of TARGET_MAP at the specified index """
	return ', '.join(_unique([item[index] for item in TARGET_MAP.keys()]))

class IDL4OptionParser(object):
	def setup(self):
		# Don't include -h in the parser object because IDL4 uses -h for something else
		parser = OptionParser(add_help_option=False,
				usage = "%s [flags] <idl file>" % (sys.argv[0]))
		parser.add_option('-p', '--platform', type="string", dest='platform',
			help="specify platform (supported: %s)" % (_help_target(1)),
			default="nicta")
		parser.add_option('-m', '--mapping', type="string", dest='mapping',
			help="specify mapping (supported: %s)" % (_help_target(3)),
			default="c")
		parser.add_option('-i', '--interface', type="string", dest='interface',
			help="specify interface (supported: %s)" % (_help_target(2)),
			default="v4nicta_n2")
		parser.add_option('-s', '--service-stubs', action="store_const",
			dest="output_type", const='service', help="generate service stubs")
		parser.add_option('-c', '--client-stubs', action="store_const",
			dest="output_type", const='client', help="generate client stubs",
			default = 'client')
		parser.add_option('--service-headers', action = 'store_const',
			dest = 'output_type', const='serviceheaders',
			help = 'generate C service headers',
			default = 'client')
		parser.add_option('--constants', action="store_const",
			dest="output_type", const='constants', help='Generate constants')
		parser.add_option('-t', '--service-template', action="store_const",
			dest="output_type", const='servicetemplate', help="generate service template")
		parser.add_option('-w', '--word-size', type="int", dest="word_size",
			help="specify word size in bits (32, 64)", default=32)
		parser.add_option('--with-cpp', type="string", dest="cpp",
			help="specify the location of the C preprocessor", default="cpp")
		parser.add_option('--with-idlcpp', type="string", dest="idl_cpp",
			help="specify the location of the C preprocessor for IDL files", default="cpp")
		parser.add_option('-I', type="string", action="append", dest="include_dirs",
			help="specify additional include directories", default=[])
		parser.add_option('-D', type="string", action="append", dest="c_defines",
			help="specify additional defines", default=[])
		parser.add_option('-h', type="string", dest="dest",
			help="specify location of destination header file")
		#parser.add_option('-o', type="string", dest="dest",
		#	help="specify location of destination header file")
		parser.add_option('-f', type="string", action="append", dest="compilerflags",
			help="parameters to pass to the IDL compiler")
		parser.add_option('--nfdonly-ugly-includepath-hack', type="string",
			help="nfd's magic to convert relative to absolute paths; DO NOT USE",
			dest="ugly_includepath_hack")
		parser.add_option('-g', type="string", dest="generator",
			help="specify generator (supported: %s)" % (_help_target(0)),
			default="idl4")
		parser.add_option('--version', action='store_true', dest='print_version',
				help = 'Display the current Magpie version ("%s") and exit' % (VERSION_STRING))
		parser.add_option('--help', action='help')
		parser.add_option('--magpie-no-cache', action="store_false", dest='cache_enabled',
				default = True, help = 'Determine whether Magpie should use its cache.')
		parser.add_option('--magpie-cache-dir', type='string', dest='cache_dir',
				help = "Specify the cache directory (default is $HOME/.magpie/cache on unix)")
		parser.add_option('--magpie-template-cache-dir', type='string', dest='template_cache_dir',
				help = "Specify the template cache directory (default is $HOME/.magpie/templates on unix)")
		parser.add_option('--cpp-flags', type='string', dest='cpp_flags', default = '',
				help = "Specify flags to the C preprocessor")
		parser.add_option('--debug', type='string', dest = 'debug',
				action = 'append', help = 'Specify debugging options')
		parser.add_option('--develop', action = 'store_true', default = False, dest = 'develop',
				help = 'Enable development mode (explicit errors)')
		self.parser = parser

	def parse_options(self):
		self.options, self.args = self.parser.parse_args()
		self.options.c_defines += ['__IDL4__']
		return self.options, self.args
	
	def print_help(self):
		self.parser.print_help()

	def get_target(self):
		# Convert our platform, generator, interface, and mapping options into a "target"
		# for the real Magpie options.
		# Mapgie used to support target inheritance, but now does not.
		target = TARGET_MAP[(self.options.generator, self.options.platform,
				self.options.interface, self.options.mapping)]
		return target
	
	def set_options(self):
		#print idl4style_options
		if self.options.dest:
			options['output_filename'] = self.options.dest
		options['target'] = self.get_target()
		options['c_preprocessor'] = self.options.cpp
		options['idl_c_preprocessor'] = self.options.idl_cpp


		# For ADS compiler, must use armcc to compile .h as a C file.
                # Also must use different cflags.  Temp fix.  L4LINUX_TODO.
                # Revisit after new IDL is merged in since that has changed
                # a bit.  Also, we might need to add a new param to accept cflag
                # options on the command line
#		if (os.path.basename(self.options.cpp)) in ["armcpp", "tcpp"]:
#			self['c_preprocessor'] = "armcc"
#			self['cpp_options'] = ['-E -D__arm__ ' +
#                                               '-DARM_LITTLE_ENDIAN -D__ARMEL__']

		if self.options.include_dirs is not None:
			options['c_include_dirs'] = self.options.include_dirs
		else:
			options['c_include_dirs'] = []

		if self.options.c_defines is not None:
			options['c_defines'] = self.options.c_defines
		else:
			options['c_defines'] = []


		if self.options.compilerflags:
			options['compiler_flags'] = self.options.compilerflags
		if self.options.word_size:
			# FIXME: this looks alittle clunky...
			if self.options.word_size == 32:
				options['hardware_arch'] = 'Generic 32'
			elif self.options.word_size == 64:
				options['hardware_arch'] = 'Generic 64'
			else:
				raise Exception("Unknown word size specified!")
		#FIXME: MIG ist NOT a hardware arch. -> this should go somewere else
		if self.options.generator == 'mig':
			options['hardware_arch'] = 'MIG'
		options['cache_enabled'] = self.options.cache_enabled
		if self.options.cache_dir:
			options['cache_dir'] = self.options.cache_dir
		if self.options.template_cache_dir:
			options['template_cache_dir'] = self.options.template_cache_dir
		options['output_type'] = self.options.output_type
		options['cpp_options'] = self.options.cpp_flags.split(' ')
		options['generator'] = self.options.generator
		if self.options.debug:
			options['debug'] = self.options.debug
		

def main():
	parser_inst = IDL4OptionParser()
	parser_inst.setup()

	cmdlineoptions, cmdlineargs = parser_inst.parse_options()
	if cmdlineoptions.print_version:
		print VERSION_STRING
		sys.exit(0)
	if cmdlineargs == []: # No files, so exit
		parser_inst.print_help()
		print >>sys.stderr, "\nExiting because no IDL files were specified.\n"
		sys.exit(1)
	if cmdlineoptions.ugly_includepath_hack is not None:
		cmdlineoptions.include_dirs = [os.path.join(cmdlineoptions.ugly_includepath_hack, i)
			for i in cmdlineoptions.include_dirs]
	parser_inst.set_options()

	options.add_idl_files(cmdlineargs)
	options['target'] = parser_inst.get_target()
	target_inst = Target()

	result = False
	try:
		result = target_inst.run()
	except Exception, e:
		if parser_inst.options.develop:
			raise
		else:
			print_error(e)

	if result is False:
		print >>sys.stderr, "*** Magpie did not complete successfully."
		sys.exit(1)
	else:
		sys.exit(0)

def print_error(e):
	if isinstance(e, CommandlineError):
		print >>sys.stderr, "*** Commandline error: %s" % (e.args[0])
	elif isinstance(e, SemanticError):
		print >>sys.stderr, str(e)
	elif isinstance(e, RecognitionError):
		print >>sys.stderr, str(e)
	else:
		# Write the traceback to a file.
		handle = file('traceback.error.out', 'w')
		traceback.print_exc(file = handle)
		handle.close()
		# Print a helpful message.

		print >>sys.stderr, """*** Magpie error.
Magpie encountered an error and can't continue. If you think this is a problem
with Magpie, please send traceback.error.out and inputparser.error.out (if it
exists) along with a test case to reproduce the problem (if possible) to
nfd@cse.unsw.edu.au."""
"""

<blog>

OMG! *blog* *blog* *blog*

DWIM!@!!!!!!1111one oneone

Hahfahrahrharahrhar

You would not belivee what l33t stuff th1s k0hd is going to do!!!

r = make_program_go_now( arguments )

</blog>
"""
	
if __name__ == '__main__':
	main()
