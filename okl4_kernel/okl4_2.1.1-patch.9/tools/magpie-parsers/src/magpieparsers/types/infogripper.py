import math
import sys
from magpieparsers.parser_common import *
#from helper import positive_id

from magpieparsers import error as parsers_error

SCOPES = ['module','type','valuetype','struct','union','function','exception','eventtype','component','home', 'MagpieAST']

def getBasicTypenode(typename, ast):
	returntype = getTypenode(typename, ast)
	if returntype is None:
		raise parsers_error.UnknownTypeError(failed_name = typename)
	return returntype
	
def getTypenode(typename, ast):
	return getNode(typename, ast, 'type')

def get_base_typename(typename):
	if isinstance(typename, basestring):
		typename = typename.split('::')
	return typename[-1]

def get_scope_for_typename(ast, typename):
	if isinstance(typename, basestring):
		typename = typename.split('::')

	if len(typename) > 1:
		working_node = find_scope(ast, typename[:-1])
		assert working_node is not None
	else:
		working_node = ast

	return working_node

def _find_in_ast(ast, node_name, node_leaf):
	found = []

	for child in ast.children:
		if child.leaf == node_leaf and (node_name is None or child.name == node_name):
			found.append(child)
	
	return found

# Node resolution:
# Try me
# Try all my parents, breadth-first
# Try my outer scope.
# Until bored or found or no more outer scopes

def _getnode_in_the_family(ast_list, node_name, node_leaf, get_all = False):
	"""
	Do a breadth-first search of the tree starting from roots at "ast_list".
	"""
	candidates = ast_list

	found = []

	while candidates:
		candidate = candidates.pop(0)
		found.extend(_find_in_ast(candidate, node_name, node_leaf))
		if found and not get_all:
			return found
		
		candidates.extend( [item.children[0] for item in candidate['inherits']] )
	
	return found

def getNode(typename, ast, nodetype = None):
	# Typename could be a basic type, or a scoped type. If it's a scoped type,
	# find the appropriate spot to begin searching.
	ast = get_scope_for_typename(ast, typename)

	# Remove any scope information from "typename".
	typename_base = get_base_typename(typename)

	# Walk up from AST until we get to a scope.
	while ast.name not in SCOPES:
		ast = ast.parent

	# Starting from AST, search all children for "nodetype" of type "typename_base".
	# If we don't find any, check the parent, until we can't go any higher, which
	# means we've reached the root (or our tree is broken)

	# This is complicated by inheritance rules. If in our travels we encounter a type
	# with an "inherits" attribute, also try "find_in_ast" in that type.
	while ast:
		# Regular search
		targets = _getnode_in_the_family([ast], nodetype, typename_base)
		if targets:
			return targets[0]

		# No luck, so try the enclosing scope (confusingly named "parent")
		ast = ast.parent
	
	return None

def getAllNodes(typename, ast, nodetype):
	# FIXME: This is rather similar to getNode, but the bulk of the main loop
	# is different.

	# Typename could be a basic type, or a scoped type. If it's a scoped type,
	# find the appropriate spot to begin searching.
	ast = get_scope_for_typename(ast, typename)
	# Remove any scope information from "typename".
	typename_base = get_base_typename(typename)

	# Walk up from AST until we get to a scope.
	while ast.name not in SCOPES:
		ast = ast.parent

	# Starting from AST, search all children for "nodetype" of type "typename_base".
	# If we don't find any, check the parent, until we can't go any higher, which
	# means we've reached the root (or our tree is broken)

	# This is complicated by inheritance rules. If in our travels we encounter a type
	# with an "inherits" attribute, also try "find_in_ast" in that type.
	found = []
	while ast:
		# Regular search
		found.extend(_getnode_in_the_family([ast], nodetype, typename_base, get_all = True))

		ast = ast.parent
	
	return found

def node_sorter(lhs, rhs):
	return cmp( (lhs.source_file, lhs.source_line), (rhs.source_file, rhs.source_line) )

def getSingleNode(typename, ast, nodetype):
	found = getAllNodes(typename, ast, nodetype)

	assert len(found) == 1 or len(found) == 2

	if len(found) == 2:
		found.sort(node_sorter)
		raise parsers_error.SymbolDefinedTwiceError(typename, found[0], found[1])
	
	return found[0]

