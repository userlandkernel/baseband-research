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

# V4 generator. 

# Laundry list: Things to remove from here - possibly move to templates, possibly
# specialise to V4-specific generator.
#  - ArchitectureInfo passed in.
#  - annotate_parameters_with_message_registers and the whole ArchitectureInfo
#    concept is very L4-specific. Perhaps another level.
#  - create_register_map_from_simplified_params
#  - annotate_parameters_with_function_modifiers
#  - get_ktable_functions
#  - get_untyped_words_*
#  - Things like output.gen_client_v4_function ...

import math

from magpie.debugging import debug_generator
from magpie.helper import c_friendly_string
from magpie.generator.simple_params import SimpleParam
from magpie.generator.wordsize import ArchitectureInfo, WordSizeAnnotator
from magpie.generator.generator import Generator
from magpie.targets.shared.options import options
from magpieparsers.types import infogripper
from magpieparsers.types.evaluator import evaluate
from magpieparsers.parser_common import Node

class Error(Exception):
	pass

# FIXME: Create architecture info somewhere better.
arch_info = ArchitectureInfo()

# FIXME: This should be configurable...
from magpie.abi.nicta_n2 import N2ABI as ABI

def get_function_name(interface_name, function_ast):
	scopelist = infogripper.get_full_scope_string(function_ast)
	scopelist.append(function_ast.leaf)
	return '_'.join(scopelist)
	#return '%s_%s' % (interface_name, function_ast)

def get_abi():
	return ABI(arch_info)
	

