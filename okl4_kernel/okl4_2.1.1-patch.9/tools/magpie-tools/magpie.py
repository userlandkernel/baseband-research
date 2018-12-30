#!/usr/bin/env python
#
# Australian Public Licence B (OZPLB)
# 
# Version 1-0
# 
# Copyright (c) 2007 National ICT Australia
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

import os, sys

from optparse import OptionParser

from targets.shared.target import Target
from targets.shared.options import options
from CAmkES.runner import CAmkES

import helper
helper.sanity_checks()

class MagpieOptionParser(object):
	class ParserError(Exception):
		pass
	def __init__(self):
		parser = OptionParser(usage = 'magpie.py [flags] <idl file>')
		parser.add_option('--target', type = 'string', dest = 'target',
				help = 'Specify a target.')
		parser.add_option('-c', '--camkes',action="store_const", dest='camkes', const= True,
				help="specify source/generation = CAmkES", default=False)
		parser.add_option('--list-targets', action = 'store_true', default = False,
				help = 'List all targets.', dest = 'list_targets')
		parser.add_option('--output', type = 'string', dest = 'output',
				help = 'Specify an output format.')
		parser.add_option('--list-outputs', action = 'store_true', default = False,
				help = 'List all outputs for a given target.', dest = 'list_outputs')
		parser.add_option('-I', type="string", action="append", dest="include_dirs",
				help="specify additional include directories", default=[])
		parser.add_option('--templates', type="string", dest = "templates_dir",
				help = "The base directory for templates")
		parser.add_option('--output-filename', type="string", dest="output_filename",
				help = "The output filename (or '-' for standard output)",
				default = '-')
		self.parser = parser
	
	def parse(self, cmdline):
		self.options, self.args = self.parser.parse_args(cmdline)

	def set_options(self):
		options.add_idl_files(self.args)
		if self.options.output:
			options['output_type'] = self.options.output
		if self.options.target:
			target_list = self.options.target.split(':')
			target_list.reverse()
			options['target'] = target_list
		options['hardware_arch'] = 'Generic 32' # FIXME
		if self.options.include_dirs == None:
			options['c_include_dirs'] = []
		else:
			options['c_include_dirs'] = self.options.include_dirs
		options['print_help'] = []
		options['output_filename'] = self.options.output_filename
		if self.options.templates_dir:
			options['template_base'] = self.options.templates_dir
		if self.options.list_targets:
			options['print_help'] = ['list_targets']
		if self.options.list_outputs:
			options['print_help'] = ['list_outputs']
		options['camkes'] = self.options.camkes
	
	def must_display_help(self):
		return self._has_help_option()

	def _has_help_option(self):
		for key in self.help_options:
			if self.options[key] is True:
				return True
	
	def _help_list_targets(self):
		print "Current targets:"
	
	def _help_list_outputs(self):
		print "Outputs for target '%s':" % ('ahar')

class Magpie(object):
	def __init__(self, cmdline):
		self.cmdline_parser = MagpieOptionParser()
		self.cmdline_parser.parse(cmdline)
	
	def run(self):
		"""
		"""
		self.cmdline_parser.set_options()
		if options['camkes']:
			camkes = CAmkES(options)
			camkes.run()
		else:
			target = Target()
			target.run()

def main():
	magpie = Magpie(sys.argv[1:])
	magpie.run()

if __name__ == '__main__':
	main()
