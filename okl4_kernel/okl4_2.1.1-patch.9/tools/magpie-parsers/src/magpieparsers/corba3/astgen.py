"""
AST helper functions
"""

from magpieparsers.parser_common import Node, TypeNode
from magpieparsers.error import UnknownTypeError, SemanticError, SymbolDefinedTwiceError, \
		InvalidDiscriminatorError, InheritanceError, InvalidTypeUsageError, \
		CannotReconcileTypesError, DimensionOutOfRangeError
from magpieparsers.types import infogripper
from magpieparsers.cplusplus.normalise import normalise_type_list

import astexpr

# FIXME: The following overrides infogripper scopes (for no reason?).
SCOPES = ('type', 'function', 'MagpieAST')

class Error(Exception):
	pass

def get_corba_or_c_type(type_name, ast):
	try:
		return infogripper.getBasicTypenode(type_name, ast)
	except UnknownTypeError:
		c_type_list = normalise_type_list([type_name])
		c_type_name = ' '.join(c_type_list)
		return infogripper.getBasicTypenode(c_type_name, ast)
	
class ASTGen(object):
	def __init__(self, ast):
		self.current_ast = ast
		
	def _dupes_check(self, ast):
		if ast.leaf is None:
			return False
#		elif ast.name == 'type' and 'forward' in ast.get_attribute('meta_type'):
#			# Forward declaration of a type - we can do this as often as
#			# we like.
#			return False
		else:
			found = infogripper.getAllNodes(ast.leaf, ast, None)
			if len(found) == 0:
				raise Exception("Internal error - couldn't find node")
			elif len(found) == 1:
				return False
			elif len(found) == 2:
				other_nodes = [item for item in found if item is not ast]
				if len(other_nodes) == 1:
					raise SymbolDefinedTwiceError(ast.leaf, other_nodes[0], ast)
				else:
					raise Exception("Internal error - identical node references present in AST")
			else:
				raise Exception("Internal error - More than 2 duplicates found for %s" % (ast.leaf))
			return False

	def convert(self, ast,filename, pt):
		return self.translation_unit(ast, filename, pt)
		
	def translation_unit(self, ast, filename, pt):
		for node in pt.children:
			if node.type == 'decorated_definition':
				self.decorated_def(ast, node)
				
			elif node.type == 'import_dcl':
				imp_node = self.import_dcl(node)
				ast.add_child(imp_node)
			else:
				#print 'node = ', node.type
				ast.add_child(UnknownNode(None, node, source = pt))
		return ast
		
	def import_dcl(self, pt):
		ast = Node(None, None, source = pt)
		ast.type = "cimport" #pt.leaf
		"""
		if pt.leaf == 'import':
			#imported_scope
			child = pt.the('imported_scope').children[0]
		elif pt.leaf == 'cimport':
			child = pt.the('anglequoted_scope').children[0]
		"""
		
		child = pt.children[0].children[0]
		assert child is not None
		
			
		
		if child.type == 'string_literal':
			ast.leaf = child.leaf
		elif child.type == 'scoped_name':
			ast.leaf = self.scoped_name(child)
		elif child.type == 'anglequoted_string_literal':
			ast.leaf = child.leaf
		else:
			ast = Node(None, child, source = pt)
			ast.type = 'import'
		return ast
		
	def decorated_def(self, ast, pt):
		decorators = []
		definitions = []
		for child in pt.children:
			if child.type == 'decorator':
				dec_children = self.decorator_elements(ast, child)
				dec_node = Node(ast, 'decorator', dec_children, source = child)
				decorators.append(dec_node)
			elif child.type == 'definition':
				self.definition(ast, child, decorators)
				decorators = []
			else:
				ast.append(UnknownNode(None, child, source = pt))

	def decorator_elements(self, ast, pt):
		children = []
		for child in pt.children:
			if child.type == 'decorator_element':
				dec_el = self.decorator_element(ast, child)
				children.append(dec_el)
			else:
				children.append(UnknownNode(child, source = pt))
		#print '\n', children
		return children
				
	def decorator_element(self, ast, pt):
		dec_node = Node(None, 'annotation', source = pt)
		for child in pt.children:
			if child.type == 'identifier':
				dec_node.leaf = child.leaf
			elif child.type == 'expr_list':
				dec_node.add_children(self.expr_list(ast, child, True))
			else:
				dec_node.add_child(UnknownNode(child, source = pt))
		return dec_node
		
	def expr_list(self, scope_ast, pt, evaluate = False):
		ex_list = []
		for child in pt.children:
			if child.type == 'const_exp':
				exp_node = self.getExpression(scope_ast, child)
				#print exp_node.leaf, exp_node.result
				ex_list.append(exp_node)
			else:
				ex_list.append(UnknownNode(child, source = pt))
		return ex_list
		
	def getExpression(self, parent_ast, pt):
		""" Convert a parse-tree expression to an IDLAST exprsesion.
		Does not evaluate the expression.
		"""
		return astexpr.get_expression(parent_ast, pt)
		
	def getValue(self, node):
		if node.children[0].type == 'integer_literal':
			return int(eval(node.children[0].leaf))
		elif node.children[0].type == 'floating_pt_literal':
			return float(eval(node.children[0].leaf))
		elif node.children[0].type == 'boolean_literal':
			return bool(eval(node.children[0].leaf))
		else:
			return None #node.leaf
		
	def definition(self, ast, pt, decorators):
		pt_child = pt.children[0]
		if pt_child.type == 'type_dcl':
			self.type_dcl(ast, pt_child, decorators)
		elif pt_child.type == 'const_dcl':
			self.const_dcl(ast, pt_child, decorators)
		elif pt_child.type == 'except_dcl':
			self.except_dcl(ast, pt_child, decorators)
		elif pt_child.type == 'interf':
			self.interf(ast, pt_child, decorators)
		elif pt_child.type == 'module':
			self.module(ast, pt_child, decorators)
		elif pt_child.type == 'value':
			self.value_def(ast, pt_child, decorators)
		elif pt_child.type == 'type_id_dcl':
			self.type_id_dcl(ast, pt_child, decorators)
		elif pt_child.type == 'type_prefix_dcl':
			self.type_prefix_dcl(ast, pt_child, decorators)
		elif pt_child.type == 'event':
			self.event_def(ast, pt_child, decorators)
		elif pt_child.type == 'component':
			self.component(ast, pt_child, decorators)
		elif pt_child.type == 'home_dcl':
			self.home_dcl(ast, pt_child, decorators)
		else:
			ast.add_child(UnknownNode(None, pt_child, source = pt))
	
	def type_dcl(self, ast, pt, decorators):
		assert not decorators
		# FIXME: SemanticError if duplicate here.
		if pt.leaf == 'typedef':
			# Typdef consists of two parts:
			# 1. type_declarator -> type_spec: the target type
			# 2. type_declarator -> declarators: the new alias name
			for declarator in pt.the('type_declarator').the('declarators')['declarator']:
				typedef_ast = self.type_from_declarator(ast, declarator)
				typedef_ast.add_attribute('meta_type', 'alias')
				new_node = typedef_ast
				ast.add_child(new_node)

				# NB we pass "ast" here because any new types we create end up going in the
				# same scope as the typedef.
				typeref, is_new_type = self._get_target_type(ast,
						pt.the('type_declarator').the('type_spec'), defining_scope = typedef_ast)

				typedef_ast.add_child(typeref)

				self._dupes_check(typeref)
			
		elif pt.children[0].name in ('struct_type', 'enum_type', 'union_type'):
			new_node = self.constr_type_spec(ast, pt.children[0], defining_scope = ast)
			# NOTE! Since we pass "defining_scope" here, the child is *already* added
			# and does not need to be added again, hence no ast.add_child(new_node) here.

		elif pt.children[0].type == 'simple_declarator':
			typenode = TypeNode(ast, source = pt)
			typenode.leaf = pt.children[0].leaf
			typenode.add_attribute('meta_type', 'native')
			new_node = typenode
			ast.add_child(new_node)
			
		elif pt.children[0].type == 'constr_forward_decl':
			typenode = TypeNode(ast, source = pt)
			typenode.leaf = pt.the('constr_forward_decl').the('identifier').leaf
			typenode.add_attribute('meta_type', 'forward')
			#typetype = 'forward ' + pt.the('constr_forward_dcl').leaf
			typenode.add_attribute('meta_type' , pt.the('constr_forward_decl').leaf)
			new_node = typenode
			ast.add_child(new_node)
		else:
			new_node = UnknownNode(None, pt.children[0], source = pt.children[0])
			ast.add_child(new_node)

		self._dupes_check(new_node)

	def const_dcl(self, ast, pt, decorators):
		"""
		Create a constant declaration.
		They look like this:
		1.	type_instance blah  (3)
			+- meta_type = ['const']
		2.	+- value = [1]
		3.	 info  (4)
			 +- target = [<Node object type:unsigned int>]
		"""
		assert not decorators
		# 1: Set up the base.
		const_node = Node(ast, 'type_instance', source = pt)
		const_node.add_attribute('meta_type', 'const')
		const_node.leaf = pt.the('identifier').leaf

		# 2: Now store the target of the decl.
		target_type, is_new_type = self._get_target_type(const_node, pt.the('const_type'),
				defining_scope = const_node)
		# ... NB if we have a new type it will be added here already as a "type" node.
		# But we always add as a "target" node too.
		const_node.add_child(target_type)

		# 3: Add the expression result wrapped in a declarator (for c++ compat.)
		declarator_wrapper = Node(const_node, 'declarator', source = pt)
		const_node.add_child(declarator_wrapper)

		expr = self.getExpression(const_node, pt.the('const_exp'))
		declarator_wrapper.add_child(expr)

		# 4: Ensure the type of the expression can be coerced to the type of the target.
		if not infogripper.can_coerce_expr(expr, target_type.the('type')):
			raise CannotReconcileTypesError(expr.the('type').leaf,
					target_type.the('type').leaf,
					const_node)

		# 5: Hurrah!
		ast.add_child(const_node)
		self._dupes_check(const_node)

	def _get_target_type(self, ast, pt, wrapper = None, defining_scope = None):
		"""
		A complicated function with side effects.
		
		Either searches for (if it is a reference) or creates (otherwise) the type
		specified by <pt>. The scope to begin searching is passed in <ast>.

		The type is returned. If a new type is created, it is always returned in a wrapper, named
		'target'. If <wrapper> (a string) is passed in, that name is used instead of <target>.

		If <defining_scope> is supplied and the type is created, the new type (sans wrapper)
		is stored in <defining_scope>. If the new type is an enumeration, more than one type
		node is created in <defining_scope>.
		"""
		new_type = False

		# Walk to the lowest child.
		while pt.name in ('const_type', 'type_spec', 'simple_type_spec',
				'base_type_spec', 'switch_type_spec', 'param_type_spec',
				'constr_type_spec',) \
				and len(pt.children) == 1:
			pt = pt.children[0]

		if defining_scope:
			ast = defining_scope # NB This means AST must already be part of the tree.

		if pt.name in ('struct_type', 'union_type', 'enum_type'):
			typeref = self.constr_type_spec(ast, pt, defining_scope = defining_scope)
			new_type = True
			# NOTE! A new type is created here, but we do not add it to defining_scope, 
			# because constr_type_spec does that for us. This could be improved :(
		elif pt.name == 'template_type_spec':
			typeref = self.template_type_spec(ast, pt)
			new_type = True
			if defining_scope:
				defining_scope.add_child(typeref)
				self._dupes_check(typeref)
		elif pt.name == 'integer_type':
			name = self.integer_type(ast, pt)
			typeref = get_corba_or_c_type(name, ast)
		elif pt.name == 'char_type':
			typeref = get_corba_or_c_type('char', ast)
		elif pt.name == 'wide_char_type':
			typeref = get_corba_or_c_type('wchar', ast)
		elif pt.name == 'boolean_type':
			typeref = get_corba_or_c_type('boolean', ast)
		elif pt.name == 'floating_pt_type':
			name = self.floating_pt_type(ast, pt)
			typeref = get_corba_or_c_type('name', ast)
		elif pt.name == 'string_type':
			typeref = self.string_type(ast, pt)
			new_type = True
			if defining_scope:
				defining_scope.add_child(typeref)
				self._dupes_check(typeref)
		elif pt.name == 'wide_string_type':
			typeref = self.wide_string_type(ast, pt)
			new_type = True
			if defining_scope:
				defining_scope.add_child(typeref)
				self._dupes_check(typeref)
		elif pt.name == 'void_type':
			typeref = get_corba_or_c_type('void', ast)
		elif pt.name == '_type':
			raise Error("Unknown type ('fixed'?)")
		elif pt.name == 'scoped_name':
			typeref = self.scoped_name_type(ast, pt)
		elif pt.name == 'octet_type':
			typeref = infogripper.getBasicTypenode('octet', ast)
		elif pt.name == 'object_type':
			typeref = infogripper.getBasicTypenode('Object', ast)
		else:
			pt.print_tree()
			raise Exception()
			typeref = TypeNode(ast, None, source = pt)
			typeref.leaf = 'UNKNOWN'

		# Wrap the type.
		if wrapper is None:
			wrapper = 'target'

		typeref = self._target_wrap(ast, typeref, wrapper)

		# All done.
		return typeref, new_type

	def _target_wrap(self, parent_ast, dest_ast, wrapper_name):
		result = Node(parent_ast, wrapper_name, source = dest_ast)
		result.add_child(dest_ast)
		return result

	def except_dcl(self, ast, pt, decorators):
		excep_node = Node(ast, 'exception', None, source = pt)
		excep_node.leaf = pt.the('identifier').leaf
		if pt.the('opt_member_list') != None:
			excep_node.add_children(self.member_list(ast, pt.the('opt_member_list')))
		ast.add(excep_node)
		self._dupes_check(excep_node)

	def type_id_dcl(self, ast, pt, decorators):
		assert not decorators
		type_id = Node(ast, name='typeid', source = pt)
		type_id.leaf = self.scoped_name(ast, pt.the('scoped_name'))
		type_id.add_attribute('value', pt.the('string_literal').leaf)
		ast.add_child(type_id)
		self._dupes_check(type_id)

	def type_prefix_dcl(self, ast, pt, decorators):
		assert not decorators
		type_id = Node(ast, name='typeprefix', source = pt)
		type_id.leaf = self.scoped_name(ast, pt.the('scoped_name'))
		type_id.add_attribute('value', pt.the('string_literal').leaf)
		ast.add_child(type_id)
	
	def interf(self,ast,  pt, decorators):
		# ... create the type
		interface = Node(ast, 'type', source = pt)
		interface.add_attribute('meta_type', 'interface')
		# ... add modifiers
		if pt.leaf == 'abstract' or pt.leaf == 'local':
			interface.add_attribute('modifier', pt.leaf)
		# ... set interface name
		interface.leaf = pt.the('identifier').leaf

		# If the interface already exists in incomplete form, use that instead.
		old_interfaces = infogripper.getAllNodes(interface.leaf, ast, 'type')
		if old_interfaces:
			# Found a previous declaration; use that.
			assert len(old_interfaces) == 1
			interface = old_interfaces[0]
		else:
			# Okay, it doesn't exist already: add it.
			ast.add_child(interface)

		# While we're building it, add the "incomplete" attribute.
		interface.add_attribute('incomplete', True)
		is_incomplete = True
		for child in pt.children:
			if child.type == 'interface_dcl':
				is_incomplete = False
				#print child
				if child.the('interface_header') != None:
					scope_list = child.the('interface_header').the('interface_inheritance_spec').the('scoped_name_list')
					for scope_name in scope_list['scoped_name']:
						inher_node = Node(interface, 'inherits', leaf = self.scoped_name(scope_name), source = child)
						inher_node.add_child(infogripper.find_scope(interface, inher_node.leaf))

						# Ensure we're not inheriting from the same class twice.
						self.check_not_inherited(interface, inher_node.children[0], error_infonode = scope_name)
						interface.add_child(inher_node)
						
						
				if child.the('interface_body') != None:
					for export_child in child.the('interface_body').children:
						self.export(interface, export_child)

		# Remove "incomplete" if the interface was fully-defined above.
		if not is_incomplete:
			interface.del_attributes('incomplete')

		interface.add_children(decorators)

		self._dupes_check(interface)
	
	def check_not_inherited(self, target_ast, new_parent_ast, error_infonode = None):
		for inherits in target_ast['inherits']:
			if inherits.children[0] is new_parent_ast:
				raise InheritanceError(node = error_infonode, text = "Can't directly inherit from the same parent more than once")
		
	def module(self, ast, pt, decorators):
		assert not decorators
		module_name = pt.the('identifier').leaf
		mod = infogripper.getNode(module_name, ast, 'module')
		mod_is_new = False
		if mod is None:
			mod = Node(ast, 'module', None, source = pt)
			ast.add_child(mod)
			mod_is_new = True
		for child in pt.children:
			if child.type == 'identifier':
				mod.leaf = child.leaf
				# Modules can be re-opened, hence the check:
				if mod_is_new:
					self._dupes_check(mod)
			elif child.type == 'definition_list':
				for def_child in child.children:
					self.decorated_def(mod, def_child)
				#def_child = self.definition(def_child)
				#mod.add_child(def_child)
			else:
				mod.add_child(UnknownNode(None,child, source = pt))


	
	def value_def(self, ast, pt, decorators):
		assert not decorators

		if pt.children[0].type == 'value_dcl':
			ast.add_child(self.value_dcl(ast, pt.children[0]))
		elif pt.children[0].type == 'value_abs_dcl':
			ast.add_child(self.value_abs_dcl(ast, pt.children[0]))
		elif pt.children[0].type == 'value_box_dcl':
			val_node = Node(ast, 'valuetype', None, source = pt.children[0])
			val_node.leaf = pt.children[0].the('identifier').leaf
			val_node.add_child(self.type_spec(val_node, pt.children[0].the('type_spec')))
			ast.add_child(val_node)
		elif pt.children[0].type == 'value_custom_dcl':
			val_node = self.value_dcl(ast, pt.children[0].the('value_dcl'))
			val_node.add_attribute('modifier', 'custom')
			ast.add_child(val_node)
		elif pt.children[0].type == 'value_forward_dcl':
			val_node = Node(ast, 'valuetype', None, source = pt.children[0])
			val_node.leaf = val_node.leaf = pt.children[0].the('identifier').leaf
			val_node.add_attribute('modifier', 'forward')
			ast.add_child(val_node)
		else:
			ast.add_child(UnknownNode(None, pt.children[0].type, source = pt.children[0]))
			
	def event_def(self, ast, pt, decorators):
		assert not decorators
		event_node = Node(ast, 'event', source = pt)
		child = pt.children[0]
		if child.type == 'event_abs':
			event_node.add_attribute('modifier', 'abstract')
			event_node.leaf = child.the('event_abs_dcl').the('event_header').the('identifier').leaf
			if child.the('event_abs_dcl') != None:
				event_node.add_child(self.value_inheritance_spec(child.the('event_abs_dcl').the('value_inheritance_spec')))
				export_node = Node(event_node, 'export', exp_list, source = child)
				for exp_child in child.the('event_abs_dcl')[export]:
					self.export(export_node, exp_child)
				if export_node.children:
					event_node.add_child(export_node)
		elif child.type == 'event_custom':
			event_node.add_attribute('modifier', 'custom')
			event_node.leaf = child.the('event_custom').the('event_header').the('identifier').leaf
			if child.the('event_elem_dcl') != None:
				event_node.add_child(self.value_inheritance_spec(child.the('event_elem_dcl').the('value_inheritance_spec')))
				export_node = Node(event_node, 'export', exp_list, source = child)
				for exp_child in child.the('event_elem_dcl')[export]:
					self.export(export_node, exp_child)
				if export_node.children:
					event_node.add_child(export_node)
		elif child.type == 'event_dcl':
			event_node.leaf = child.the('event_dcl').the('event_header').the('identifier').leaf
			if child.the('event_elem_dcl') != None:
				event_node.add_child(self.value_inheritance_spec(child.the('event_elem_dcl').the('value_inheritance_spec')))
				export_node = Node(event_node, 'export', exp_list, source = child)
				for exp_child in child.the('event_elem_dcl')[export]:
					self.export(export_node, exp_child)
				if export_node.children:
					event_node.add_child(export_node)
		
		else:
			event_node = UnknownNode(None, child, source = pt)

		ast.add_child(event_node)
	
	def component(self,ast, pt, decorators):
		assert not decorators
		component = Node(ast, 'component', None, source = pt)
		component.leaf = pt.the('identifier').leaf
		dcl = pt.the('component_dcl')
		if  dcl != None:
			inh = dcl.the('component_inheritance_spec')
			if inh != None:
				inher_node = Node(component, 'inherits', leaf = self.scoped_name(interface, scope_name), source = dcl)
				component.add_child(inher_ndoe)
				inher_node.add_child(infogripper.find_scope(interface, inher))
			
			sup = dcl.the('supported_interface_spec')
			if sup != None:
				supnode = Node(component, 'support', source = sup)
				name_list = []
				for name in sup['scoped_name']:
					newname = self.scoped_name(name)
					name_list.append(newname)
					target_list.append(getTypenode(newname, component))
				supnode.leaf = name_list
				supnode.add_attribute('target_scope_list', target_list)
				component.add_child(supnode)
			
			bod = dcl.the('component_body')
			if bod != None:
				exp_list = self.component_body(component, bod)
				if exp_list != []:
					component.add_children(exp_list)

		ast.add_child(component)
	
	def home_dcl(self, ast, pt, decorators):
		assert not decorators
		homenode = Node(ast, 'home', source = pt)
		#
		#	HEADER
		#
		homeheader = pt.the('home_header')
		homenode.leaf = homeheader.the('identifier').leaf
		if homeheader.the('home_inheritance_spec') != None:
			inhnode = Node(homenode, 'inheritance', source = homeheader)
			inhnode.leaf = self.scoped_name(homeheader.the('home_inheritance_spec').the('scoped_name'))
			inhnode.add_attribute
			homenode.add_child(inhnode)
		if homeheader.the('supported_interface_spec') != None:
			supnode = Node(homenode, 'support', source = homeheader)
			namelist = []
			for name in homeheader.the('supported_interface_spec')['scoped_name']:
				namelist.append(self.scoped_name(name))
			supnode.leaf = namelist
			homenode.add_child(supnode)
		mannode = Node(homenode, None, name='manages', source = pt)
		mannode.leaf = self.scoped_name(homheader.the('scoped_name'))
		mannode.add_attribute('target_scope', infogripper.find_scope(mannode.leaf))
		if homeheader.the('primary_key_spec') != None:
			mannode.add_attribute('primarykey', infogripper.find_scope(self.scoped_name(homenode, homeheader.the('primary_key_spec').the('scoped_name'))))
		homenode.add_child(mannode)
		#
		#	BODY
		#
		homebody = pt.the('home_body')
		for child in homebody.children:
			if child.children[0].type == 'export':
				self.export(homenode, child.children[0])
			elif child.children[0].type == 'factory_dcl':
				homenbode.add_child(self.factory_dcl(homenode, child.children[0]))
			elif child.children[0].type == 'finder_dcl':
				homenode.add_child(self.finder_dcl(homenode, child.children[0]))
			else:
				homenode.add_child(UnknownNode(None, child.children[0]), source = pt)
		
		ast.add_child(homenode)
	
	def factory_dcl(self,ast, pt):
		fac_node = Node(ast, None, name = 'factory', source = pt)
		fac_node.leaf = pt.the('identifier').leaf
		fac_node.add_children(self.init_param_decls(fac_node, pt.the('init_param_decls')))
		if pt.the('raises_expr') != None:
			raises = Node(fac_node, name='raises', source = pt.the('raises_expr'))
			leaflist = []
			target_list = []
			for node in child.the('scoped_name_list').children:
				if node.type == 'scoped_name':
					newname = self.scoped_name(node)
					target_list = getTypenode(newname, ast)
					leaflist.append()
			raises.leaf = leaflist
			raises.add_attribute('target_scope_list', target_list)
			fac_node.add_child(raises)
		return fac_node
	
	def finder_dcl(self, ast, pt):
		fac_node = Node(ast, None, name = 'finder', source = pt)
		fac_node.leaf = pt.the('identifier').leaf
		fac_node.add_children(self.init_param_decls(fac_node, pt.the('init_param_decls')))
		if pt.the('raises_expr') != None:
			raises = Node(fac_node, name='raises', source = pt)
			leaflist = []
			target_list = []
			for node in child.the('scoped_name_list').children:
				if node.type == 'scoped_name':
					newname = self.scoped_name(node)
					target_list = getTypenode(newname, ast)
					leaflist.append(newname)
			raises.leaf = leaflist
			raises.add_attribute('target_scope_list', target_list)
			fac_node.add_child(raises)
		return fac_node
		
	
	def component_body(self, ast, pt):
		exp_list = []
		for exp in pt.children:
			exp_node = Node(ast, 'export', source = exp)
			if exp.children[0].type == 'provides_dcl':
				exp_node.add_attribute('type', 'provides')
				exp_node.leaf = exp.children[0].the('identifier').leaf
				if exp.children[0].the('interface_type').the('scoped_name') != None:
					scopedname = self.scoped_name(exp.children[0].the('interface_type').the('scoped_name'))
					exp_node.add_attribute('name', scopedname)
				else:
					exp_node.add_attribute('name', 'Object')
			elif exp.children[0].type == 'uses_dcl':
				typestr = ''
				for child in exp.children[0]['uses_dcl']:
					typestr += child.leaf
				exp_node.add_attribute('type', typestr)
				exp_node.leaf = exp.children[0].the('identifier').leaf
				if exp.children[0].the('interface_type').the('scoped_name') != None:
					scopedname = self.scoped_name(exp.children[0].the('interface_type').the('scoped_name'))
					exp_node.add_attribute('name', scopedname)
				else:
					exp_node.add_attribute('name', 'Object')
			elif exp.children[0].type == 'emits_dcl':
				exp_node.add_attribute('type', 'emits')
				exp_node.leaf = exp.children[0].the('identifier').leaf
				scopedname = self.scoped_name(exp.children[0].the('scoped_name'))
				exp_node.add_attribute('name', scopedname)
			elif exp.children[0].type == 'publishes_dcl':
				exp_node.add_attribute('type', 'publishes')
				exp_node.leaf = exp.children[0].the('identifier').leaf
				scopedname = self.scoped_name(exp.children[0].the('scoped_name'))
				exp_node.add_attribute('name', scopedname)
			elif exp.children[0].type == 'consumes_dcl':
				exp_node.add_attribute('type', 'consumes')
				exp_node.leaf = exp.children[0].the('identifier').leaf
				scopedname = self.scoped_name(exp.children[0].the('scoped_name'))
				exp_node.add_attribute('name', scopedname)
			elif exp.children[0].type == 'attr_dcl':
				# FIXME: Not used?
				self.attr_dcl(exp_node, exp.children[0])
			else:
				exp_list.append(UnknownChild(None, exp))
			exp_list.append(exp_node)
		return exp_list
		
	
	def value_dcl(self, ast, pt):
		node = Node(ast, 'valuetype', None, source = pt)
		for child in pt.children:
			if child.type == 'identifier':
				node.leaf = child.leaf
			elif child.type == 'value_inheritance_spec':
				val_inh_node = self.value_inheritance_spec(node, self, child)
				node.add_child(val_inh_node)
			else:
				node.add_child(UnknownNode(child, source = pt))
		return node
	
	def value_abs_dcl(self, at, pt):
		node = Node(ast, 'valuetype', None, source = pt)
		node.add_attribute('modifier', 'abstract')
		for child in pt.children:
			if child.type == 'identifier':
				node.leaf = child.leaf
			elif child.type == 'value_abs_full_dcl':
				val_inh_node = self.value_inheritance_spec(node, self, child.the('value_inheritance_spec'))
				export_node = Node(val_inh_node, 'export', source = child)
				for exp_child in child[export]:
					self.export(export_node, exp_child)
				if export_node.children:
					val_inh_node.add_child(export_node)
			else:
				node.add_child(UnknownNode(None, child, source = pt))
		return node
		
	def value_inheritance_spec(self, ast, pt):
		if pt.children[0].type == 'value_value_inheritance_spec':
			inh_node = Node(ast, 'inheritance', None, source = pt.children[0])
			inh_node.add_attribute('relation', 'value')
			if pt.children[0].the('value_value_inheritance_spec') != None:
				inh_node.add_attribute('modifier', pt.children[0].the('value_value_inheritance_spec').leaf)
			name_list = []
			target_list = []
			for name in pt.the('value_value_inheritance_spec')['value_name']:
				newname = self.scoped_name(name.the('scoped_name'))
				name_list.append(newname)
				target_list.append(getTypenode(newname, ast))
			inh_node.leaf = name_list
			inh_node.add_attribute('target_scope_list',target_list)
			return inh_node
		elif pt.children[0].type == 'value_interface_inheritance_spec':
			inh_node = Node(ast, 'inheritance', None, source = pt.children[0])
			inh_node.add_attribute('relation', 'interface')
			name_list = []
			target_list = []
			for name in pt.the('value_interface_inheritance_spec')['interface_name']:
				newname = self.scoped_name(name.the('scoped_name'))
				name_list.append(newname)
				target_list.append(getTypenode(newname, ast))
			inh_node.add_attribute('target_scope_list',target_list)
			inh_node.leaf = name_list
			return inh_node
		else:
			return UnknownNode(None, pt, source = pt)
	
	def state_member(self, ast, pt):
		type_node = TypeNode(ast, None, source = pt)
		
		if pt.the('state_member') != None:
			type_node.leaf = child.leaf
		type_node.add_child(self.type_spec(ast, pt.the('type_spec')))
		decls = pt.the('declarators')
		decl_list = []
		for child in decls.children:
			decl_list.append(self.getDeclarator(child))
		dec_node = Node(type_node, 'declarator', None, source = pt)
		dec_node.leaf = decl_list
		type_node.add_child(dec_node)
		return type_node
	
