"""
Create IDLAST-compatible expressions.

Less confusing than the corba3/astexpr.py because the C++ parser returns expressions that are already basically sane and wee just simplify them.
"""

from magpieparsers.parser_common import Node, reversed
from magpieparsers.types.infogripper import getNode, getTypenode

class Error(Exception):
	pass

def get_scoped_name(scope_ast, pt, ast_gen):
	the_name = pt.get_single_attribute('value')
	
	scoped_name = Node(scope_ast, 'expression', leaf = 'scoped_name', source = pt)
	scoped_name.add_attribute('value', the_name)

	# Try to find a target.
	type_ast = getNode(the_name, scope_ast)
	# It's not the end of the world if we don't find a target.
	if type_ast:
		target_ast = Node(scoped_name, 'target', [type_ast])
		scoped_name.add_child(target_ast)

	return scoped_name

def _get_call_params(scope_ast, pt, ast_gen):
	params_ast = Node(scope_ast, 'expression', leaf = 'parameters', source = pt)

	if pt.leaf == 'comma':
		for param_pt in pt.get_children_named('expression'):
			param_expr_ast = get_expression(params_ast, param_pt, ast_gen)
			if param_expr_ast is None:
				param_pt.print_tree()
			params_ast.add_child(param_expr_ast)
	elif pt.name == 'expression':
		params_ast.add_child(get_expression(params_ast, pt, ast_gen))
	else:
		# hm
		pt.print_tree()
		raise Error("Unable to decode parameter list")

	return params_ast

def get_call(scope_ast, pt, ast_gen):
	"""
	Translate function calls. PT looks like this:
	expression call
	 expression identifier
	  (value) = <function name>
	 expression comma (optional)
	  expression param1
	  expression param2
	  ...
	
	We turn them into this AST:
	expression call
	 expression scoped_name
	  (value) = <function name>
	 expression parameters (optional)
	  expression param1
	  expression param2
	  ...
	"""
	# Translate function calls.
	node = Node(scope_ast, 'expression', leaf = 'call', source = pt)

	expression_identifier = pt.children[0]
	assert expression_identifier.leaf == 'id_expression'

	node.add_child(get_scoped_name(node, expression_identifier, ast_gen))

	# Add parameters.
	if len(pt.children) == 2:
		node.add_child(_get_call_params(node, pt.children[1], ast_gen))

	return node

def get_literal(scope_ast, pt, ast_gen):
	"""
	Translate a constant that looks like this:
	expression literal
	 int 0   (or "float 3.1415", etc)

	to:
	expression literal
	 (value) = <constant>
	 type (backref)
	"""
	# Extract type
	type_list = [pt.child().name]
	type_ast = ast_gen.get_type_from_list(scope_ast, type_list)

	value = pt.child().leaf

	node = Node(scope_ast, 'expression', leaf = 'literal', source = pt)
	node.add_attribute('value', value)
	node.add_child(type_ast)

	return node

def get_conditional_expression(scope_ast, pt, ast_gen):
	"""
	Conditional expressions (ternary expression)
	"""
	node = Node(scope_ast, "expression", leaf = "ternary_if", source = pt)
	cond_pt, true_pt, false_pt = pt.children

	node.add_child(get_expression(node, cond_pt, ast_gen))
	node.add_child(get_expression(node, true_pt, ast_gen))
	node.add_child(get_expression(node, false_pt, ast_gen))

	return node

def get_cast(scope_ast, pt, ast_gen):
	"""
	Casts
	"""
	node = Node(scope_ast, 'expression', leaf = 'cast', source = pt)

	type_ast = ast_gen.get_type(scope_ast, pt)
	assert type_ast is not None
	# Indirection stays the same
	indirection_ast = Node(scope_ast, 'indirection', leaf = pt.leaf)
	# Expression is converted.
	expr_ast = get_expression(node, pt.get_child('expression'), ast_gen)

	# And we're done
	node.add_children([type_ast, indirection_ast, expr_ast])
	return node

OK_ALREADY = ['add', 'subtract', 'multiply', 'divide', 'equals', 'notequals', 'bitwiseand',
		'shiftright', 'shiftleft', 'lessthan', 'greaterthan',
		'timesqueals', 'dividesequals', 'minusequals',
		'plusequals', 'modequals', 'shiftleftequals', 'shiftrightequals',
		'bitwiseandequals', 'bitwiseorequals', 'bitwisexorequals',
		'bitwiseor', 'bitwisexor', 'comma',
		'postfixdot', 'postfixptr', 'unary_not', 'unary_pointsto', 'arrayindex',
		'unary_ampersand', 'plusplus', 'minusminus', 'logicaland', 'logicalor']
HANDLERS = {'call': get_call, 'literal': get_literal, 'id_expression': get_scoped_name,
		'c_ternary': get_conditional_expression,
		'cast': get_cast}

def get_expression(scope_ast, pt, ast_gen):
	if pt.name == 'expression' and pt.leaf == '' and len(pt.children) == 1:
		pt = pt.children[0]

	if pt.name == 'expression' and pt.leaf in OK_ALREADY:
		node = Node(scope_ast, 'expression', leaf = pt.leaf, source = pt)
		for child_pt in pt.children:
			node.add_child(get_expression(node, child_pt, ast_gen))
		return node
	elif pt.name == 'expression' and pt.leaf in HANDLERS:
		return HANDLERS[pt.leaf](scope_ast, pt, ast_gen)
	else:
		pt.print_tree()
		raise Error("Couldn't translate PT")