class V4Function(object):
	directions_in = ['in', 'inout']
	directions_out = ['out', 'inout', 'return']
	directions_all = ['in', 'inout', 'out', 'return']

	BYREF_STYLE = 'magpie'
	
	def __init__(self, function_ast = None, interface = None, function_num = None):
		function_ast = self._rename_params(function_ast)

		self.ast = function_ast
		self.decl_ast = self.ast
		self.interface = interface
		self.number = function_num
		self.simplified_params = self.make_simple_param_list(self.decl_ast)
		self._annotate_parameters(self.simplified_params)
		self.abi = get_abi()
		#word_annotator = WordSizeAnnotator(self.simplified_params, self.arch_info)
		#word_annotator.annotate()

	def _rename_params(self, ast):
		if self.get_is_pagefault(ast):
			new_ast = ast.copy()

			new_ast.children = []

			for child in ast.children:
				if child.name != 'parameter':
					new_ast.children.append(child.copy())
				elif child.the('target').the('type').leaf != 'fpage':
					new_ast.children.append(child.copy())
				else:
					# Create a new magical node with the correct name...
					fp_param = Node(new_ast, 'parameter', leaf = child.leaf)
					fp_param.add_attribute('direction', child.get_single_attribute('direction'))
					target = Node(fp_param, 'target')
					target.add_child(infogripper.getTypenode('idl4_mapitem', ast))
					fp_param.add_child(target)

					new_ast.children.append(fp_param)

			return new_ast
		else:
			return ast

	def make_simple_param_list(self, function_decl_ast, simpleparam_cls = SimpleParam):
		""" Return a list of SimpleParam objects to pass to the templates, so
		template code doesn't have to know how to walk the AST."""
		
		# Create the list of simplified params
		simple_param_list = []
		
		# ... create each positional parameter
		parameter_decl_list = function_decl_ast['parameter']
		for parameter_decl in parameter_decl_list:
			newparam = simpleparam_cls()
			newparam.init_fromdecl(parameter_decl, self.param_is_byref(parameter_decl))
			simple_param_list.append(newparam)
			
		# ... create the return type (special case)
		newparam = simpleparam_cls()
		newparam.init_asreturn(function_decl_ast)
		simple_param_list.append(newparam)
		# ... done.
		return simple_param_list

	# For client
	def get_name(self):
		scope_list = infogripper.get_full_scope_string(self.interface)
		assert self.interface.leaf is not None
		assert isinstance(scope_list, list)
		func_scope = scope_list
		func_scope.append(self.interface.leaf)
		func_scope.append(self.ast.leaf)
		return '_'.join(func_scope)

	def get_name_raw(self):
		decl_ast = self.ast
		return decl_ast.leaf

	def set_name_raw(self, new_name):
		self.ast.leaf = new_name

	def get_return_type(self, side='client'):
		#print 'gt_return_type: ', self.decl_ast.type
		return_type = self.decl_ast.maybe_walk_to('return_type', 'target', 'type')
		if not return_type:
			self.decl_ast.print_tree()
			assert False
		#print 'target_type = ', self.decl_ast.the('return_type').get_attribute('target_type')[0]
		indirection = self.decl_ast.the('return_type').get_attribute('indirection', [''])[0]
		type_name = infogripper.get_param_type_spec(return_type, side)
		return '%s%s' % (type_name, indirection)
	
	def is_nonvoid(self, side = 'client'):
		return self.get_return_type(side) != 'void'

	def get_params_direction(self, direction_list, flag = None):
		result = []
		for direction in direction_list:
			for param in self.simplified_params:
				if flag is not None and flag not in param['flags']:
					continue
				if param['direction'] != direction:
					continue
				result.append(param)
		return result

	def get_params(self, flag = None, ignore = None):
		result = []
		for param in self.simplified_params:
			if flag is not None and flag not in param['flags']:
				continue
			if ignore is not None and ignore in param['flags']:
				continue
			result.append(param)
		return result

	def get_params_in(self):
		#print "Get params in *",self.get_params(direction_list = ['in', 'inout'], flag = 'call'), "*"
		return self.get_params_direction(['inout', 'in'], flag = 'call')

	def get_params_out(self):
		return self.get_params_direction(['inout', 'out'], flag = 'call')

	def get_params_inout(self):
		return self.get_params_direction(['inout'], flag = 'call')

	def get_param_return(self):
		result = self.get_params_direction(['return'])
		if result:
			assert len(result) == 1
			return result[0]
		return None

	def get_call_params(self):
		return self.get_params(flag = 'call')

	def get_fpages_count(self):
		count = 0
		for item in self.simplified_params:
			if 'fpage' in item['flags']:
				count += 1
		return count

	def get_is_pagefault(self, decl_ast = None):
		if decl_ast is None:
			decl_ast = self.decl_ast

		modifiers = []
		#print 'get_is_pagefault(): decl.ast = ', self.decl_ast
		for deco_node in decl_ast['decorator']:
			for modifier_ast in deco_node['annotation']:
				if modifier_ast.leaf == 'kernelmsg':
					if evaluate(modifier_ast.the('expression')) =='idl4::pagefault':
						return True
		return False

	def get_modifiers(self):
		modifiers = []
		for deco_node in self.decl_ast['decorator']:
			for modifier_ast in deco_node['annotation']:
				arg_ast = []
				for exp_node in modifier_ast['expression']:
					arg_ast.append(evaluate(exp_node))
				if arg_ast == []:
					modifiers.append( (modifier_ast.leaf, ()) )
				elif len(arg_ast) == 1:
					modifiers.append( (modifier_ast.leaf, arg_ast[0]) )
				else:
					modifiers.append( (modifier_ast.leaf, arg_ast) )
		return modifiers

	def get_number(self):
		return self.number

	def get_defable_name(self):
		return '_MAGPIE_' + self.get_name().upper()

	def get_ifdef_name(self):
		return '_funcdef___%s' % (c_friendly_string(self.get_name()))

	def get_priv_param_name(self):
		for param in self.simplified_params:
			if 'priv' in param['flags']:
				return param['name']
		return None

	def rename_args(self, type_remap):
		for param in self.simplified_params:
			#print 'trying to remap %s [%s] with dict: %s' %(param['name'], param['flags'], type_remap)
			for key in type_remap.keys():
				if key in param['flags']:
					param['typename'] = type_remap[key]
					param['type'].leaf = type_remap[key]
		
	
	def describe_parameter(self, ast):
		pf = self.get_is_pagefault()

		flags = []

		direction = ast.get_attribute('direction', [None])[0]

		if pf and ast.leaf == 'priv':
			flags = ['priv', 'call']
		elif pf and ast.the('target').the('type').leaf == 'fpage':
			flags = ['fpage', 'call', 'marshal']
		elif direction in ['in', 'out', 'inout']:
			flags = ['call', 'marshal']

		return flags

	def _annotate_parameters(self, params):
		# We only handle one sort of modifier right now
		for param in params:
			param['flags'] = self.describe_parameter(param['ast'])
	
	def get_ast_param_list(self, direction):
		# Bored with simple params now...
		for param_ast in self.ast['parameter']:
			param_direction = param_ast.get_single_attribute('direction')
			param_description = self.describe_parameter(param_ast)
			if param_direction in direction and 'marshal' in param_description:
				yield param_ast

		if 'return' in direction:
			return_ast = self.decl_ast.maybe_walk_to('return_type')
			if return_ast and return_ast.the('target').the('type').leaf != 'void':
				yield return_ast

	def _flat_struct(self, type_ast):
		for type_instance_ast in type_ast.the('members')['type_instance']:
			for param_ast in self.flat_param(type_instance_ast):
				yield param_ast

	def flat_param(self, some_ast):
		type_ast = some_ast.the('target').the('type')
		type_ast = infogripper.find_alias_target(type_ast)
		meta_type = type_ast.get_single_attribute('meta_type')

		# And work out what to do with it
		if meta_type == 'basic':
			yield [some_ast]
		elif meta_type == 'struct':
			for struct_member_ast in self._flat_struct(type_ast):
				yield [some_ast] + struct_member_ast
		elif meta_type == 'union':
			# FIXME: We just pick the first element - we should probably
			# marshal it bytewise.
			first_member = type_ast.the('members').children[0]
			for union_member_ast in self.flat_param(first_member):
				yield [some_ast] + union_member_ast
		elif meta_type == 'enum':
			# It's essentially an int -- hmm
			yield [some_ast]
		else:
			type_ast.print_tree()
			raise Exception("Unexpected type encountered.")

	def flat_param_list(self, directions):
		"""
		Find a list of paramaters appropriate to "direction" and return
		a flattened list (no structs). The list is of lists consisting
		of all parameters / types leading up to the flattened param.
		"""
		for param_ast in self.get_ast_param_list(directions):
			for flat_ast in self.flat_param(param_ast):
				yield flat_ast
	
	def _get_type_from_param(self, param_ast):
		type_ast = param_ast.the('target').the('type')
		type_ast = infogripper.find_alias_target(type_ast)
		return type_ast

	def _get_type_size(self, type_ast):
		meta_type = type_ast.get_single_attribute('meta_type')

		if meta_type == 'basic':
			result = type_ast.get_single_attribute('size')
		elif meta_type == 'alias':
			indirection = type_ast.get_attribute('indirection')
			if indirection and indirection[0]:
				result = arch_info.word_size_in_bits
			else:
				# Step down just one level (there may be more indirection)
				alias_target_ast = type_ast.the('target').the('type')
				result = self._get_type_size(alias_target_ast)
		elif meta_type == 'enum':
			# FIXME: This is a bit evil...
			int_type = infogripper.getBasicTypenode('signed int', type_ast)
			result = int_type.get_single_attribute('size')
		elif meta_type == 'union':
			member_types = [member.the('target').the('type')
					for member in type_ast.the('members')['type_instance'] ]

			# Choose the biggest element and return its size.
			largest = max([self._get_type_size(member)
					for member in member_types])

			result = largest
		elif meta_type == 'struct':
			member_types = [member.the('target').the('type')
					for member
					in type_ast.the('members')['type_instance'] ]

			# Add all the struct elements and return the size.
			total = sum([self._get_type_size(member)
					for member in member_types])

			result = total
		else:
			type_ast.print_tree()
			raise Exception("Unknown meta type")

		if not result:
			type_ast.print_tree()
			raise Exception()

		return result
	
	def _get_param_size(self, param_ast):
		if param_ast.get_attribute('indirection'):
			return arch_info.word_size_in_bits # FIXME: Assume ptr == word size...
		elif param_ast.name == 'type':
			return self._get_type_size(param_ast)
		else:
			type_ast = param_ast.the('target').the('type')
			return self._get_type_size(type_ast)

	def _type_is_variable_length(self, type_ast):
		return type_ast.leaf in ('smallstring',)

	def sort_params_for_marshal(self, direction):
		# Simple sort method: preserve order except that
		# variable-length parameters go at the end.
		fixed_params = []
		variable_params = []

		for asts in self.flat_param_list(direction):
			param_ast = asts[-1]
			param_type = self._get_type_from_param(param_ast)

			if self._type_is_variable_length(param_type):
				variable_params.append(asts)
			else:
				fixed_params.append(asts)

		return fixed_params + variable_params
			
	def _c_type_name(self, param_ast):
		type_ast = param_ast.the('target').the('type')
		# FIXME: This is near - duplicated in simple_params.py
		MAP = {'smallstring': 'char *'}
		type_name = MAP.get(type_ast.leaf, type_ast.leaf)
		type_name += param_ast.attribute('indirection', '')
		return type_name

	def _marshal_unmarshal(self, direction, prefix, startword, client_side):
		if direction == 'in':
			params = self.sort_params_for_marshal(['in', 'inout'])
		elif direction == 'out':
			params = self.sort_params_for_marshal(['inout', 'out', 'return'])
		else:
			raise Error("Unknown direction %s" % (direction))

		bytesperword = arch_info.word_size_in_bits / 8
		bytepos = startword * bytesperword

		in_varlength = False

		for param_info in params:
			final_type = self._get_type_from_param(param_info[-1])
			param_name = self.param_name_from_asts(param_info, client_side = client_side)

			# Create the type name
			type_name = self._c_type_name(param_info[-1])

			if self._type_is_variable_length(final_type):
				if in_varlength is False:
					yield(('set_varptr', (bytepos,)))
					in_varlength = True
				if final_type.leaf == 'smallstring':
					yield(('%s_smallstring' % (prefix), (param_name, type_name)))
				else:
					raise Error("Don't understand this type: %s" % (str(final_type)))
			elif final_type.get_single_attribute('meta_type') in ('basic', 'enum'):
				# Basic type such as an int
				# Results in something like marshal_32
				param_size = self._get_param_size(param_info[-1])
				store_cmd = '%s_%d' % (prefix, param_size)
				# ... align to the size of the parameter
				bytepos += self._align_natural(bytepos, param_size / 8)

				yield((store_cmd, (param_name, bytepos, type_name)))
				bytepos += (param_size / 8)
			else:
				raise Error("Don't understand this type: %s" % str(final_type))
	
	def _align_natural(self, bytepos, type_size):
		# Natural alignment of this word size to the nearest byte
		max_align = min(arch_info.word_size_in_bits / 8, type_size)
		if bytepos % max_align == 0:
			return 0
		else:
			return max_align - (bytepos % max_align)

	def marshal(self, direction, startword = 0, client_side = True):
		return self._marshal_unmarshal(direction, 'marshal', startword, client_side)
			
	def unmarshal(self, direction, startword = 0, client_side = True):
		return self._marshal_unmarshal(direction, 'unmarshal', startword, client_side)

	def has_varlength_params(self, direction):
		for param_info in self.sort_params_for_marshal(direction):
			final_type = self._get_type_from_param(param_info[-1])
			if self._type_is_variable_length(final_type):
				return True
		return False
	
	def has_varlength_params_in(self):
		return self.has_varlength_params(['in', 'inout'])
	
	def has_varlength_params_out(self):
		return self.has_varlength_params(['inout', 'out', 'return'])
	
	def _marshal_size_inwords(self, directions):
		bitsize = 0

		for param_info in self.sort_params_for_marshal(directions):
			final_type = self._get_type_from_param(param_info[-1])
			if self._type_is_variable_length(final_type):
				raise Error("Can't determine size of buffer")
			else:
				bitsize += self._get_param_size(param_info[-1])

		wordsize = int(math.ceil(float(bitsize) / arch_info.word_size_in_bits))

		return wordsize
	
	def marshal_size_inwords_in(self):
		return self._marshal_size_inwords(['in', 'inout'])
	
	def marshal_size_inwords_out(self):
		return self._marshal_size_inwords(['inout', 'out', 'return'])
		
	def aligned_parameters(self, directions):
		"""
		Returns a list of flattened, aligned parameters.

		Where param_asts is in the format returned by flat_param_list
		"""
		# FIXME: This is old and (now) obsolete.
		position = 0

		for param_asts in self.flat_param_list(directions):
			
			# Calculate alignment for the next one.
			final_type = self._get_type_from_param(param_asts[-1])
			type_size = self._get_param_size(param_asts[-1])

			if len(param_asts) == 1:
				parent_type = None
			else:
				parent_type = self._get_type_from_param(param_asts[-2])

			if param_asts[-1].get_attribute('indirection'):
				indirection = True
			else:
				indirection = False
				
			alignment = self.abi.align(indirection, final_type, parent_type)
	
			# Align the start of the variable with the variable's alignment.
			if position % alignment:
				position += (alignment - (position % alignment))

			yield {'asts': param_asts,
					'position': position,
					'length': type_size,
			}

			# And move on to the next one.
			position += type_size

	def grid_parameters(self, directions, start = None):
		"""
		Return a list of aligned parameters suitable for stuffing into
		word-sized boxes. Automatically splits params (and returns
		multiples) if they overflow a box... which means the same
		param may be referenced multiple times in a list.

		"""
		# FIXME: This is old, over-complicated, and (now) obsoleted by 'marshal_parameters'
		# above.
		if start is None:
			start = 0

		BOX_SIZE = arch_info.word_size_in_bits

		for param_info in self.aligned_parameters(directions):
			# We want five pieces of information.
			# - Box number to use
			# - Number of bits to put in this box
			# - Start of box
			# - Start of parameter
			# - Parameter name

			# The box number is derived from the parameter's bit position.
			box_number = param_info['position'] / BOX_SIZE

			# The start of the box is derived from the parameter's bit 
			# position too.
			start_of_box = param_info['position'] % BOX_SIZE

			# The number of bits to put in this box is the whole parameter,
			# if it fits, or as much parameter as we can to fill up the box,
			# if not.
			space_left_in_box = BOX_SIZE - start_of_box
			bits_in_box = min(param_info['length'], space_left_in_box)

			# The first time we encounter a given parameter, "start of parameter"
			# will be 0. The next time, it will equal "number of bits put in box".
			start_of_parameter = 0

			# Parameter name comes from the ASTs.
			param_name = self.param_name_from_asts(param_info['asts'])

			# If the data type is a pointer, we must cast it before doing any
			# shifts on it.

			if param_info['asts'][-1].get_attribute('indirection', [''])[0]:
				param_cast = 'L4_Word_t' # FIXME
			else:
				param_cast = ''

			yield {'box_number': box_number + start,
					'start_of_box': start_of_box,
					'bits_in_box': bits_in_box,
					'start_of_parameter': start_of_parameter,
					'param_cast': param_cast,
					#'param_name': param_name
					'asts': param_info['asts']
			}

			# If we just marshalled the whole parameter, we're done. But
			# if the parameter was larger than the space left in the box,
			# we have to spread the rest over more boxes.
			bits_remaining = param_info['length'] - bits_in_box
			assert bits_remaining >= 0

			start_of_parameter += bits_in_box
			while bits_remaining > 0:
				# Marshal the parameter into the next box.
				box_number += 1
				bits_in_box = min(bits_remaining, BOX_SIZE)
				yield {'box_number': box_number + start,
						'start_of_box': 0,
						'bits_in_box': bits_in_box,
						'start_of_parameter': start_of_parameter,
						'param_cast': param_cast,
						#'param_name': param_name
						'asts': param_info['asts']
				}
				start_of_parameter += bits_in_box
				bits_remaining -= bits_in_box
	
	def _get_marshal_start(self, directions):
		if 'in' in directions:
			return self.abi.client_marshal_box_start
		elif 'out' in directions:
			return self.abi.client_unmarshal_box_start
		else:
			raise Exception("Wanted 'in' or 'out'")

	def param_is_byref(self, param_asts):
		if isinstance(param_asts, list):
			ast = param_asts[0]
		elif isinstance(param_asts, Node):
			ast = param_asts
		else:
			raise Exception("Unknown parameter type %s" % str(param_asts))

		if self.BYREF_STYLE == 'magpie':
			return self.param_is_byref_magpie(ast)
		elif self.BYREF_STYLE == 'nicta_n2':
			return self.param_is_byref_nicta_n2(ast)
		else:
			raise Exception("Unknown byref style")
	
	def param_is_byref_magpie(self, ast):
		if ast.name == 'return_type':
			return False
		elif ast.get_single_attribute('direction') in ['out', 'inout'] \
				or self._get_param_size(ast) > arch_info.word_size_in_bits:
			return True
		else:
			return False
	
	def param_is_byref_nicta_n2(self, ast):
		type_ast = self._get_type_from_param(ast)

		if ast.name == 'return_type':
			return False
		elif ast.get_single_attribute('direction') in ['out', 'inout'] \
				or type_ast.get_single_attribute('meta_type') != 'basic':
			return True
		else:
			return False
		
	def param_name_from_asts(self, param_asts, client_side = True):
		result = []
		for ast in param_asts:
			if ast.name == 'return_type':
				result.append('__return')
			else:
				assert ast.leaf is not None
				result.append(ast.leaf)

		if client_side:
			if self.param_is_byref(param_asts):
				if len(result) == 1:
					return '*%s' % (result[0])
				elif len(result) == 2:
					return '->'.join(result)
				elif len(result) > 2:
					return '->'.join(result[:2]) + '.' + '.'.join(result[2:])
				else:
					raise Exception()
			else:
				return '.'.join(result)
		else:
			return '.'.join(result)
	
	def param_type_from_asts(self, param_asts, client_side = True, indirects = True):
		if indirects:
			indirection = param_asts[-1].get_attribute('indirection', [''])[0]
		else:
			indirection = ''
		return '%s %s' % (param_asts[-1].the('target').the('type').leaf, indirection)
	