def can_coerce_expr(expr_from, type_to):
	type_from = expr_from.the('type')
	# If a type is None, it may mean that it came from an expression node that
	# did not contain a child Type node.
	assert type_from is not None and type_to is not None

	#print "expr from"
	#expr_from.print_tree()
	#print "type to"
	#type_to.print_tree()

	type_from = find_alias_target(type_from)
	type_to = find_alias_target(type_to)

	if type_from is type_to:
		return True
	
	# If we can't do any further comparisons, and the types aren't strictly the same,
	# bail.
	if not type_from.has_attribute('smallest') or not type_to.has_attribute('smallest') \
			or not type_from.has_attribute('largest') or not type_to.has_attribute('largest'):
		return False
	
	# If "type_from" fits within "type_to" then we're ok.
	if type_from.attribute('smallest') >= type_to.attribute('smallest') \
			and type_from.attribute('largest') <= type_to.attribute('largest'):
		return True
	
	# If "type_from" happens to be a constant with a known value, and *that* 
	# fits with in "type_to", then we're OK.
	if expr_from.attribute('value', None) is not None \
			and expr_from.attribute('value') >= type_to.attribute('smallest') \
			and expr_from.attribute('value') <= type_to.attribute('largest'):
		return True
	
	# The lesson is: never try.
	return False

def dump_types_n_size(node, prefix = None):
	returnlist = []
	if node.type in SCOPES:
		prefix = prefix + node.leaf + '_'
	if node.type == 'type' and node.leaf != None:
		type_size = size(node)
		name = prefix + node.leaf
		returnlist.append([name, type_size, node])
	for child in node.children:
		child_list = dump_types_n_size(child, prefix)
		if child_list != []:
			returnlist.extend(child_list)
	return returnlist

	
def find_type_in_scope(scope_node, typename):
	return find_node_in_scope(scope_node, typename, 'type')
	
def find_node_in_scope(scope_node, typename, nodetype):
	# look for type typename without descending into a node of SCOPES (i.e. a new scope)
	type_node = scope_node.find_node(nodetype,typename, SCOPES)
	if type_node != None:
		return type_node
	# else: look for typename in inherited scopes
	if scope_node.has_attribute('inheritance'):
		inher_list = scope_node.get_attribute('inheritance')
		for inher_scope_name in inher_list:
			inher_scope = find_scope(scope_node, inher_scope_name.split('::'))
			assert inher_scope is not None
			inher_result = find_node_in_scope(inher_scope, typename, nodetype)
			if inher_result != None:
				return inher_result
	return None
	
			
			
	
def find_scope(startnode, name):
	if not isinstance(name, list):
		name = name.split('::')
	
	#first scope to be searched
	find = name[0]
	if find == '':
		# root_scope ...
		working_node = get_root_scope(startnode)
	else:
		# -> just bottom-up till a "scope"-node
		working_node = startnode
		while working_node != None and ((working_node.type not in SCOPES) or (working_node.leaf != find)):
			result_node = working_node.find_node(SCOPES, name, SCOPES)
			if result_node is not None:
				#print 'returning1: %s' %(result_node)
				return result_node
			#print '2working_node is now: %s(%s)' %(working_node.type, working_node)
			working_node = working_node.parent
		
	for scope in name[1:]:
		working_node = working_node.find_node(SCOPES, scope, SCOPES)

	assert working_node is not None
	return working_node
		
def get_root_scope(startnode):
	root = startnode
	while root.parent != None:
		root = root.parent
	return root
	
def get_full_scope(node):
	if node is None:
		return []
	node = node.parent
	scope_list = []
	while node.parent is not None:
		if node.name in SCOPES:
			scope_list.append(node.leaf)
		node = node.parent
		
	scope_list.reverse()
	return scope_list
		
def get_full_scope_string(node):
	if node is None:
		return ''
	scope_list = []

	node = node.parent

	while node.parent is not None:
		if node.name in SCOPES:
			scope_list.append(node.leaf)
		node = node.parent
		
	scope_list.reverse()
	#print scope_list
	return scope_list
	
	
def get_param_type_spec(node, side='client'):
	#possible meta_types: basic, alias, string, wstring, scoped_name, pointer
	meta_type = node.attribute('meta_type')
	if meta_type in ['basic','alias']:
		scopelist = get_full_scope(node)
		scopelist.append(node.leaf)
		return '_'.join(scopelist)
	elif meta_type in ['string','wstring']:
		return 
	elif meta_type == 'pointer':
		return '*' + get_param_type_spec(node.get_attribute('target_type')[0])
	elif meta_type == 'polymorphic':
		if side == 'client':
			return get_param_type_spec(node.attribute('receiver_type'))
		else:
			return get_param_type_spec(node.attribute('sender_type'))
	elif meta_type == 'scoped_name':
	
		target = node.the('target').the('type')
		return '_'.join(get_full_scope(target).append(target.leaf))
	else:
		print 'meta_type unknown: ', meta_type
		assert False
	
	
	
def basic_size(type_node):
	if isinstance(type_node.attribute('size'), str):
		# FIXME: Can this go? Do we ever get str sizes? - nfd
		try:
			return int(eval(type_node.attribute('size')))
		except Exception:
			print 'Exception in basic_size-calculation of ', type_node.attribute('size')
			return -1
	else:
		return type_node.attribute('size')
		
