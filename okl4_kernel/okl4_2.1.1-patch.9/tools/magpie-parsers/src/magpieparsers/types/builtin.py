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

from magpieparsers.parser_common import Node, TypeNode
from magpieparsers.types.infogripper import getTypenode
from arch_sizes import construct_for_arch

CORBA_NAMES = ("string", 'smallstring', "wstring", "wchar")

C_NAMES = ("void", "signed char", "signed short int", "signed int", "signed long int",
	"signed long long int", "signed __int64",
	"unsigned char", "unsigned short int", "unsigned int", "unsigned long int",
	"unsigned long long int", "unsigned __int64",
	"float", "double", "long double",
	"wchar_t", "bool", "octet", "fpage", "__builtin_va_list")

C_ALIAS = (('boolean', 'bool'),
	)

MIG_NAMES = (
		"MACH_MSG_TYPE_INTEGER_64",
		"MACH_MSG_TYPE_REAL_64",
		"polymorphic",
		"MACH_MSG_TYPE_BOOLEAN",
		"MACH_MSG_TYPE_INTEGER_32",
		"MACH_MSG_TYPE_REAL_32",
		"MACH_MSG_TYPE_PORT_NAME",
		"MACH_MSG_TYPE_PORT_internal",
		"MACH_MSG_TYPE_SEND_internal",
		"MACH_MSG_TYPE_RECEIVE_internal",
		"MACH_MSG_TYPE_INTEGER_16",
		"MACH_MSG_TYPE_INTEGER_8",
		"MACH_MSG_TYPE_CHAR",
		"MACH_MSG_TYPE_BYTE",
		"MACH_MSG_TYPE_BIT",
		"MACH_MSG_TYPE_POLYMORPHIC",
		'MACH_MSG_TYPE_UNSTRUCTURED',
		"MACH_MSG_TYPE_REAL",
		"MACH_MSG_TYPE_STRING",
		"MACH_MSG_TYPE_STRING_C" 
)
		
MIG_PROMOTIONS = {}

MIG_ALIAS = ()

def _make_struct(ast, name, *args):
	struct = TypeNode(ast, leaf = name, source_file = '<builtin>')
	struct.add_attribute('meta_type', 'struct')

	members = Node(struct, 'members')
	struct.add_child(members)

	for arg_type, arg_name in args:
		inst_node = Node(None, 'type_instance', leaf = arg_name)

		target = Node(inst_node, 'target')
		type_node = getTypenode(arg_type, ast)
		target.add_child(type_node)

		inst_node.add_child(target)
		members.add_child(inst_node)
	
	return struct

def create_special_C(arch_info, ast):
	type_list = ast.children
	struct_t = TypeNode(ast, source_file = '<builtin>')
	struct_t.leaf = 'idl4_server_environment'
	struct_t.add_attribute('meta_type','struct')
	members = Node(struct_t, 'members')
	typeinst = Node(struct_t, 'type_instance')
	typeinst.leaf = '_action'
	index = [element.leaf for element in type_list].index('signed int')
	typeinst.add_attribute('target_type',type_list[index] )
	members.add_child(typeinst)
	typeinst = Node(struct_t, 'type_instance')
	typeinst.leaf = '_data'
	index = [element.leaf for element in type_list].index('void')
	typeinst.add_attribute('target_type',type_list[index] )
	members.add_child(typeinst)

	# Create IDL4 scope for giggles.
	idl4 = Node(ast, 'type', leaf = 'idl4', source_file = '<builtin>')
	idl4.add_attribute('meta_type', 'private')
	pagefault = Node(idl4, 'type', leaf = 'pagefault')
	idl4.add_child(pagefault)
	ast.add_child(idl4)
	
	#FIXME: CORBA-C Type-Hack!!!
	aliases = ( ('Object', 'signed int'),
			('any', 'signed int'),
			('ValueBase', 'signed int'),
			('Word', 'signed int')
	)

	# Create aliases to C nodes.
	for alias, name in aliases:
		newType = TypeNode(ast, source_file = '<builtin>')
		newType.leaf = alias
		newType.add_attribute('meta_type', 'alias') 
		type_list = ast.children
		index = [element.leaf for element in type_list].index(name)
		target_node = Node(newType, 'target')
		target_node.add_child(type_list[index])
		newType.add_child(target_node)

		ast.add_child(newType)
	
	# Explicitly add the IDL4 mapitem struct, yuck yuck
	mapitem = _make_struct(ast, 'idl4_mapitem', ('unsigned long int', 'base'), ('unsigned long int', 'fpage'))
	ast.add_child(mapitem)

