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

raise ObsoleteError()
from generator import v4generator
from idltypes import builtin, idltype
from debugging import debug_iguana_generator

basic_conversions = {
	'char': 'hhi',
	'signed char': 'hhi',
	'int': 'i',
	'unsigned int': 'x',
	'unsigned': 'x',
	'short': 'hi',
	'unsigned short': 'hx',
	'long': 'li',
	'unsigned long': 'lx',
	'long long': 'lli',
	'unsigned long long': 'llx',
	'float': 'f',
	'double': 'e',
	'long double': 'Le',
	'octet': 'i',
}

class IguanaV4DebugServiceGenerator(v4generator.V4ServiceGenerator):
	""" Annotate functions with the sort of type info needed by 'printf' """

	def iguana_get_info_for_simple_type(self, thetype):
		debug_iguana_generator("basic - %s" % thetype.name)
		format_string = basic_conversions[thetype.name]
		access = thetype.var_name
		description = thetype.var_name
		return ([format_string], [description], [access])
	
	def iguana_get_info_for_bitfield_type(self, thetype):
		debug_iguana_generator("bitfield - %s" % thetype.name)
		# FIXME: Perhaps formalise this interface?
		real_type = thetype.basic_type_inst
		real_type_info = self.iguana_get_info_for_random_type(real_type)
		# FIXME: For some reason GCC ignores the original type 
		# of bitfields and just treats them all as ints (?):
		return ('i', real_type_info[1], real_type_info[2])
	
	def iguana_get_info_for_alias_type(self, thetype):
		debug_iguana_generator("alias - %s" % thetype.name)
		real_type = thetype.real_type_instance
		return self.iguana_get_info_for_random_type(real_type)
	
	def iguana_get_info_for_constructed_type(self, thetype):
		# FIXME: Seedy knowledge of other classes as per bitfield_type?
		debug_iguana_generator("constructed - %s" % thetype.name)
		format_string_list = []
		description_list = []
		access_list = []
		for member in thetype.members_inst:
			new_fmtstring_list, new_desc_list, new_ac_list \
				= self.iguana_get_info_for_random_type(member)
			format_string_list.extend(new_fmtstring_list)
#			if isinstance (thetype, builtin.IDLTypeUnionBase):
#				description_list.extend(new_desc_list)
#				access_list.extend(new_ac_list)
#			else:
			# Not an union class; we need to pass through it...
			description_list.extend(['%s.%s' % (thetype.var_name, desc) for desc in new_desc_list])
			access_list.extend(['%s.%s' % (thetype.var_name, ac) for ac in new_ac_list])
		return (format_string_list, description_list, access_list)
	
	def iguana_get_info_for_random_type(self, thetype):
		debug_iguana_generator("random - %s" % thetype.name)
		# We understand four fundamental types of types: simple, bitfield, alias, and constructed.
		if isinstance(thetype, builtin.IDLTypeBitfieldMemberBase):
			# No wuzzas.
			return self.iguana_get_info_for_bitfield_type(thetype)
		elif isinstance(thetype, idltype.IDLTypeSimple):
			# Still no wuzzas.
			return self.iguana_get_info_for_simple_type(thetype)
		elif isinstance(thetype, idltype.IDLTypeAlias):
			# No real wuzzas.
			return self.iguana_get_info_for_alias_type(thetype)
		elif isinstance(thetype, idltype.IDLTypeConstructed):
			# Okay, now we have wuzzas.
			return self.iguana_get_info_for_constructed_type(thetype)
		else:
			# Too many wuzzas.
			raise Exception("Unknown idl type-type %s" % (type(thetype)))
	
	def satisfactory_param(self, param):
		if param['direction'] == 'in' and param['flags'] == []:
			return True
		return False

	def iguana_generate_calling_parameter_string(self):
		# Make life easier by doing this here rather than in the template.
		# Basically we want to print each parameter out using printf. To
		# do this gets complicated if we have complicated structures.
		# We want three lists:
		format_string_list = [] # - List of format specifiers ['%d', '%f']
		description_list = [] # - List of param names ['struct.p1', 'struct.p2']
		access_list = [] # - List of dereferences to get to the data ['struct.p1', 'struct.p2']
		for param in self.output.context.function.call_params:
			if self.satisfactory_param(param):
				thetype = param['idltype']
				debug_iguana_generator('* Decoding new type')
				new_format_string_list, new_description_list, new_access_list \
					= self.iguana_get_info_for_random_type(thetype)
				debug_iguana_generator("  decoded to %s" % new_access_list)
				format_string_list.extend(new_format_string_list)
				description_list.extend(new_description_list)
				access_list.extend(new_access_list)
		return format_string_list, description_list, access_list
		# FIXME: Magical targets such as pagefaults don't get handled properly...
		
	def hook_function_IDL4_PUBLISH_begin(self):
		# Print out the calling parameters.
		format_string_list, description_list, access_list \
			= self.iguana_generate_calling_parameter_string()
		fc = self.output.context.function
		fc.iguana_debug_format_string_list = format_string_list
		fc.iguana_debug_description_list = description_list
		fc.iguana_debug_access_list = access_list
		self.output.gen('iguana.service.functions.generic.v4.41IDL4_PUBLISH_FUNC_print_params.template.c')
	
	def hook_function_IDL4_PUBLISH_end(self):
		""" Display what we're returning, too"""
		self.output.gen('iguana.service.functions.generic.v4.49IDL4_PUBLISH_FUNC_print_return_param.template.c')

class IguanaGeneratorController(v4generator.V4GeneratorController):
	service_generator = IguanaV4DebugServiceGenerator
	#client_generator is unchanged

def construct(*args):
	return IguanaGeneratorController(*args)