def sized_alias_size(type_node):
	#type_node.print_tree()
	return basic_size(type_node)

def alias_size(type_node):
	
	#print 'alias_size of:'
	#type_node.print_tree()
	target_node = type_node.maybe_walk_to('target', 'type')
	if target_node == None:
		type_node.print_tree()
		assert False
	return size(target_node)

def union_size(type_node):
	case_list = [case.the('type_instance') for case in type_node.the('members')['case']]
	maxsize = 0
	for case in case_list:
		target_type = case.get_attribute('target_type')[0]
		target_size = size(target_type)
		if target_size > maxsize:
			maxsize = target_size
	return maxsize
	
def enum_size(type_node):
	# FIXME: Is this right? Enums in C should just be sizeof(int)...
	int_type = getBasicTypenode('signed int', type_node)
	return basic_size(int_type)

	# Old code:
	#length = len(type_node.get_attribute('enumeration'))
	#enumsize = math.ceil(math.log(math.ceil(math.log(length, 2)),32)) * 32
	#return enumsize
	
def struct_size(type_node):
	# Create a list of target types.
	type_list = []
	for type_inst in type_node.the('members')['type_instance']:
		target_ast = type_inst.the('target').the('type')
		type_list.append(target_ast)

	structsize = 0
	for inst_type in type_list:
		structsize += size(inst_type)
	return structsize
	
def array_size(type_node):
	shape = type_node.get_attribute('shape')
	base_size = size(type_node.the('target').the('type'))
	if shape is None:
		return 0
	else:
		for dim in shape:
			if dim.isdigit():
				base_size *= eval(dim)
			else:
				base_size *= eval(constant_lookup(dim))
		return base_size
	
def sequence_size(type_node):
	shape = type_node.get_attribute('shape')
	base_size = size(type_node.the('target').the('type'))
	if shape is None:
		return 0
	else:
		for dim in shape:
			if dim.isdigit():
				base_size *= eval(dim)
			else:
				base_size *= eval(constant_lookup(dim))
		return base_size
	
def polymorphic_size(type_node):
	sender_size = size(type_node.attribute('sender_type'))
	receiver_size = size(type_node.attribute('receiver_type'))
	return (sender_size, receiver_size)
	
def pointer_size(type_node):
	int_node = getTypenode('int', type_node)
	if int_node != None:
		return size(int_node)
	else:
		print 'int-Node in infogripper.py -> pointer_size not found!'
		return 32

def string_size(type_node):
	return pointer_size(type_node)

def word_size(type_node):
	int_node = getTypenode('signed int', type_node)
	if int_node != None:
		return size(int_node)
	else:
		print 'int-Node in infogripper.py -> word_size not found!'
		return 32
		
def size(type_node):
	if type_node.type=='target':
		type_node = type_node.the('type')
	if not type_node.has_attribute('meta_type'):
		type_node.print_tree()
		assert False
	meta_type = type_node.get_attribute('meta_type')[0] + '_size'
	#print globals()[meta_type]
	#print 'meta_type = ', meta_type
	return globals()[meta_type](type_node)

def find_alias_target(ast):
	""" If "ast" is an alias type, return its target.
	If "ast" is a type instance, go to the constructing type first.
	"""
	try_again = True
	while try_again:
		try_again = False

		if ast.name == 'type_instance':
			ast = ast.the('target').the('type')
			try_again = True

		while ast.name == 'type' and ast.the('customised'):
			ast = ast.the('customised').children[0]
			try_again = True

		while ast.name == 'type' and ast.get_attribute('meta_type') == ['alias']:
			ast = ast.the("target").children[0]
			try_again = True
	
	return ast
	
# Helpers for type info
def is_basic_type(ast):
	return ast.name == 'type' and ast.get_attribute('meta_type') == ['basic']

def is_corba_integer(ast):
	ast = find_alias_target(ast)
	return is_basic_type(ast) and ast.leaf in ('short', 'unsigned short',
			'long', 'unsigned long', 'long long', 'unsigned long long')

def is_bool(ast):
	ast = find_alias_target(ast)
	return is_basic_type(ast) and ast.leaf == 'bool'

def is_char(ast):
	ast = find_alias_target(ast)
	return is_basic_type(ast) and ast.leaf == 'char'

def is_octet(ast):
	ast = find_alias_target(ast)
	return is_basic_type(ast) and ast.leaf == 'octet'

def is_enum(ast):
	ast = find_alias_target(ast)
	return ast.get_attribute('meta_type') == ['enum']

def is_void(ast):
	ast = find_alias_target(ast)
	return is_basic_type(ast) and ast.leaf == 'void'