def create_special_MIG(arch_info, ast):
	poly_list =(
	("MACH_MSG_TYPE_PORT_RECEIVE",	'32', 'MACH_MSG_TYPE_PORT_internal',	'MACH_MSG_TYPE_POLYMORPHIC'),
	("MACH_MSG_TYPE_PORT_SEND",		'32', 'MACH_MSG_TYPE_PORT_internal',	'MACH_MSG_TYPE_POLYMORPHIC'),
	("MACH_MSG_TYPE_PORT_SEND_ONCE", '32', 'MACH_MSG_TYPE_SEND_internal',	'MACH_MSG_TYPE_POLYMORPHIC'),
	("MACH_MSG_TYPE_COPY_SEND",		'32', 'MACH_MSG_TYPE_SEND_internal',	'MACH_MSG_TYPE_PORT_SEND'),
	("MACH_MSG_TYPE_MAKE_SEND",		'32', 'MACH_MSG_TYPE_SEND_internal',	'MACH_MSG_TYPE_PORT_SEND'),
	("MACH_MSG_TYPE_MOVE_SEND",		'32', 'MACH_MSG_TYPE_SEND_internal',	'MACH_MSG_TYPE_PORT_SEND'),
	("MACH_MSG_TYPE_MAKE_SEND_ONCE", '32', 'MACH_MSG_TYPE_SEND_internal',	'MACH_MSG_TYPE_PORT_SEND_ONCE'),
	("MACH_MSG_TYPE_MOVE_SEND_ONCE", '32', 'MACH_MSG_TYPE_SEND_internal',	'MACH_MSG_TYPE_PORT_SEND_ONCE'),
	("MACH_MSG_TYPE_MOVE_RECEIVE",	'32', 'MACH_MSG_TYPE_RECEIVE_internal', 'MACH_MSG_TYPE_PORT_RECEIVE')
	)

	type_list = ast.children
	for name, size, sender, receiver in poly_list:
		newType = TypeNode(ast, None, source_file = '<builtin>')
		newType.leaf = name
		newType.add_attribute('meta_type', 'polymorphic')
		info = Node(newType, 'info')
		newType.add_child(info)
		index = [element.leaf for element in type_list].index(sender)
		info.add_attribute('sender_type',type_list[index] )
		index = [element.leaf for element in type_list].index(receiver)
		info.add_attribute('receiver_type',type_list[index] )
		type_list.append(newType)
		ast.add_child(newType)

def create_basictype_ast(arch_name, typetype):
	ast = Node(None, 'MagpieAST', None)
	arch_info = construct_for_arch(arch_name, typetype)
	if typetype == 'idl4':
		names = CORBA_NAMES + C_NAMES
		aliases = C_ALIAS
		
	if typetype == 'mig':
		names = CORBA_NAMES + MIG_NAMES
		aliases = MIG_ALIAS
		
	for name in names:
		newType = TypeNode(ast, None, source_file = '<builtin>')
		newType.leaf = name
		newType.add_attribute('meta_type', 'basic')
		newType.add_attribute('size', arch_info.size_in_bits(name))
		
		if arch_info.smallest(name) is not None:
			newType.add_attribute('smallest', arch_info.smallest(name))

		if arch_info.largest(name) is not None:
			newType.add_attribute('largest', arch_info.largest(name))

		ast.add_child(newType)
		
	for alias, name in aliases:
		newType = TypeNode(ast, None, source_file = '<builtin>')
		newType.leaf = alias
		newType.add_attribute('meta_type', 'alias')
		type_list = ast.children
		index = [element.leaf for element in type_list].index(name)
		target = Node(newType, 'target', [type_list[index]])
		newType.add_child(target)
		ast.add_child(newType)
		
	if typetype == 'idl4':
		create_special_C(arch_info, ast)
	if typetype == 'mig':
		create_special_MIG(arch_info, ast)
	
	return ast
