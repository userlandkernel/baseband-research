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
"""
These are Magpie generic options. These are set by magpie.py (magpie's native cmdline interface)
or by magpidl4.py (magpie's IDL4 compatibility interface).

If you are writing a new interface, you should examine the way magpie.py and magpidl4.py set
these options.
"""

import os
from magpie import helper

class enum(object):
	pass

class _internal_GenericOptions(object):
	def __init__(self):
		# options_set is False until the main driver program (eg magpidl4.py)
		# initialises all options.
		self.filenames = []
		self.supported_options = {#'type_inference_c': (bool, False),
			'output_filename': (str, '-'),
			'output_directory': (list, []),
			'output_type': (str, None),
			'target': (list, ['idl4/generic_l4v4']),
			'c_preprocessor': (str, 'cpp'),
			'idl_c_preprocessor': (str, 'cpp'),
			'cpp_options': (list, ['-E']),
			'c_include_dirs': (list, []),
			'c_defines': (list, []),
			'compiler_flags': (list, []),
			'hardware_arch': (str, 'Generic 32'),
			'generator': (str, 'idl4'),
			'path_to_package': (str, ''),
			'print_help': (list, []),
			'template_base': (str, ''),
			# Cache stuff
			'cache_enabled': (bool, True),
			'cache_dir': (str, None),
			'template_cache_dir': (str, None),
			'camkes': (bool, False),
			# Debugging stuff
			'debug': (list, []),
			# Commands related to prettifying the output. 
			#'indent_cmd': (str, 'indent'),
			#'indent_cmd_options': (str, '-kr -bfda -sob -i4 -ts4 -ut') # K&R minus personal annoyances
		}
		self.options = {}
		self._set_auto_options()
	
	def extend_option_enum(self, key, extras_list):
		the_enum = self.supported_options[key]
		assert the_enum[0] == enum
		self.supported_options[key] = \
			(enum, the_enum[1], the_enum[2] + extras_list)
		
	def _get_option(self, key):
		if key in self.options:
			return self.options[key]
		elif key in self.supported_options:
			return self.supported_options[key][1] # Return default value
		else:
			raise KeyError("Uknown option key %s" % (key))

	def __getitem__(self, key):
		return self._get_option(key)
	
	def __setitem__(self, key, value):
		if key not in self.supported_options:
			raise KeyError("Unknown option %s" % (key))
		if self.supported_options[key][0] == enum:
			if value not in self.supported_options[key][2]:
				raise TypeError, "Key %s referenced unknown enum %s" % (key, value)
		elif type(value) is not self.supported_options[key][0]:
			raise TypeError, "Type for key %s must be %s, not %s" %\
				(key, self.supported_options[key][0], type(value))
		self.options[key] = value
	
	def add_idl_files(self, files):
		self.filenames.extend(files)
		#self['c_include_dirs'].extend([os.path.dirname(idl_file) for idl_file in idl_files])

	def _set_auto_options(self):
		"""
		Set any automatic options.

		NB use self.options['whatever'] here rather than self['whatever'] because 
		otherwise __getitem__ complains that we shouldn't be reading these options
		yet.
		"""
		# Figure out the module path and add him.
		self['path_to_package'] = helper.get_module_base_path()
		# Add the default template path if it hasn't been specified on the command line.
		if self._get_option('template_base') == '':
			self['template_base'] = 'templates'
	

# Singleton
options = _internal_GenericOptions()

