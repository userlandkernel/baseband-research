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

from generic import BaseOutput

class V4Output(BaseOutput):
	"""
	Must conform to output interface. Python needs interfaces goddamnit
	"""
	def __init__(self, *args):
		BaseOutput.__init__(self, *args)
		self.context.file.arch_string = 'generic'
	
	### BEGIN output interface conformance

	### END output interface conformance

	def gen_copyright(self):
		self.fromtemplate('v4_generic/copyright.template.c')
	
	def gen_ifndef_magic_start(self, key):
		self.fromtemplate('v4_generic/ifndef.start.template.c', key=key)

	def gen_ifndef_magic_finish(self, key):
		self.fromtemplate('v4_generic/ifndef.finish.template.c', key=key)

	def gen_imports(self):
		self.fromtemplate('v4_generic/import.template.c')
	
	def gen_idl4_defines(self):
		self.fromtemplate('v4_generic/idl4.defines.c')
		
	def gen_constant_propagation(self):
		self.fromtemplate('v4_generic/constants.template.c')

	# Client
	def gen_client_interface_preamble(self, ifndef_objdef, objdef):
		self.fromtemplate('v4_generic/client.interface.preamble.generic.template.c',
			ifndef_objdef=ifndef_objdef, objdef=objdef)

	def gen_client_v4_function(self, function_ast):
		# Something of a cop-out?
		self.fromtemplate('v4_generic/client.function.generic.v4.template.c',
			ast=function_ast)

	# Service
	def gen_service_interface_preamble_generic_v4(self):
		self.fromtemplate('v4_generic/service.interface.preamble.generic.v4.template.c')

	def gen_service_v4_main(self):
		self.fromtemplate('v4_generic/service.functions.generic.v4.main.template.c')
	
	# Service template
	def gen_servicetemplate_function(self):
		self.fromtemplate('v4_generic/servicetemplate.function.implementation.template.c')
	
	def gen_servicetemplate_interface_vtable(self):
		self.fromtemplate('v4_generic/servicetemplate.interface.vtable.template.c')

	def gen_servicetemplate_mainloop(self):
		self.fromtemplate('v4_generic/servicetemplate.mainloop.template.c')
	
	def gen_servicetemplate_imports(self):
		self.fromtemplate('v4_generic/servicetemplate.imports.template.c')
	
	
def construct(options):
	return V4Output(options)

"""
Context dictionary.

file - per-file things
function - per-function things


file:
	- output_filename : current output filename
	- idl_filename    : source IDL filename
	- platform_string : "V4 Generic" or such.
	- version_string  : "multi_idl blah blah blah"
"""

