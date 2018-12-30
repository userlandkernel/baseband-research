"""
Transform the parse tree produced by parser.py to a higher-level tree using
recursive descent.
"""

from magpieparsers.parser_common import *
from magpieparsers.types.evaluator import evaluate
from magpieparsers.types.infogripper import *
from magpieparsers.cplusplus.normalise import normalise_type_list
from astexpr import get_expression
import operator

class Error(Exception):
	pass

CLASS_SPECIFIERS = ('union', 'struct', 'class', 'enum_specifier')

# AST helper functions
def any_child(node, name_list):
	"""
	Return the first child of "node" with a name matching any
	in "name_list"
	"""
	for child in node.children:
		if child.name in name_list:
			return child

def any_in_list(list1, list2):
	""" Return the item if any items in list1 are present in list2"""
	for item in list1:
		if item in list2:
			return item


def all_children(node, names):
	for child_name in node.get_child_names():
		if child_name not in names:
			return False
	return True
		
# AST generator
class ASTGen(object):
	def __init__(self, types_only = True, experimental = False):
		"""
		If types_only is set to True, only typedefs are converted.
		Conversion of entities other than typedefs is experimental
		and is likely to fail with an exception.

		If experimental is set to True, a slightly different (new-format)
		data structure will be produced.
		"""
		self.types_only = types_only
		self.experimental = experimental

	def convert(self, ast, pt, filename, types_only = True):
		""" Convert 'pt' to 'ast'.
		"""
		return self.translation_unit(ast, pt, filename)

	def translation_unit(self, baseast, pt, filename):
		#print 'PT'
		#pt.print_tree()
		#print 'PT done'
		#ast = Node(baseast, "translation_unit")
		#ast.leaf = filename
		ast = baseast # Hmm - nfd
		#ref_node = ast
		for node in pt.children:
			if node.name == 'declaration':
				#print '*******************************\n\
				#	processing tree: '
				#node.print_tree()
				
				#decl_types = self.declaration(ref_node, node)
				decl_types = self.declaration(ast, node)
				ast.add_children(decl_types)
				
				#ref_node = decl_types[-1]
				#print '*******************************\n\
				#	new starting-tree: '
				#ref_node.print_tree()
				
				
				#print '*******************************\n\
				#	processing final-tree: '
				#ast.print_tree()
					
			elif node.name == 'function_definition':
				if not self.types_only:
					self.function_definition(ast, node)
				#ast.add_child(Node(ast, 'unfinished_function_definition', source = node))
			elif node.name == 'enum':
				if not self.types_only:
					self.enum(ast, node)
			else:
				ast.add_child(UnknownNode(ast, node, 'translation_unit', source = node))
		return ast
	
	def _wrap_node(self, name, parentast, ast):
		result = Node(parentast, name, source = parentast)
		result.add_child(ast)
		return result

	def enum(self, parent, pt):
		"""
		We look for:
		enum
		 enum_specifier
		  enumerator_list
		   enumerator
		    ...
		   enumerator
		    ...
		 init_declarator_list (optional)
		"""
		# FIXME: Stub
		ast = Node(parent, 'unfinished')
		return ast

	def function_definition(self, parent, pt):
		"""
		Yay a function! We expect it to look like this:
		function_definition
		 declaration_specifiers - 'inline' etc plus return type
		 function_declarator - function name and parameters
		 declaration_list - function parameters (K&R only)
		 compound_statement - function body
		"""

		def walk_to_fdd(fd_pt):
			""" Walks a chain of function declarators, collecting
			indirection, until we get to function_direct_declarator.
			"""
			indirection = ''
			while fd_pt:
				if fd_pt.leaf == '*':
					indirection += '*'
				fdd_pt = fd_pt.get_child('function_direct_declarator')
				fd_pt = fd_pt.get_child('function_declarator')
			
			return indirection, fdd_pt
		# Stick the node in the AST.
		node = Node(parent, 'function', source = pt)
		parent.add_child(node)

		# Parts of the parse tree
		# ... function_direct_declarator holds param list & name
		retval_indirection, function_direct_declarator = \
				walk_to_fdd(pt.get_child('function_declarator'))
		declaration_specifiers = pt.get_child('declaration_specifiers')
		compound_statement = pt.get_child('compound_statement')

		# Get function name...
		node.leaf = function_direct_declarator.leaf

		# Function return type...
		return_type_ast = Node(node, 'return_type', source = declaration_specifiers)
		return_type_ast.add_attribute('indirection', retval_indirection)
		return_type_ast.add_child(self._wrap_node('target', node,
				self.get_type(node, declaration_specifiers)))
		node.add_child(return_type_ast)

		# Function parameters...
		parameter_list = function_direct_declarator.get_child('parameter_list')
		if parameter_list:
			parameter_declaration_list = parameter_list.get_child('parameter_declaration_list')
			for parameter_declaration in parameter_declaration_list.get_children_named('parameter_declaration'):
				parameter_ast = self._function_parameter(node, parameter_declaration)
				node.add_child(parameter_ast)

		# We ignore K&R-style declaration list

		# Function body.
		body_ast = Node(node, 'body', source = compound_statement)
		node.add_child(body_ast)
		body_ast.add_children(self._compound_statement(body_ast, compound_statement))

	def get_type(self, parentast, pt):
		"""
		Given a PT containing a type reference as a child, return the AST node
		describing this type. The AST node may already exist, or it may be
		created here.
		"""
		if pt.has_child('simple_type_specifier'):
			#type_name = ' '.join(pt.get_child('simple_type_specifier').leaf)
			#return TypeNode(parentast, None, type_name)
			simple_type_specifier = pt.get_children_named('simple_type_specifier')[0]
			type_list = [child_pt.name for child_pt in simple_type_specifier.children]
			typenode = self.get_type_from_list(parentast, type_list)
		elif any_child(pt, CLASS_SPECIFIERS):
			child_pt = any_child(pt, CLASS_SPECIFIERS)
			# We're either declaring an instance of an existing class, or 
			# creating a new one.
			typenode = self.find_or_create_type(parentast, child_pt)
		elif pt.my_children_are('enum_specifier'):
			# FIXME: We still have bad handling of enums.
			typenode = getTypenode('int', parentast)
		else:
			pt.print_tree()
			parentast.print_tree()
			raise Exception()
		if typenode is None:
			print "Couldn't find type node in here:"
			pt.print_tree()
			print "My parent is:"
			parentast.print_tree()
			assert typenode is not None
		return typenode
	
	def get_type_from_list(self, parentast, type_list):
		type_list = normalise_type_list(type_list)
		type_name = ' '.join(type_list)
		return getTypenode(type_name, parentast)

	def find_or_create_type(self, parentast, pt):
		# Called when we're not sure if the current type must be created
		# or not.
		# FIXME: We could/should extend this to other types.
		assert pt.type in CLASS_SPECIFIERS
		
		# The class may exist
		ast = None
		if pt.leaf:
			ast = getTypenode(pt.leaf, parentast)
		# There is one case where getTypenode can succeed and yet the type
		# can be defined below: when a previous forward declaration is
		# being defined.
		if ast and not ast.get_child('members') and pt.get_child('members'):
			# We're defining it here - so forget about the previous reference
			# and use this one.
			ast = None
		
		# By this point the name isn't declared, or we need to define a 
		# forward reference.
		if not ast:
			# The class doesn't exist - create it
			ast = self.create_type(parentast, pt)
		
		# Done: if we're None at this point something is broken.
		assert ast is not None
		return ast

	def create_type(self, parentast, pt):
		"""
		Create a new type from a list of member_declarations
		"""
		# We know how to construct the following types:
		# structs
		# unions
		# enums
		# forward declarations of structs
		if pt.name == 'struct':
			handler = self.create_type_struct
		elif pt.name == 'union':
			handler = self.create_type_union
		elif pt.name == 'enum_specifier':
			handler = self.create_type_enum
		else:
			raise Error("Unknown parsed type '%s'" % (pt.type))
		return handler(parentast, pt)
	
	def create_type_struct(self, parentast, pt):
		members_pt = pt.get_child('members')

		if pt.leaf and members_pt and getTypenode(pt.leaf, parentast) is not None:
			# This is a forward declaration!
			# Modify the forward declaration in-place.
			ast = getTypenode(pt.leaf, parentast)
		else:
			ast = TypeNode(parentast, None, leaf = pt.leaf, source = members_pt)
			ast.add_attribute('meta_type', 'struct')
			
		# We may not have a "members" tree if this was a forward decl
		if members_pt:
			ast.add_child(Node(ast, 'members', source = members_pt))
			decls_list = self.make_declarations_list(ast, members_pt)
			ast.get_child('members').add_children(decls_list)
		return ast

	def create_type_union(self, parentast, pt):
		members_pt = pt.get_child('members')
		ast = TypeNode(parentast, None, source = members_pt)
		ast.add_attribute('meta_type', 'union')
		if members_pt:
			decls_list = self.make_declarations_list(ast, members_pt)
			ast.add_child(Node(ast, 'members', children = decls_list, leaf = pt.leaf, source = members_pt))
		ast.add_attribute('switch_type', None)
		return ast

	def create_type_enum(self, parentast, pt):
		"""
		FIXME: The code below is truly broken. In C++ enums can look like this:
		enum Blah{...} blah;
		enum {...} blah;
		enum {...};

		Where the {...} is a list of IDs with (optional) assignment to integers.

		A helpful site:
		http://cpptips.hyperformix.com/Enums.html
		"""
		members_pt = pt.get_child('enumerator_list')
		ast = TypeNode(parentast, None, source = members_pt)
		ast.add_attribute('meta_type', 'enum')
		decls_list = self.make_declarations_list(ast, members_pt)
		#print "There is only one Power in this world that knows all about the Rings and their effects.\n  -> Use this Power to solve the BUG!"
		ast.add_children(decls_list)
		return ast

	def make_declarations_list(self, ast, members_pt):
		decls = []
		for child in members_pt.children:
			if child.name in ('declaration', 'member_declaration'):
				decls.extend(self.declaration(ast, child))

		return decls

	def _is_declaration(self, pt):
		return pt.name in ('declaration', 'using_declaration', 'linkage_specification')
		
	def declaration(self, parentast, pt):
		"""
		Process a declaration.

		Complicated because the declaration could be:
		  * A simple variable declaration
		  * A definition
		  * A forward declaration

		It also supports member declarators (inside enums, typed class members, typedefs)
		in addition to init declarators (all other types of declaration)
		"""
		# The first part of the declaration specifies the type.
		# (the "int" part of "int x")
		decl_specifier = pt.get_child('declaration_specifiers')
		# This function is used for both member declarations (lists of declarators
		# inside structs) and init declarators (values on the RHS of a declaration)
		# so we test for both. This list contains intialisers and names.
		# (the "x" part of "int x")
		init_decl_list = pt.get_child('init_declarator_list')
		if not init_decl_list:
			init_decl_list = pt.get_child('member_declarator_list')

		# Bail early if this doesn't look at all right.
		if not decl_specifier:
			return [UnknownNode(None, pt, name = 'declaration_type')]
	
		if init_decl_list:
			# The type referenced may already exist, or it may be declared here.
			declarators = self._init_declarator_list(parentast, init_decl_list)
		else:
			# If there are no init declarators, that means that either:
			# 1. This is a new type, or
			# 2. This is a forward declaration.
			declarators = []


		# Now we're ready to create declarations. We create as many declarations
		# as there are names members in the initialiser list.
		newNodes = []
		decl_type = [ds.name for ds in decl_specifier.children]
		decl_type_node = self.get_type(parentast, decl_specifier)

		def _any_combination(target, acceptable):
			"""
			Returns True if target has any combination of "acceptable" in it
			(including zero elements - ie if target is [])
			"""
			for item in target:
				if item not in acceptable:
					return False
			return True

		if 'typedef' in decl_type:
			node_template = TypeNode(None, source = init_decl_list)
			node_template.add_attribute('meta_type', 'alias')
		elif 'const' in decl_type:
			node_template = Node(None, 'type_instance', source = init_decl_list)
			node_template.add_attribute('meta_type', 'const')
		elif init_decl_list and _any_combination(decl_type, ['static', 'inline']):
			node_template = Node(None, 'type_instance', source = init_decl_list)
		elif any_in_list(CLASS_SPECIFIERS, decl_type):
			node_template = Node(None, 'type_instance', source = init_decl_list)
		elif init_decl_list is not None:
			node_template = Node(None, 'type_instance', source = init_decl_list)
			

		if init_decl_list is None:
			# Forward declaration.
			newNodes.append(decl_type_node)
		else:
			# Build declarations from the node template.
			for decl in declarators:
				if decl.name == 'attribute':
					# FIXME: Attributes are ignored right now
					continue
				newNode = node_template.copy()
				newNode.leaf = decl.leaf
				newNode.add_attribute('indirection',
						decl.get_single_attribute('indirection', ''))
				newNode.add_child(self._wrap_node('target', newNode, decl_type_node))
				newNode.add_child(decl)
				if 'static' in decl_type:
					newNode.attributes['c_static'] = True
				if 'inline' in decl_type:
					newNode.attributes['c_inline'] = True
				newNodes.append(newNode)
		return newNodes

	def _init_declarator_list(self, parentast, pt):
		return self._some_declarator_list(parentast, pt)
	
	def _some_declarator_list(self, parentast, pt):
		"""
		Init declarators are separated in source using commas.
		"""
		assert pt.name in ("init_declarator_list", "member_declarator_list")

		init_declarators = []
		for child in pt.children:
			init_declarators.append(self.init_declarator(parentast, child))
		return init_declarators
	
	def init_declarator(self, parentast, pt):
		"""
		Return an init_declarator node.

		We used to return:
		 name
		 expression
		 result
		 indirection

		This now becomes a Node with name "name", child "expression", attribute
		"indirection", a result, and possibly other children
		"""
		decl = Node(None, 'declarator')
		if pt.name == 'member_declarator_bitfield':
			decl.leaf = pt.leaf
			decl.add_child(pt.get_child('expression'))
			try:
				decl.result = evaluate(pt.get_child('expression'), parentast)
			except Exception:
				decl.result = pt.get_child('expression').result
		elif pt.name == 'attribute':
			attribute_pt = pt.children[0]
			# FIXME: Attribute support
			decl.set_name('attribute')
			decl.set_leaf(attribute_pt.name)
		elif pt.name in ("init_declarator", "member_declarator"):

			indirection = ''

			# Now we want this:
			# declarator *
			#  direct_declarator ID
			#
			# ... or any of these :

			# direct_declarator ID   <- function declaration
			#  parameter_list
			#  type_qualifier
			#  exception_specification

			# direct_declarator ID <- class instantation / array declaration
			#  expression

			# direct_declarator ID <- ?

			# declarator <- function ptr
			# declarator_suffixes

			decl_pt = pt.get_child('declarator')
			direct_decl_pt = pt.get_child('direct_declarator')
			decl_suffixes_pt = None

			# We want to get to a direct_declarator if it exists. This is a convoluted
			# process -- probably best to refer to the relevant grammar section.

			while decl_pt is not None:
				if decl_pt.leaf == '*':
					indirection += '*'
				direct_decl_pt = decl_pt.get_child('direct_declarator')
				if direct_decl_pt:
					break
				else:
					decl_pt = decl_pt.get_child('declarator')
					assert decl_pt

			decl.add_attribute('indirection', indirection)
			# Now we are down to direct_declarator.
			if direct_decl_pt.has_child('declarator'):
				# This is a function pointer - we don't support these too well.
				decl_pt = direct_decl_pt.get_child('declarator')
				direct_decl_pt = decl_pt.get_child('direct_declarator')

			decl.leaf = direct_decl_pt.leaf
			expr = pt.get_child('expression')
			if not expr and pt.has_child('initializer'):
				expr = pt.get_child('initializer').get_child('expression')
			ast_expr = None
			if expr:
				ast_expr = get_expression(parentast, expr, self)
			if ast_expr:
				decl.add_child(ast_expr)
		return decl

	def _function_parameter(self, ast, pt):
		"""
		Looks like this:
		parameter_declaration
		 declaration_specifiers
		 direct_declarator | declarator
		"""
		declaration_specifiers = pt.get_child('declaration_specifiers')

		node = Node(ast, 'parameter', source = pt)

		target_ast = self._wrap_node('target', ast,
				self.get_type(node, declaration_specifiers))
		node.add_child(target_ast)

		indirection = ''
		if pt.get_child('direct_declarator'):
			final_declarator = pt
		else:
			declarator = pt.get_child('declarator')
			final_declarator = declarator
			while declarator:
				indirection += declarator.leaf
				final_declarator = declarator
				declarator = declarator.get_child('declarator')

		node.leaf = final_declarator.get_child('direct_declarator').leaf
		indirection_ast = Node(node, 'indirection', leaf = indirection, source = final_declarator)
		node.add_child(indirection_ast)

		return node

	def _statements(self, ast, pt):
		"""
		Returns a list of parsed statements from "ast".
		"""
		results = None

		while pt.name == 'statement':
			pt = pt.children[0]

		if pt.name == 'jump_statement':
			results = [self._jump_statement(ast, pt)]
		elif pt.name == 'expression':
			results = [get_expression(ast, pt, self)]
		elif pt.name == 'selection_statement':
			results = [self._selection_statement(ast, pt)]
		elif pt.name == 'case_statement':
			results = [self._case_statement(ast, pt)]
		elif pt.name == 'default_statement':
			results = [self._default_statement(ast, pt)]
		elif pt.name == 'iteration_statement':
			results = [self._iteration_statement(ast, pt)]
		elif pt.name == 'compound_statement':
			results = self._compound_statement(ast, pt)
		elif pt.name == 'declaration':
			results = self.declaration(ast, pt)
		elif pt.name == 'asm_block':
			results = [UnfinishedNode('asm_block')]
		elif pt.name == 'blank':
			results = []
		else:
			pt.print_tree()
			raise Exception("Unknown statement '%s'" % (pt.name))

		return results

	def _compound_statement(self, ast, pt):
		"""
		Return a node representing a compound statement.
		"""
		statement_list = pt.get_child('statement_list')

		results = []

		# The compound statement may be empty - {} - in which case no statement
		# list node is present.
		if statement_list:
			for statement_pt in statement_list.children:
				results.extend(self._statements(ast, statement_pt))

		return results

	def _jump_statement(self, ast, pt):
		node = Node(ast, 'exit', source = pt)

		if pt.leaf == 'return':
			expression = pt.get_child('expression')
			if expression:
				node.add_child(expression)
		elif pt.leaf in ('break', 'continue'):
			# No arguments
			pass
		else:
			raise Exception("Unknown jump statement type %s" % (pt.leaf))

		return node
	
	def _selection_statement(self, ast, pt):
		"""
		Handle "switch() {} and if() {}"
		
		The PT's two children should be an expression and a compound_statement.

		We rewrite switch statements to be multiple "if" statements -
		possibly stupidly...
		"""
		if pt.leaf == 'switch':
			return self._selection_statement_switch(ast, pt)
		elif pt.leaf == 'if':
			return self._selection_statement_if(ast, pt)
		else:
			raise Exception("Unknown selection_statement %s" % (pt.leaf))
	
	def _selection_statement_if(self, ast, pt):
		""" Handle "if" statement. See caller for more info. """
		condition_pt = pt.children[0]
		cases_pt = pt.children[1]

		expr_ast = get_expression(ast, condition_pt, self)
		statements_ast = Node(ast, "body", source = cases_pt)
		
		if_ast = Node(ast, 'if', [expr_ast, statements_ast], source = pt)
		statements_ast.add_children(self._statements(statements_ast, cases_pt))

		# Handle "else" cases.
		else_clause_pt = pt.get_child('else')
		if else_clause_pt:
			else_statements_pt = else_clause_pt.child()

			else_clause_ast = Node(if_ast, 'else', source = else_clause_pt)
			if_ast.add_child(else_clause_ast)

			statements_list = self._statements(else_clause_ast, else_statements_pt)

			else_clause_ast.add_children(statements_list)

		return if_ast

	def _selection_statement_switch(self, ast, pt):
		""" Handle "switch" statement. See caller for more info.
		Output of type
		switch
		 expression (to switch on)
		 expression (actions)
		"""
		# FIXME: Do we handle fall-through switching at all?
		expr_pt, cases_pt = pt.children

		expr_ast = get_expression(ast, expr_pt, self)
		statements_ast = self._statements(ast, cases_pt)

		return Node(ast, 'switch', children = [expr_ast] + statements_ast)
	
	def _case_statement(self, ast, pt):
		expr_pt, statement_pt = pt.children
		
		expr_ast = get_expression(ast, expr_pt, self)
		statements_ast = self._statements(ast, statement_pt)

		return Node(ast, 'case', children = [expr_ast] + statements_ast)

	def _default_statement(self, ast, pt):
		statement_pt = pt.child()
		statements_ast = self._statements(ast, statement_pt)

		return Node(ast, 'default', children = statements_ast)
	
	def _iteration_statement(self, ast, pt):
		if pt.leaf == 'while':
			return self._iteration_statement_while(ast, pt)
		elif pt.leaf == 'do':
			return self._iteration_statement_do(ast, pt)
		elif pt.leaf == 'for':
			return self._iteration_statement_for(ast, pt)
		else:
			pt.print_tree()
			raise Error("Unknown iteration statement %s" % (pt.leaf))

	def _iteration_statement_while(self, ast, pt):
		expr_pt, statement_pt = pt.children

		expr_ast = get_expression(ast, expr_pt, self)
		statements_ast = Node(ast, "body", source = statement_pt)
		statements_ast.add_children(self._statements(statements_ast, statement_pt))

		return Node(ast, 'while', [expr_ast, statements_ast], source = pt)
	
	def _iteration_statement_for(self, ast, pt):
		init_pt, while_pt, post_pt, body_pt = pt.children

		# init_pt is an expression or a declaration.
		if init_pt.name == 'blank':
			init_ast = Node(ast, 'blank')
		elif self._is_declaration(init_pt):
			init_ast = self.declaration(init_pt)
		else:
			init_ast = [get_expression(ast, init_pt, self)]

		if while_pt.name != 'blank':
			while_ast = get_expression(ast, while_pt, self)
		else:
			while_ast = Node(ast, 'blank')

		if post_pt.name != 'blank':
			post_ast = get_expression(ast, post_pt, self)
		else:
			post_ast = Node(ast, 'blank')

		body_ast = self._statements(ast, body_pt)

		return Node(ast, 'for', init_ast + [while_ast, post_ast] + body_ast)
		

	def _construct_expr_from_value(self, parent_ast, value):
		""" Cheesy hack to encode full (non-typechecked) values of expressions
		as a string """
		expr_ast = Node(parent_ast, 'expression', leaf = 'raw', source = parent_ast)
		expr_ast.add_attribute('value', value)
		return expr_ast
	
	def find_or_create_type(self, parentast, pt):
		# Called when we're not sure if the current type must be created
		# or not.
		# FIXME: We could/should extend this to other types.
		assert pt.name in CLASS_SPECIFIERS
		
		# The class may exist
		ast = None
		if pt.leaf:
			ast = getTypenode(pt.leaf, parentast)
		# There is one case where getTypenode can succeed and yet the type
		# can be defined below: when a previous forward declaration is
		# being defined.
		if ast and not ast.get_child('members') and pt.get_child('members'):
			# We're defining it here - so forget about the previous reference
			# and use this one.
			ast = None
		
		# By this point the name isn't declared, or we need to define a 
		# forward reference.
		if not ast:
			# The class doesn't exist - create it
			ast = self.create_type(parentast, pt)
		
		# Done: if we're None at this point something is broken.
		assert ast is not None
		return ast
		
	def _member_declarator_list(self, parentast, pt):
		"""
		Return a list of instance names.
		"""
		return self._some_declarator_list(parentast, pt)

	def _init_declarator_get_value(self, pt):
		pass
	
def gen(baseast, pt, filename, **kwargs):
	astgen = ASTGen(**kwargs)
	return astgen.convert(baseast, pt, filename)
