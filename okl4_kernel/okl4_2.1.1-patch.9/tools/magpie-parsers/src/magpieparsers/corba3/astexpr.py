# Turns CORBA parse tree expressions into AST expressions.
# Code adapted from SqlCobble, cobbler/sqlcobbleparser.py::ASTMaker::get_expr()
# Original: http://nyloncactus.com/cgi-bin/darcsweb/darcsweb.cgi?r=sqlcobble;a=headblob;f=/cobbler/sqlcobbleparser.py
# The IDLAST expression format is described here:
# http://lardcave.net/~wzdd/hype/doku.php?id=magpie:doc.idlast

import re
from magpieparsers.parser_common import Node, reversed
from magpieparsers.types import infogripper
from magpieparsers import error

OPER_ARITY = {"=": 2, "+": 2, "*": 2, "/": 2, '-': 2, '&': 2, '<<': 2, '>>': 2,
		'u-': 1, 'u+': 1, 'u~': 1, '|': 2, '^': 2, '%': 2}
# Ordered from lowest to highest.
OPER_PREC = [("*", "/", '%'), ('&','|'), ("+", "-"), ('<<', '>>'), ("=",), ('u+', 'u-', 'u~')]
OPER_NAMES = {'+': 'add', "=": 'equals', '-': 'subtract', '/': 'divide', '*': 'multiply',
	'&': 'bitwise_and', '|': 'bitwise_or', '<<': 'lsl', '>>': 'lsr', 'u-': 'unary_minus',
	'u+': 'unary_plus', 'u~': 'unary_tilde', '^': 'bitwise_xor', '%': 'modulus'}
PYTHON_UNARY = {'u-': '-', 'u+': '+', 'u~': '~'}
 
class Error(Exception):
	pass

def get_precedence(oper):
	for class_idx, prec_class in enumerate(OPER_PREC):
		if oper in prec_class:
			return class_idx
	# NB if the operator it mentions is OPEN_PAREN, then you have a bug
	# elsewhere.
	raise Error("Operator %s doesn't exist in precedence table" % (oper))

def get_arity(oper):
	try:
		return OPER_ARITY[oper]
	except KeyError:
		raise Error("Operator %s doesn't exist in arity table" % (oper))

def smallest_type_hack(value):
	# FIXME: This is a pretty gross hack.
	if isinstance(value, int):
		if value > 0 and value <= 255:
			return 'octet'
		elif value > -32768 and value <= 32767:
			return 'short'
		elif value > 0 and value <= 65535:
			return 'unsigned short'
		else:
			return 'long'
	elif isinstance(value, float):
		return 'float'
	else:
		return None

# Helper function: 
def get_expr_list(pt):
	"""
	Do a depth-first walk of the tree. If we encounter any
	nodes in STOP_AT, yield them without going further.
	"""
	STOP_AT = {}
	for item in ('literal', 'scoped_name', 'unary_operator'):
		STOP_AT[item] = True
	explore = [pt]
	while explore:
		current = explore.pop()
		if current.name in STOP_AT:
			yield current
		elif current.name == 'primary_expr' and current.leaf == '(':
			yield current
			explore.extend(reversed(current.children))
		elif current.children == []:
			yield current
		else:
			explore.extend(reversed(current.children))

def get_expr_type(ast):
	if ast.leaf == 'scoped_name':
		return infogripper.find_alias_target(ast.the('target').children[0])
	else:
		return ast.the('type')

def pop_and_add(scope_ast, oper, operands):
	# Pop and create new tree
	node = Node(None, 'expression')
	node.leaf = OPER_NAMES[oper]
	for count in range(get_arity(oper)):
		node.add_front(operands.pop())
	
	# Work out the type for the resulting expression.
	type_ast = get_expr_type(node.children[0])
	for new_type_expr in node.children[1:]:
		new_type_ast = get_expr_type(new_type_expr)
		if infogripper.can_coerce_expr(new_type_expr, type_ast):
			type_ast = new_type_ast
		else:
			raise error.CannotReconcileTypesError(new_type_ast.leaf, type_ast.leaf, scope_ast)
	
	# Store a value, if we can. This over-rides the above discovered type.
	# FIXME: This is a cheesy hack.
	if type_ast.has_attribute('smallest') and type_ast.has_attribute('largest'):
		# It's a scalar type, we can play with it.
		if len(node.children) > 1:
			result = oper.join(['(%s)' % (child.attribute('value')) for child in node.children])
		else:
			# FIXME: Even cheesier
			result = "%s%s" % (PYTHON_UNARY[oper] , node.children[0].attribute('value'))
		try:
			result = eval(result)
			node.add_attribute('value', result)
		except:
			pass

		smallest_type_name = smallest_type_hack(result)
		if smallest_type_name:
			type_ast = infogripper.getNode(smallest_type_name, scope_ast, 'type')
	
	node.add_child(type_ast)
	operands.append(node)

