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

import glob
import sys
import os

from magpie import helper
from magpie.targets.shared.inputparser import ASTImportError, FileNotFoundError
from magpie.output.generic import GenericOutput
from magpie.generator.generator import ASTInfo
from magpie.targets.shared.options import options
from magpie.targets.shared.astgen import ASTGen
from magpieparsers.types.evaluator import evaluate, flatten

class CommandlineError(Exception):
	pass

class TargetReadError(Exception):
	pass

class Target(object):
	"""
	The target class:
		- Create a type registry
		- Create input, output, and generator classes
		- Generate ASTs
		- Generate output for ASTs.
		- Write the output somewhere
		- Pretty-print the output.
	
	Key instance variables:
		self.type_registry
		self.generator
		self.templates
		self.output
	"""
	def __init__(self):
		target_data = self.get_target_data(options['target'])
		#print target_data
		
		# Create the type registry.
		# self.type_registry = new_registry(options['hardware_arch'], options['generator'])
		self.hardware_arch = options['hardware_arch']
		self.generator_type = options['generator']
		#print target_data['Templates']
		self.templates = target_data['Templates']()
		self.output = GenericOutput()
		self.generator = target_data['Generator']
		if target_data['DataStorage'] is not None:
			self.dataStorage = target_data['DataStorage']()
		else:
			self.dataStorage = None
		
	
	def run(self):
		# Meat of the generator.
		# By this stage options have been parsed; the task is simply to read 
		# the relevant files, generate an AST, and then walk that AST
		# to produce the correct output files according to the options.
		if options['print_help']:
			# Print help only
			self._print_help(options['print_help'])
			return False
		else:
			# Run Magpie proper.
			self._sanity_check_cmdline(options)
			
			try:
				self.generate_asts()
				self.generate_output()
			except ASTImportError, e:
				print >>sys.stderr, "Import error: %s" % (str(e))
				if e.show_traceback is True:
					raise
				return False
			except Exception, e:
				raise
			return True
	
	def generate_asts(self):
		#print options.filenames
		assert len(options.filenames) == 1
		try:
			gen = ASTGen(self.hardware_arch, self.generator_type, cpp = options['c_preprocessor'],
					idl_cpp = options['idl_c_preprocessor'], cpp_options = options['cpp_options'],
					cpp_include_dirs = options['c_include_dirs'], cpp_defines = options['c_defines'])
			ast = gen(options.filenames[0])
		except FileNotFoundError:
			raise CommandlineError("The file %s does not exist" % (filename))

		self.ast = ASTInfo(ast)
	
	def generate_output(self):
		generator_inst = self.generator(self.ast)
		self.generator_go(generator_inst)
		output_str = self.output.getvalue()
		self.write_output(output_str)
		
	def generator_go(self, generator_inst):
		# Initialise the output context...
		self.output.context['evaluate'] = evaluate
		self.output.context['flatten'] = flatten
		self.output.context['templates'] = self.templates
		self.output.context['dataStorage'] = self.dataStorage
		self.output.context.file.version_string = helper.VERSION_STRING
		self.output.context['generator'] = generator_inst
		self.output.context.file.output_filename = options['output_filename']
		self.output.context.file.idl_filename = generator_inst.astinfo.get_filename()
		#print options['output_type']
		base_template = self.templates.get(options['output_type'])
		if base_template is None:
			raise Exception("Unknown output type '%s'. Specify a correct output type, or add support for this output type in your target." % (options['output_type']))
		
		self.output.go(base_template)
		
		
		
	def write_output(self, output_str):
		# FIXME: All of this (and generate_output?) should go somewhere generic
		if options['output_filename'] == '-':
			sys.stdout.write(output_str)
		else:
			if options['output_directory'] != []:
				full_filename = helper.get_OSPath(options['output_directory'] + [options['output_filename']])
			else:
				full_filename = options['output_filename']
			self.outputfile = file(full_filename, 'w')
			self.outputfile.write(output_str)
			self.outputfile.close()

	def get_target_data(self, target_name_list):
		"""
		Return a target options dictionary.

		Format: {'Templates': <object>,
			'Generator': <object>,
			'DataStorage': <object>}
		"""
		# We do *not* support target name inheritance any more.
		assert len(target_name_list) == 1
		target_name = target_name_list[0]
		# Pre-init the options with a very liberal set.
		target_opts = {'Templates': None,
			'Generator': None,
			'DataStorage':None}
		# We've initialised the options. Now add each target in sequence.
		target_pathname = self._get_target_data_produce_pathname(target_name)
		#print target_pathname, target_opts
		execfile(target_pathname, target_opts)
		#print target_opts
		# Sanity-check the data
		if target_opts.get('Templates', None) is None:
			raise TargetReadError("Target %s does not define a valid Templates class" %\
					(target_name))
		if target_opts.get('Generator', None) is None:
			raise TargetReadError("Target %s does not define a valid Generator" %\
					(target_name))
					
		#if target_opts.get('DataStorage', None) is None:
		#	raise TargetReadError("DataStorage %s does not define a valid DataStorage" %\
		#			(target_name))
		return target_opts

	def _get_target_path(self):
		return os.path.join(helper.get_module_base_path(), 'targets')

	def _get_target_data_produce_pathname(self, target_name):
		target_filename = '%s_target.py' % (target_name)
		target_pathname = os.path.join(self._get_target_path(), target_filename)
		return target_pathname
	
	def _sanity_check_cmdline(self, cmdline_options):
		if not isinstance(cmdline_options['output_type'], basestring):
			raise CommandlineError("Output type not specified.")
		if not cmdline_options.filenames:
			raise CommandlineError("IDL filename not specified.")


	def _print_help(self, help):
		printed = False
		if 'list_targets' in help:
			self._print_targets()
			printed = True
		if 'list_outputs' in help:
			pretty_target_list = ' : '.join(options['target'])
			print "Public outputs for target list %s:" % (pretty_target_list)
			if not hasattr(self.templates, 'public') or not self.templates.public:
				print " - None found."
			else:
				for public in self.templates.public:
					print " * %s" % (public)
			printed = True
		if not printed:
			print "No help to print (internal error?)"
	
	def _print_targets(self):
		targets = []
		for root, dirs, files in os.walk(self._get_target_path()):
			files = [os.path.join(root, afile) for afile in files if afile.endswith('_target.py')]
			targets.extend(files)
		targets.sort()
		print "Available targets:"
		target_path_length = len(self._get_target_path())
		for target in targets:
			# We assume that each target is contained in exactly one directory.
			path, name = os.path.split(target)
			dirname = os.path.split(path)[1]
			pretty_name = os.path.join(dirname, name)
			pretty_name = pretty_name.split('_target.py')[0]
			print ' * %s' %(pretty_name)


	
#	def imp(self, name):
#		""" Import the module 'name'. This workaround for weird __import__ 
#		behaviour comes straight out of the Python built-in-functions documentation.
#		"""
#		package_end = name.rindex('.')
#		package, module = name[:package_end], name[package_end + 1:]
#		mod = __import__(package, globals(), locals(), module)
#		mod = getattr(mod, module)
#		return mod
	