class V4Interface(object):
	def __init__(self, interface_ast = None, decorators_list = None, interface_map = None):
		self.ast = interface_ast
		self.decorators_list = decorators_list
		self.exports_list = self.ast['function']
		self.inherits = self.create_inheritance_list(interface_map)
		
	def create_inheritance_list(self, interface_map):
		inherits = []
		if self.ast['inherits'] == None:
			return []
		for name in self.ast['inherits']:
			if interface_map.has_key(name.leaf):
				inherits.append(interface_map[name.leaf])
			else:
				print 'Inheritance error! No interface with name: ', name
				pass
		return inherits
		
		
	# public functions
	def get_functions(self, modifier = None):
		function_class = self._my_function_class()
		inher_exports = []
		for inher_num, inher_ast in enumerate(self.inherits):
			inher_exports.extend(inher_ast['function'])

		for function_num, export_ast in enumerate(inher_exports + self.exports_list):
			new_func = function_class(function_ast = export_ast,
					interface = self.ast,
					function_num = function_num)
			if modifier is not None and modifier not in new_func.get_modifiers():
				continue
			yield new_func

	def _get_handler(self, name):
		for decorator in self.ast['decorator']:
			annotation = decorator.the('annotation')
			if annotation.leaf == name:
				if annotation.children:
					return annotation.the('expression').get_single_attribute('value')
				else:
					return True

		return False

	def get_irq_handler(self):
		return self._get_handler('irq_handler')
	
	def get_exception_handler(self):
		return self._get_handler('exception_handler')
	
	def get_unknown_ipc_handler(self):
		return self._get_handler('unknown_ipc_handler')
	
	def get_async_handler(self):
		return self._get_handler('async_handler')

	def get_workloop_function(self):
		for decorator in self.ast['decorator']:
			annotation = decorator.the('annotation')
			if annotation.leaf == 'before_every_message':
				return annotation.the('expression').get_single_attribute('value')

		return None
	
	def get_server_loop_function(self):
		for decorator in self.ast['decorator']:
			annotation = decorator.the('annotation')
			if annotation.leaf == 'server_loop_name':
				return annotation.the('expression').get_single_attribute('value')
	
	def get_pagefault_functions(self):
		return self.get_functions(modifier = ('kernelmsg', 'idl4::pagefault'))
		
	def get_ifdef_name(self):
		return '_objdef___%s' % (self.get_name())

	def get_name(self):
		#print self.ast.leaf
		scopelist = infogripper.get_full_scope_string(self.ast)
		scopelist.append(self.ast.leaf)

		return '_'.join(scopelist)

	def get_uuid(self):
		uuid = self._get_uuid_ast()
		if uuid is not None:
			return evaluate(uuid.the('expression'))
		return None

	def get_annotations(self):
		decorators = {}
		for decorator in self.decorators_list:
			for arg in decorator.children:
				args = []
				#for exp in arg.children:	
				#args.append(arg.the('expression').leaf)
				if len(arg['expression']) != 0:
					#print 'get_annotations: ', arg['expression']
					for exp in arg['expression']:
						args.append(evaluate(exp))
				decorators[arg.leaf] = args
		return decorators

	# For service
	def get_function_names(self):
		func_names = []
		for function_ast in self.exports_list:
				func_scope = infogripper.get_full_scope_string(function_ast)
				func_scope.append(function_ast.leaf)
				func_names.append('_'.join(func_scope))
		return func_names

	def get_fid_mask(self, num_functions):
		# We want to mask off the bits used to specify function ID.
		# So if we have 5 functions we want to mask off bits 0, 1 and 2 
		# so we return 0x7. If we have 15 functions we want to mask off bits
		# 0, 1, 2, and 3 so we return 0xf.
		return int(2 ** math.ceil(math.log(num_functions, 2))) - 1

	
	def get_defined_constants(self):
		#return []
		
		# Walk to the top of the tree. FIXME: Pretty bodgy...
		top = self.ast
		while top.parent is not None:
			top = top.parent
		# FIXME: Why this AND get_constants in the generator?!?!?!?! -nfd 5/sep/05
		uuid_ast = self._get_uuid_ast()
		desired_name = uuid_ast.children[0].leaf
		for const_node in top['const']:
			if const_node.leaf == desired_name:
				return [ (desired_name, const_node.get_attribute('value')[0]) ]
		return []

	def _get_uuid_ast(self):
		# FIXME: smart nodes instead. CF similarly-named function in the annotator.
		for decorator in self.decorators_list:
			if decorator.children[0].leaf == 'uuid':
				return decorator.children[0]
		return None # didn't find a UUID.

	def _my_function_class(self):
		return V4Function

	def __repr__(self):
		return '<%s named %s at %x>' % (self.__class__.__name__, 'ahar', id(self))


