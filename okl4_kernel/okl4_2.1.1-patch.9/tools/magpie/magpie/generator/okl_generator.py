from magpie.generator import v4generator

class Function(v4generator.V4Function):
	BYREF_STYLE = 'nicta_n2'

class Interface(v4generator.V4Interface):
	def _my_function_class(self):
		return Function

class Generator(v4generator.V4Generator):
	def _my_interface_class(self):
		return Interface

