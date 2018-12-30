import sys

from parser import parse

def get_func_name(pt_in):
	pass

class H4Object(object):
	def __init__(self):
		self.h4type = None

class H4Function(H4Object):
	def __init__(self, func_name, func_params, func_expr):
		H4Object.__init__(self)
		self.name = func_name
		self.params = func_params

	def get_func_name(pt_in):
		" Normalises a function name "
		name = []
		skip_next = False
		for element in pt_in.children:
			if element.name == ':':
				name.append(':')
				skip_next = True
			elif skip_next:
				skip_next = False
			else:
				name.append(element.name)
		return ''.join(name)
	get_func_name = staticmethod(get_func_name)

	def get_func_params(pt_in):
		" Returns a list of parameters from a function name "
		params = []
		skip_next = True
		for element in pt_in.children:
			if element.name == ':':
				skip_next = False
			elif skip_next:
				pass
			else:
				params.append(element.name)
				skip_next = True
		return params
	get_func_params = staticmethod(get_func_params)

	def from_pt(cls, pt_in):
		assert pt_in.my_children_are('func_name', 'expression')
		
		# Get func name
		func_name = cls.get_func_name(pt_in.the('func_name'))
		func_params = cls.get_func_params(pt_in.the('func_name'))
		print func_name, func_params

		obj = cls(func_name, None, None)
		pt_in.print_tree()
		return obj
	from_pt = classmethod(from_pt)
	

class H4(H4Object):
	"""
	A single scope.
	"""
	def __init__(self):
		H4Object.__init__(self)
		self.names = {}
	
	def pt_function(self, pt):
		func = H4Function.from_pt(pt)

class H4Builder(object):
	def __init__(self, pt):
		self.pt = pt
		# A list of all scopes
		self.scope = H4()
	
	def go(self):
		assert self.pt.name == 'declarations'
		for pt in self.pt.children:
			self.declaration(pt)
	
	def defer(self, pt_in):
		""" For nodes with only one child - call appropriate handler. """
		pt = pt_in.child()
		handler = getattr(self, pt.name)
		handler(pt)
		
	def declaration(self, pt_in):
		self.defer(pt_in)
	
	def module_decl(self, pt_in):
		module_name = pt_in.children[0]
		declarations = pt_in.children[1]
		builder = H4Builder(declarations)
		builder.go()
	
	def func_decl(self, pt_in):
		self.scope.pt_function(pt_in)
	
	# Convenience methods
	def from_file(cls, filename):
		pt = parse(file(filename))
		obj = cls(pt)
		return obj.go()
	from_file = classmethod(from_file)

def test():
	filename = sys.argv[1]
	h4obj = H4Builder.from_file(filename)

if __name__ == '__main__':
	test()