class V4Generator(Generator):
	""" Code to generate V4 client stubs """
	def __init__(self, ast):
		Generator.__init__(self, ast)
		self.interface_map = {}
		

	def get_interfaces(self):
		# AST-walkin' time!
		interface_class = self._my_interface_class()
		#print 'interfaces = ', self.astinfo.get_definitions('interface')
		for interface in self.astinfo.get_definitions('type'):
			if interface.attribute('meta_type', None) == 'interface':
				self.interface_map[interface.leaf] = interface
				yield interface_class(interface_ast = interface,
						decorators_list = interface['decorator'],
						interface_map = self.interface_map)

	def get_constants(self):
		for container, defn in self.astinfo.get_definitions('const'):
			scopelist = infogripper.get_full_scope_string(defn)
			scopelist.append(defn.leaf)
			const_name = '_'.join(scopelist)
			const_value = defn.get_attribute('value')[0]
			yield (const_name, const_value)
			
	def get_imports(self):
		#print 'ast = ', self.ast.ast
		for impn in self.astinfo.ast['cimport']:
			yield impn.leaf

	def get_arch_info(self):
		return arch_info

	def get_idl_modulename(self):
		return options.filenames[0].split('.')[0].split('/')[-1]

	def get_idl_filename(self):
		""" Return the first IDL filename. """
		return options.filenames[0]
	
	def get_irq_handler_name(self):
		return self.get_idl_modulename() + "_irq_handler"
	
	def get_async_handler_name(self):
		return self.get_idl_modulename() + "_async_handler"

	def get_server_loop_name(self):
		for interface in self.get_interfaces():
			function = interface.get_server_loop_function()
			if function:
				return function

		# No annotation, use default
		return self.get_idl_modulename() + "_server_loop"
	
	def get_output_filename(self):
		return options['output_filename']

	def get_ifdefable_filename(self):
		return '__%s__' % (c_friendly_string (self.get_output_filename()))

	def search(self, *args, **kwargs):
		return list(self.astinfo.ast.search(*args, **kwargs))
	
	def get_pagefault_handler(self):
		handlers = []
		for interface in self.get_interfaces():
			for function in interface.get_functions():
				if function.get_is_pagefault():
					handlers.append(function)

		if handlers:
			assert len(handlers) == 1
			return handlers[0]
		else:
			return None
	
	def get_irq_handler(self):
		# We only have one of these per file, and it's externally-defined.
		handler = False

		for interface in self.get_interfaces():
			if interface.get_irq_handler():
				assert handler is False
				handler = True

		return handler

	def get_async_handler(self):
		# We only have one of these per file, and it's externally-defined.
		handler = False

		for interface in self.get_interfaces():
			if interface.get_async_handler():
				assert handler is False
				handler = True

		return handler
	
	def get_exception_handler(self):
		handler = False

		for interface in self.get_interfaces():
			handler = interface.get_exception_handler()
			if handler:
				return handler

		return handler

	def get_unknown_ipc_handler(self):
		handler = False

		for interface in self.get_interfaces():
			handler = interface.get_unknown_ipc_handler()
			if handler:
				return handler

		return handler
	
	
	def get_workloop_function(self):
		for interface in self.get_interfaces():
			function = interface.get_workloop_function()
			if function:
				# FIXME: This is a hack for now - ensure we don't pass
				# parameters to function until we work out how to do this
				# cleanly
				assert '(' not in function
				return function

		return None
	
	def get_word_size(self):
		return arch_info.word_size_in_bits

	def _my_interface_class(self):
		return V4Interface

def construct(ast):
	return V4Generator(ast)