#	def init_dcl(self, ast, pt):
#		type_node = Node(ast, None, name = 'factory', source = pt)
#		type_node.leaf = pt.the('identifier').leaf
#		if pt.the('init_param_decls') != None:
#			type_node.add_children(self.init_param_decls(type_node, pt.the('init_param_decls')))
#		if pt.the('raises_expr') != None:
#			raises = Node(type_node, name='raises', source = pt.the('raises_expr'))
#			leaflist = []
#			target_list = []
#			for node in child.the('scoped_name_list').children:
#				if node.type == 'scoped_name':
#					newname = self.scoped_name(node)
#					leaflist.append(newname)
#					target_list.append(getTypenode(newname, type_node))
#			raises.leaf = leaflist
#			type_node.add_child(raises)
#		return type_node
		
	def init_param_decls(self, ast, pt):
		decl_list = []
		for child in pt['init_param_decl']:
			param_node = Node(ast, 'parameter', None, source = child)
			type_ast, new_type = self._get_target_type(param_node, child.the('param_type_spec'),
					defining_scope = param_node)
			param_node.leaf = child.the('simple_declarator').leaf
			if child.the('init_param_attribute') != None:
				param_node.add_attribute('attribute', child.the('init_param_attribute').leaf)
			decl_list.append(param_node)
		return decl_list

	def export(self, ast, pt):
		decorators = []
		if pt.children[0].type == 'type_dcl':
			self.type_dcl(ast, pt.children[0], decorators)
		elif pt.children[0].type == 'const_dcl':
			self.const_dcl(ast, pt.children[0], decorators)
		elif pt.children[0].type == 'except_dcl':
			self.except_dcl(ast, pt.children[0], decorators)
		elif pt.children[0].type == 'attr_dcl':
			self.attr_dcl(ast, pt.children[0])
		elif pt.children[0].type == 'op_dcl':
			self.op_dcl(ast, pt.children[0])
		elif pt.children[0].type == 'type_id_dcl':
			self.type_id_dcl(ast, pt.children[0], decorators)
		elif pt.children[0].type == 'type_prefix_dcl':
			self.type_prefix_dcl(ast, pt.children[0], decorators)
		else:
			ast.add_child(UnknownNode(None, pt.children[0], source = pt.children[0]))
		
	def attr_dcl(self, ast, pt):
		attr_node = Node(ast, 'attribute', None, source = pt)
		ast.add_child(attr_node)

		type_node, new_type = self._get_target_type(attr_node, pt.children[0].the('param_type_spec'),
				defining_scope = attr_node)
		if pt.children[0].type == 'readonly_attr_spec':
			attr_node.add_attribute('mode', 'readonly')
			attr_node.add_child(self.attr_declarator(attr_node, pt.children[0].the('readonly_attr_declarator')))
			#self.attr_declarator(pt.children[0].the('readonly_attr_declarator'), type_node)
		elif pt.children[0].type == 'attr_spec':
			attr_node.add_attribute('mode', 'readwrite')
			attr_node.add_child(self.attr_declarator(attr_node, pt.children[0].the('attr_declarator')))
			#attr_node.add_child(self.attr_declarator(pt.children[0].the('attr_declarator')), type_node)
		else:
			attr_node = UnknownNode(None, pt.children[0], source = pt.children[0])
		#type_node.add_attribute('declarator', attr_node.the('declarator').leaf)
		#attr_node.add_child(type_node)

		
	def op_dcl(self,ast, pt):
		fcn = Node(ast, 'function', None, source = pt)
		ast.add_child(fcn)
		for child in pt.children:
			if child.type == 'decorator':
				#print child
				dec_children = self.decorator_elements(fcn, child)
				dec_node = Node(fcn, 'decorator', dec_children, source = child)
				fcn.add_child(dec_node)
			elif child.type == 'op_attribute':
				fcn.add_attribute('attribute', child.leaf)
			elif child.type == 'op_type_spec':
				#if child.leaf == 'void':
				#	target = getTypenode('void', fcn)
				#print 'trying to get typenode for: %s' %(child.the('param_type_spec'))
				#else:
				typenode = TypeNode(fcn, source = pt)
				typenode.name = 'return_type'
				fcn.add_child(typenode)
				target, new_type = self._get_target_type(typenode, child.the('param_type_spec'),
						defining_scope = typenode)
				# Add the target explicitly.
				typenode.add_child(target)
				if child.the('allow_indirection'):
					typenode.add_attribute('indirection', child.the('allow_indirection').leaf)
				elif child.the('ref_indirection'):
					typenode.add_attribute('indirection', ['*'])
			elif child.type == 'op_dcl':
				fcn.leaf = child.leaf
			elif child.type == 'parameter_dcls':
				fcn.add_children(self.parameter_dcls(fcn, child))
			elif child.type == 'raises_expr':
				raises = Node(fcn, name='raises', source = child)
				leaflist = []
				target_list = []
				for node in child.the('scoped_name_list').children:
					if node.type == 'scoped_name':
						newname = self.scoped_name(node)
						leaflist.append(newname)
						target_list.append(getTypenode(newname, fcn))
				raises.leaf = leaflist
				raises.add_attribute('target_scope_list', target_list)
				fcn.add_child(raises)
			elif child.type == 'context_expr':
				context = Node(fcn, name='context', source = child)
				leaflist = []
				for node in child.the('string_literal_list').children:
					if node.type == 'string_literal':
						leaflist.append(node.leaf)
				context.leaf = leaflist
				fcn.add_child(context)
			else:
				fcn = UnknownNode(None, child, source = child)

		# Add it and include its symbol.
		self._dupes_check(fcn)
		
	def parameter_dcls(self, ast, pt):
		param_list = []
		decl_list_pt = pt.the('param_dcl_list')
		if decl_list_pt:
			dec_node = None
			param_node = None
			for child in decl_list_pt.children:
				if child.type == 'decorator':
					#print child
					dec_els = self.decorator_elements(ast, child)
					dec_node = Node(ast, 'decorator', dec_els, source = child)
					
					#print dec_node
				if child.type == 'param_dcl':
					param_node = self.param_dcl(ast, child)
					if dec_node != None:
						param_node.add_child(dec_node)
						dec_node = None
					param_list.append(param_node)
		return param_list
	
	def param_dcl(self, ast, pt):
		"""
		Param decls look like constant declarations: refer to "const_dcl".
		1.	parameter blah  (3)
			+- meta_type = ['param']
		    +- direction = ['in']
			+- indirection = ['*']
			+- target = [<Node object type:unsigned int>]
		"""
		param_ast = Node(ast, 'parameter', None, source = pt)

		# Add direction
		param_ast.add_attribute('direction', pt.leaf)

		for child in pt.children:
			if child.type == 'param_type_spec':
				#param_ast.add_attribute('target',self.param_type_spec(ast, child))
				target_type, is_new_type = self._get_target_type(param_ast, child,
						defining_scope = param_ast)
				param_ast.add_child(target_type)
			elif child.type == 'allow_indirection':
				indirection_list = [child.leaf]
				for indirection in child['allow_indirection']:
					indirection_list.append(indirection.leaf)
				param_ast.add_attribute('indirection', indirection_list)
			elif child.type == 'simple_declarator':
				param_ast.leaf = child.leaf
			else:
				param_ast.add_child(UnkownNode(None, child, source = pt))

		# Check we're not passing raw void parameters.
		if not self.is_valid_parameter_spec(param_ast):
			raise InvalidTypeUsageError(target_type.the('type').leaf, child)
		return param_ast
			
	def is_valid_parameter_spec(self, ast):
		target_type = ast.the('target').the('type')
		if infogripper.is_void(target_type) and not self._has_indirection(ast):
			return False
		
		return True
	
	def _has_indirection(self, ast):
		"""
		Return True if ast contains some indirection (ie, it's a pointer type),
		False otherwise.

		Works on parameter ASTs and type ASTs
		"""
		if ast.has_attribute('indirection') and ast.get_attribute('indirection'):
			return True
		elif ast.has_child('target'):
			return self._has_indirection(ast.the('target').the('type'))
		else:
			return False
		
	def attr_declarator(self, ast, pt):
		decl = Node(ast, 'declarator', None, source = pt)
		decl.leaf = []
		for child in pt.children:
			if child.type == 'simple_declarator':
				decl.leaf.append(child.leaf)
			elif child.type == 'attr_raises_expr':
				decl.add_child(self.attr_raises_expr(decl, child))
			else:
				decl.add_child(UnkownNode(None, child, source = pt))
		return decl
		
		
	def attr_raises_expr(self, pt):
		raisesnode = Node(None, None, source = pt)
		for child in pt.children:
			if child.type == 'get_excep_expr':
				raisesnode.leaf = 'getraises'
				exception_list = self.exception_list(child.the('exception_list'))
				raisesnode.add_attribute('exception_list', exception_list)
			elif child.type == 'set_excep_expr':
				raisesnode.leaf = 'setraises'
				exception_list = self.exception_list(child.the('exception_list'))
				raisesnode.add_attribute('exception_list', exception_list)
			else:
				raisesnode = UnknownNode(None, child, source = pt)
		return raisesnode
		
	def exception_list(self, pt):
		ex_list = []
		for child in pt.children:
			if child.type == 'scoped_name':
				ex_list.append(self.scoped_name(child))
		return ex_list
	
		
	def struct_type(self, ast, pt, defining_scope = None):
		type_node = TypeNode(ast, source = pt)
		type_node.leaf = pt.the('identifier').leaf
		type_node.add_attribute('meta_type', 'struct')
		membernode = self.member_list(type_node, pt.the('member_list'))
		type_node.add_children(membernode)
		if defining_scope:
			defining_scope.add_child(type_node)
		return type_node

	def enum_type(self, ast, pt, defining_scope):
		type_node = TypeNode(ast, source = pt)
		type_node.add_attribute('meta_type', 'enum')

		enum_list = []
		for child in pt.children:
			if child.type == 'identifier':
				type_node.leaf = child.leaf
			elif child.type == 'enumerator_list':
				for enum in child['enumerator']:
					enum_list.append(enum.leaf)
				type_node.add_attribute('enumeration', enum_list)
	
		# All the enum internal names get added to the outer scope (Do they?!)
		if defining_scope:
			# Add the enum itself, too.
			defining_scope.add_child(type_node)

			for count, child_pt in enumerate(pt.the('enumerator_list')['enumerator']):
				enum_item_ast = TypeNode(defining_scope, leaf = child_pt.leaf, source = child)
				defining_scope.add_child(enum_item_ast)
				enum_item_ast.add_attribute('meta_type', 'enum_member')
				enum_item_ast.add_attribute('value', count)
				self._dupes_check(enum_item_ast)

				enum_item_target = Node(enum_item_ast, 'target')
				enum_item_target.add_child(type_node)
				enum_item_ast.add_child(enum_item_target)


		return type_node

	def union_type(self,ast, pt, defining_scope = None):
		union_ast = TypeNode(ast, source = pt)
		union_ast.add_attribute('meta_type', 'union')
		# Discriminated unions consist of three parts: the union name, the
		# discriminator (a switch), and the body (a switch body).

		# Union name:
		union_ast.leaf = pt.the('identifier').leaf

		# Type spec:
		type_node, is_new_type = self._get_target_type(ast, pt.the('switch_type_spec'),
				wrapper = 'discriminator', defining_scope = union_ast)

		if not self.is_valid_discriminator(type_node):
			raise InvalidDiscriminatorError(union_ast)

		# Add the type wrapped as "disciminator"
		union_ast.add_child(type_node)

		# Switch body:
		switch_members = self.case_stmt_list(union_ast, pt.the('switch_body').the('case_stmt_list'))
		union_ast.add_child(switch_members)

		if defining_scope:
			defining_scope.add_child(union_ast)

		return union_ast

	def case_stmt_list(self, ast, pt):
		"""
		switch
		 case
		  expression
		  [expression...]
		  declarator
		 [case...]
		"""
		# Find the appropriate scope for resolving scoped names, if any are present in
		# the case statement list. Switches on enums are constrained to the scope of
		# the enum (I think).
		
		switch_ast = Node(ast, 'switch', source = pt)
		for case_pt in pt.children:
			case_ast = Node(switch_ast, 'case', source = case_pt)
			switch_ast.add_child(case_ast)

			for choice_pt in case_pt['const_exp']:
				expr_ast = self.getExpression(case_ast, choice_pt)
				case_ast.add_child(expr_ast)

			typeref, new_type = self.type_spec(case_ast, case_pt.the('element_spec').the('type_spec'))
			if new_type:
				case_ast.add_child(typeref)
			
			decl = case_pt.the('element_spec').the('declarator')

			inst_list = self.create_instance_list(case_ast, typeref, [decl])
			case_ast.add_children(inst_list)

		return switch_ast
	
	def is_valid_discriminator(self, ast):
		""" Check that a union discriminator is valid.
		Valid types: integers, bool, char, octet, enum
		"""
		if ast.name == 'discriminator':
			ast = ast.children[0]

		return infogripper.is_corba_integer(ast) or infogripper.is_bool(ast)\
				or infogripper.is_char(ast) or infogripper.is_octet(ast)\
				or infogripper.is_enum(ast)
 
	def member_list(self, ast, pt):
		members = Node(ast, None, source = pt)
		members.type = 'members'
		for child in pt.children:
			if child.type == 'member':
				typeref, new_type = self.type_spec(members, child.the('type_spec'))
				if new_type:
					if isinstance(typeref, str):
						pass
					else:
						members.add_child(typeref)
				members.add_children(self.create_instance_list(members, typeref, child.the('declarators').children))
			else:
				print 'memberlist_unknown' + child.type
		return [members]

	def scoped_name(self, pt):
		"""
		Expect something like this:
		scoped_name <ignored>
		 scope_operator ::
		 identifier <scope_elem>
		 [scope_operator]
		 [identifier]
		 [...]
		"""
		return ''.join([child.leaf for child in pt.children])

	def scoped_name_type(self, ast, pt):
		scoped_name = self.scoped_name(pt)
		target_type = infogripper.getTypenode(scoped_name, ast)
		if not target_type:
			raise UnknownTypeError(failed_name = scoped_name, node = pt)
		return target_type
	
	def type_spec(self, ast, pt):
		typeref = None
		new_type = True
		if pt.children[0].type == 'simple_type_spec':
			typeref, new_type = self.simple_type_spec(ast, pt.children[0])
		else: #constr_type_spec
			typeref = self.constr_type_spec(ast, pt.children[0], defining_scope = ast)
		return (typeref, new_type)
		
	def simple_type_spec(self, ast, pt):
		typeref = None
		new_type = False
		if pt.children[0].type == 'base_type_spec':
			typeref = self.base_type_spec(ast, pt.children[0])
		elif pt.children[0].type == 'template_type_spec':
			typeref = self.template_type_spec(ast, pt.children[0])
			new_type = True
		else: #scoped_name
			typeref = self.scoped_name_type(ast, pt.children[0])
			if typeref is None:
				#print ast, '  ', ast.leaf
				raise SemanticError("Unknown type '%s'" % (pt.leaf))
		return (typeref, new_type)
		
	def base_type_spec(self, ast, pt):
		typename = None
		if pt.children[0].type == 'floating_pt_type':
			typename = self.floating_pt_type(ast, pt.children[0])
		elif pt.children[0].type == 'integer_type':
			typename = self.integer_type(ast, pt.children[0])
		else:
			typename = pt.children[0].leaf
		targetnode = infogripper.getBasicTypenode(typename, ast)
		if targetnode == None:
			print 'name = %s not found!' %(typename)
		return targetnode

	def integer_type(self, ast, pt):
		return ' '.join([child.leaf for child in pt.children])

	def floating_pt_type(self,ast, pt):
		name = pt.leaf
		if pt.children != []:
			name += ' ' + pt.children[0].leaf
		return name

	def template_type_spec(self, ast, pt):
		typenode = TypeNode(ast, None, source = pt)
		if pt.children[0].type == 'sequence_type':
			typenode = self.sequence_type(ast, pt.children[0])
		elif pt.children[0].type == 'string_type':
			typenode = self.string_type(ast, pt.children[0])
		elif pt.children[0].type == 'wide_string_type':
			typenode = self.wide_string_type(ast, pt.children[0])
		elif pt.children[0].type == 'fixed_pt_type':
			typenode.add_attribute('meta_type', 'fixed_pt')
			pos_const_node = pt.the('fixed_pt_type')['positive_int_const'][0]
			expr = self.getExpression(ast, pos_const_node.the('const_exp'))
			pre_size = expr.leaf
	
			pos_const_node = pt.the('fixed_pt_type')['positive_int_const'][1]
			const_expr = self.getExpression(ast, pos_const_node.the('const_exp'))
			post_size = const_expr.leaf
		
			typenode.add_attribute('shape', (pre_size, post_size))
		else:
			typenode = UnknownNode(None, pt.children[0], source = pt)
		return typenode
		
	def sequence_type(self, ast, pt):
		typenode = TypeNode(ast, None, source = pt)
		typenode.add_attribute('meta_type', 'sequence')
		
		target, is_new_type = self._get_target_type(ast, pt.the('simple_type_spec'), defining_scope = typenode)
		typenode.add_child(target)
		
		pos_const_node = pt.the('opt_pos_int')
		if pos_const_node != None:
			expr = self.getExpression(ast, pos_const_node.the('positive_int_const').the('const_exp'))
			typenode.add_attribute('max_length', expr.leaf)
		return typenode
		
	def string_type(self, ast, pt, wstring = False):
		typenode = Node(ast, 'type', source = pt)
		if wstring:
			target = Node(typenode, 'customised', [infogripper.getTypenode('wstring', ast)])
		else:
			target = Node(typenode, 'customised', [infogripper.getTypenode('string', ast)])
		typenode.add_child(target)
		pos_const_node = pt.the('positive_int_const')
		if pos_const_node != None:
			expr = self.getExpression(ast, pos_const_node.the('const_exp'))
			typenode.add_attribute('max_length', expr.leaf)
		# This is a new, anonymous type.
		return typenode
		
	def wide_string_type(self, ast, pt):
		stringtype = self.string_type(ast, pt, True)
		return stringtype
	
	def constr_type_spec(self, ast, pt, defining_scope = None):
		while pt.name in ('constr_type_spec'):
			assert len(pt.children) == 1
			pt = pt.children[0]

		if pt.name == 'struct_type':
			constr_type_node = self.struct_type(ast, pt, defining_scope = defining_scope)
		elif pt.name == 'union_type':
			constr_type_node = self.union_type(ast, pt, defining_scope = defining_scope)
		elif pt.name == 'enum_type':
			constr_type_node = self.enum_type(ast, pt, defining_scope = defining_scope)
		else:
			pt.print_tree()
			raise Exception("constr_type_spec: unexpected node type '%s'" % (pt.name))
		return constr_type_node
	
	def getDeclarator(self, pt):
		decl_str = ''
		if pt.children[0].type == 'simple_declarator':
			decl_str = pt.children[0].leaf
		else: #complex_declarator
			array_dcl = pt.the('complex_declarator').the('array_declarator')
			leaf_str = array_dcl.leaf
			for child in array_dcl.children:
				exp_node = child.the('positive_int_const').the('const_exp')
				expr = self.getExpression(ast, exp_node)
				array_dcl = []
				length_str = ''
				if expr.result is not None:
					array_dcl.append(res)
					length_str += '+(' + str(expr.result) + ')'
				else:
					array_dcl.append(expr.leaf)
					length_str += '+(' + expr.leaf+ ')'
			decl_str = [[leaf_str, length_str, array_dcl]]
		return decl_str
		
	def create_instance_list(self, ast, typeref, declarators):
		instance_list = []
		for child in declarators:
			instance = Node(ast, 'type_instance', None, source = child)
			
			if child.children[0].type == 'simple_declarator':
				instance.leaf = child.children[0].leaf
				instance.add_child(typeref)
			else: #complex_declarator
				newtype = TypeNode(ast, source = pt)
				newtype.add_attribute('meta_type','array')
				
				array_dcl = child.the('complex_declarator').the('array_declarator')
				array_dim = []
				for dimension in array_dcl.children:
					exp_node = dimension.the('positive_int_const').the('const_exp')
					expr = self.getExpression(ast, exp_node)
					array_dim.append(expr.leaf)
					'''
					if res != None:
						array_dcl.append(str(res))
					else:
						array_dcl.append(exp_str)
					'''
				newtype.add_attribute('shape',array_dim)
				newtype.add_child(typeref)
				
				instance.add_child(newtype)
				instance.leaf = array_dcl.leaf
			instance_list.append(instance)
		return instance_list

	def type_from_declarator(self, parent_ast, declarator):
		"""
		Construct a partial type. Only used for typdefs currently.
		"""
		typenode = TypeNode(parent_ast, None, source = declarator)
		# Don't know the meta type here

		if declarator.my_children_are('simple_declarator'):
			typenode.leaf = declarator.the('simple_declarator').leaf
			#typenode.add_attribute('target_type', typeref)
		else: #complex_declarator
			newtype = TypeNode(parent_ast, source = declarator)
			newtype.add_attribute('meta_type','array')
			
			array_dcl = declarator.the('complex_declarator').the('array_declarator')
			array_dim = []
			shape = Node(newtype, 'shape', source = declarator)
			newtype.add_child(shape)

			for dimension in array_dcl.children:
				exp_node = dimension.the('positive_int_const').the('const_exp')
				expr = self.getExpression(shape, exp_node)
				if expr.attribute('value') < 0:
					raise DimensionOutOfRangeError(dimension)
				shape.add_child(expr)
			
			typenode.leaf = array_dcl.leaf
			typenode.add_child(newtype)
			
		assert typenode.leaf != None
		return typenode
			
def gen(pt, filename, ast):
	astgen = ASTGen(ast)
	output = astgen.convert(ast,filename, pt)
	return output