def is_name(text):
	"""
	Returns True if "text" looks like a scoped name, False otherwise.
	"""
	for scope in text.split('::'):
		if not re.match(r'[A-Za-z_][A-Za-z0-9_]*', scope):
			return False
	return True

def get_expression(scope_ast, master_pt):
	# Dijkstra's shunting yard algorithm.
	# Good conceptual explanation:
	#   http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm
	# Good algorithmic explanation:
	#   http://www.codeproject.com/jscript/JSExpressionEval.asp
	# Here we are building a tree, so we push nodes onto the operand stack.
	operators = []
	operands = []

	for pt in get_expr_list(master_pt):
		#continue
		if pt.name in ('literal',):
			node = make_literal_ast(scope_ast, pt)
			operands.append(node)
		elif pt.name == 'primary_expr' and pt.leaf in ("(",):
			operators.append(pt.leaf)
		elif pt.leaf in (")",):
			while operators[-1] != "(":
				pop_and_add(scope_ast, operators.pop(), operands)
			operators.pop() # Remove open-paren
		elif is_name(pt.leaf):
			if pt.name == 'scoped_name':
				node = make_scoped_name_ast(scope_ast, pt)
			else:
				node = make_identifier_ast(pt)
			operands.append(node)
		else:
			# Assume it's an operator
			if pt.name == 'unary_operator':
				# Special case here
				oper_name = 'u' + pt.leaf
			else:
				oper_name = pt.leaf
			assert oper_name not in (".", "(",) # Just for debugging
			if operators and operators[-1] != "("\
					and get_precedence(operators[-1]) > get_precedence(oper_name):
				pop_and_add(scope_ast, operators.pop(), operands)
				operators.append(oper_name)
			else:
				operators.append(oper_name)
	# Pop all remaining opers off the stack
	while operators:
		pop_and_add(scope_ast, operators.pop(), operands)
	if len(operands) != 1:
		print "Got confused by"
		for pt in get_expr_list(master_pt):
			pt.print_tree()
		for operand in operands:
			print "**** Operand:"
			operand.print_tree()
	assert len(operands) == 1
	return operands[0]

# Map PT type names to AST type names.
TYPENAME_MAP = {'integer_literal': 'signed int', 'wide_string_literal': None, 'boolean_literal': 'bool',
		'floating_pt_literal': 'float', 'string_literal': 'string', 'character_literal': 'char',
		'wide_character_literal': 'wchar', 'wide_string_literal': 'wstring'}

	
def get_python_value(s):
	# FIXME: Cheesy hack.
	if s == 'TRUE':
		return True
	elif s == 'FALSE':
		return False
	else:
		try:
			return eval(s)
		except:
			return s
	
def make_literal_ast(scope_ast, pt):
	assert pt.name == 'literal'
	child_pt = pt.children[0]

	ast = Node(None, "expression")

	ast_type_name = TYPENAME_MAP.get(child_pt.name)
	if ast_type_name is None:
		raise Error("Unknown literal type %s" % child_pt.name)
	
	target_type = infogripper.getNode(ast_type_name, scope_ast, 'type')
	if target_type is None:
		raise Error("Couldn't find type %s in AST" % (ast_type_name))
	
	if target_type.has_attribute('smallest') and target_type.has_attribute('largest'):
		# FIXME: Shouldn't be doing this.
		ast_type_name = smallest_type_hack(get_python_value(child_pt.leaf))
		target_type = infogripper.getNode(ast_type_name, scope_ast, 'type')

	# FIXME: Use of eval is evil
	ast.add_attribute('value', get_python_value(child_pt.leaf))

	# Remember the original name for later.
	ast.add_attribute('display_name', child_pt.name)

	ast.add_child(target_type)

	return ast

def make_scoped_name_ast(scope_ast, pt):
	assert pt.name == 'scoped_name'

	ast = Node(None, "expression", leaf = 'scoped_name')

	# Sanity check
	for child in pt.children:
		assert child.name in ('scope_operator', 'identifier')

	# Build the name.
	scoped_name_list = [child.leaf for child in pt.children]
	value = ''.join(scoped_name_list)

	# Check that it's valid
	target_asts = infogripper.getAllNodes(value, scope_ast, None)
	if len(target_asts) > 1:
		raise error.AmbiguousScopedNameError(value, scope_ast, target_asts)
	elif len(target_asts) == 0:
		raise error.NameNotFoundError(value, pt)

	target_wrapper = Node(scope_ast, 'target', [target_asts[0]])
	ast.add_child(target_wrapper)
	ast.add_attribute('value', value)

	return ast

def make_identifier_ast(pt):
	assert pt.name == 'identifier'

	ast = Node(None, "expression")
	child_ast = Node(ast, "identifier")
	ast.add_child(child_ast)

	child_ast.add_attribute('value', pt.leaf)

	return ast

