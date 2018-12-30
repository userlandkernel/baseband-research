class ASTInfo(object):
	"""
	This is where we put all the convenience functions to walk the AST.
	This class is just for the namespace and instances do not store state
	apart from the AST itself.
	"""
	def __init__(self, ast):
		#print 'adding ast = ', ast
		self.ast = ast
		
	def get_filename(self):
		return self.ast.leaf
	
	def get_definitions(self, childname):
		#print 'trying to find %s in ast = %s' %(childname, self.ast['definition'])
		for defn in self.ast[childname]:
			yield defn

	def get_ast_definitions(self, ast, childname):
		raise NotImplementedError()
		def_list = []
		for child in ast.children:
			if child.type == 'definition':
				result = child.the(childname)
				def_list.append((child, result))
			else:
				def_list.extend(self.get_ast_definitions(child, childname))
		return def_list
	
	def get_name(self, ast):
		NAME_MODIFIER = ('pointer', 'array')
		
		name = ast.leaf
		modifier, target_node = name_modifier(ast)
		while modifier in NAME_MODIFIER:
			
			if modifier == 'pointer':
				name = '(*'+name+')'
			elif modifier == 'array':
				array_dim_str = '[' + ']['.join(target_node.the('info').get_attribute('shape')) + ']'
				name = name + array_dim
			modifier, target_node = name_modifier(target_node)
		return name

	def name_modifier(self, ast):
		if ast.the('info') == None:
			return None, None
		elif ast.the('info').has_attribute('target_type'):
			target_node = ast.the('info').get_attribute('target_type')[0]
			modifier = target_node.get_attribute('meta_type')[0]
			return modifier, target_node
		else:
			return None, None
		

	def get_target_type(self, ast):
		target_node = ast.the('info').get_attribute('target')[0]
		if target_node.leaf != None:
			return targte_node.leaf
		else:#anonymous type ... construct inline
			return target_node
		
		
	def get_instance_type(self, ast):
		target_node = ast.the('info').get_attribute('target')[0]
		if target_node.leaf != None:
			return targte_node.leaf
		else:#anonymous type ... construct inline
			return target_node
		


class Generator(object):
	"""
	Walks the IDL AST and calls relevant output functions.

	Writing a generator necessarily means deciding on the basic structure of
	the output.
	"""
	def __init__(self, astinfo):
		self.astinfo = astinfo
