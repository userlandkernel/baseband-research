import sys, types
from cStringIO import StringIO

from magpieparsers.antlr import antlr
sys.modules['antlr'] = antlr

import IDLLexer
import IDLParser
import extendAST

from magpieparsers.parser_common import Node, UnfinishedNode
from magpieparsers.cpreprocessor.preprocess import cpp_file
from magpieparsers import error

from astgen import gen as ast_gen
#from magpietypes.builtin import create_basictype_ast

import antlr

def debug_warn(text):
	#print text
	pass

def debug_info(text):
	#print text
	pass

class Parser(IDLParser.Parser):
	""" This Parser class replicates the stuff in Support.cpp. Really, it shouldn't be
	an overloading class, because knows all about the inner workings of its base. But unlike
	C++, Python doesn't let you separate interface and implementation, so I do
	this this way to avoid clogging up the grammar file.
	"""
	def __init__(self, *args, **kwargs):
		self.finished = False
		self.nodeinfo = {'depth': 0, 'nodes': []}
		lexer = args[0]
		lexer.set_new_filename_handler(self.set_file)
		IDLParser.Parser.__init__(self, *args, **kwargs)
	
	def addASTChild(self,currentAST, child):
		if not child:
			return
			
		rootnode = Node(None,sys._getframe(1).f_code.co_name,
				source_line = self.LT(1).getLine(),
				source_file = self.getFilename())
		if child.node:
			if not currentAST.root:
				rootnode.children = [child.node]
			else:
				rootnode = child.node # Node(sys._getframe(1).f_code.co_name, children=[child.node])

		child.node = rootnode
		child.node.leaf = child.getText()

		if child.node is None:
			print child
		if not currentAST.root:
			currentAST.root = child
		elif not currentAST.child:
			currentAST.root.setFirstChild(child)
		else:
			currentAST.root.node.add_child(child.node)
			currentAST.child.setNextSibling(child)
			currentAST.child = child
			currentAST.advanceChildToEnd()
	
	def set_file(self, filename):
		self.setFilename(filename)

	def reportError(self, x):
		raise error.from_antlr(x, self.getFilename())

	# IDL-specific error handling
	def error_in_definition(self, ex):
		if ex.token.getText() == 'interface':
			# Parsed "interface" but couldn't get beyond it.
			# Complain about the token.
			raise error.from_antlr_token(self.LT(2), self.getFilename())
		else:
			raise
	
	def error_in_specification(self, ex):
		if ex.token.isEOF():
			# Unexpected end of file
			raise error.from_antlr_token(ex.token, filename = self.getFilename(),
					text = "Unexpected end of file")
		else:
			raise
	
class Lexer(IDLLexer.Lexer):
	def set_new_filename_handler(self, handler):
		self._new_filename_handler = handler

	def preprocessor_directive(self, txt):
		# Handle a preprocessor directive.
		# They look like <line number> <filename> <optional flags>
		# ... grab line number
		line_number, txt = txt.getText().strip().split(' ', 1)
		# ... Find options, if any exist.
		opt_start = len(txt)
		while txt[opt_start-1] in '1234 ':
			opt_start -= 1
		options = txt[opt_start:].strip()
		# ... Finally, grab the filename
		txt = txt[:opt_start].strip()
		if txt[0] == '"':
			txt = txt[1:]
		if txt[-1] == '"':
			txt = txt[:-1]

		self.setLine(int(line_number) - 1)
		self.setFilename(txt)

		# NB the parser has to know the filename - the lexer doesn't pass its
		# knowledge along.
		self._new_filename_handler(txt)

def parse_data_to_pt(filename, data):
	inputhandle = StringIO(data)
	lexer = Lexer(inputhandle)
	parser = Parser(lexer)
	parser.setFilename(filename) # Just in case cpp doesn't tell us?
	try:
		parser.specification()
	except antlr.ANTLRException, e:
		raise error.from_antlr(e, filename)
	result = parser.returnAST.node
	return result
	
def parse_to_pt(filename, cpp_options = None):
	if cpp_options is None:
		cpp_options = {}

	data = cpp_file(filename, **cpp_options)
	
	return parse_data_to_pt(filename, data)

def parse_to_ast(pt, filename, ast):
	ast = ast_gen(pt, filename, ast)
	return ast

def parse(filename, basicast):
	pt = parse_to_pt(filename)
	ast = parse_to_ast(pt, filename, basicast)
	return ast
	
def parse_data(filename, data, basicast):
	pt = parse_data_to_pt(filename, data)
	ast = parse_to_ast(pt, filename, basicast)
	return ast

def test(inputhandle):
	lexer = Lexer(inputhandle)
	parser = Parser(lexer)
	ast = parser.specification()
	#print '\n\n\n\n', ast
	ast.node.print_tree()
	print '\n\n\n\n', ast
	
if __name__ == '__main__':
	handle = file(sys.argv[1])
	test(handle)
