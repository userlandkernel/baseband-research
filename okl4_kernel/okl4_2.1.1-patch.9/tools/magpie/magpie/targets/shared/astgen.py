"""
Class to parse an IDL file and return an AST. Can be used as a library.
"""
from magpieparsers.types.builtin import create_basictype_ast
from magpie.targets.shared.inputparser import construct as inputparser_construct

class CLanguageAnnotator(object):
	# Annotate an AST with appropriate names for C output.
	# FIXME: This should go elsewhere...
	def go(self, ast):
		self.constants(ast)

	def constants(self, ast):
		"""
		Add a "c_name" for all constants in interfaces.
		"""
		interface_finder = ['interface']
		constant_finder = ['type_instance', {'meta_type': 'const'}]

		for interface_ast in ast.search(interface_finder):
			for constant_ast in interface_ast.search(constant_finder):
				c_name = '%s_%s' % (interface_ast.leaf, constant_ast.leaf)
				constant_ast.add_attribute('c_name', c_name)

def language_annotate(ast):
	annotator = CLanguageAnnotator()
	annotator.go(ast)
	
class ASTGen(object):
	def __init__(self, hardware_arch, generator_type, cpp = None, idl_cpp = None,
			cpp_options = None, cpp_include_dirs = None, cpp_defines = None):

		self.basicast = create_basictype_ast(hardware_arch, generator_type)
		self.inputparser = inputparser_construct(cpp = cpp, idl_cpp = idl_cpp,
				cpp_options = cpp_options, include_dirs = cpp_include_dirs,
				defines = cpp_defines)
	
	def __call__(self, pathname):
		ast = self.inputparser.produce_ast(pathname, 'CORBA', self.basicast)
		annotator = CLanguageAnnotator()
		annotator.go(ast)
		return ast

