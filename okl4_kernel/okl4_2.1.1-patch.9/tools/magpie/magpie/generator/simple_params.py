from magpieparsers.types.infogripper import word_size, size

class SimpleParam(object):
	"""
	A SimpleParam is a list of parameter attributes. A list of them is passed to
	the template so it doesn't have to know about the structure of the AST.
	"""
	def __init__(self):
		pass
	
	def init_fromdict(self, attributes_dict):
		self.attributes_dict = attributes_dict
	
	def init_fromdecl(self, decl_ast, byreference):
		self.attributes_dict = self.create(decl_ast, byreference)

	def init_asreturn(self, function_decl_ast):
		# Special case: function return type counts as special parameter with
		# direction 'return'. 
		#<print op_return_type
		the_type = function_decl_ast.maybe_walk_to('return_type', 'target', 'type')
		if the_type:
			type_name = the_type.leaf
		else:
			type_name = None
		self.init_fromdict({'indirection': '',
				'c_impl_indirection': '',
				'name': '__return',
				'type': the_type,
				'typename': type_name,
				'direction': 'return',
				'ast': function_decl_ast.the('return_type'),
				'flags':'',
		})
	
	def __repr__(self):
		return '<SimpleParam: %s%s(%s), %sc>' % (self['indirection'], self['name'], self['typename'], self['c_impl_indirection'])
	
	def __getitem__(self, key):
		try:
			return self.attributes_dict[key]
		except:
			print self.attributes_dict
			raise
	
	def __setitem__(self, key, value):
		self.attributes_dict[key] = value
	
	def get(self, key, default):
		return self.attributes_dict.get(key, default)
	
	def get_c_base_typename(self):
		MAP = {'smallstring': 'char *'}
		default = self['typename']
		return MAP.get(self['typename'], default)
	c_base_typename = property(get_c_base_typename)

	def get_c_typename(self):
		typename = self.c_base_typename
		if self['indirection'] or self['c_impl_indirection']:
			typename += ' %s%s' % (self['indirection'],
				self['c_impl_indirection'])

		return typename
	c_typename = property(get_c_typename)

	def create(self, parameter_decl, by_reference):
		wordsize = word_size(parameter_decl)
		newparam = {}
		param_size = size(parameter_decl.the('target'))
		newparam['indirection'] = parameter_decl.attribute('indirection', '')
		assert newparam['indirection'] is not None
		if parameter_decl.has_attribute('direction'):
			newparam['direction'] = parameter_decl.attribute('direction')
		elif parameter_decl.name == 'return_type':
			# FIXME: Kinda hackish
			newparam['direction'] = 'return'
		else:
			assert False #direction is required!!

		assert newparam['direction'] in ('in', 'out', 'inout', 'return')
		newparam['ast'] = parameter_decl
		if parameter_decl.has_attribute('display_name'):
			newparam['typename'] = parameter_decl.get_single_attribute('display_name')
		else:
			newparam['typename'] = parameter_decl.the('target').the('type').leaf
		newparam['name'] = parameter_decl.leaf
		newparam['type'] = parameter_decl.the('target').the('type')
		if by_reference:
			newparam['c_impl_indirection'] = '*'
		else:
			newparam['c_impl_indirection'] = ''
		return newparam

