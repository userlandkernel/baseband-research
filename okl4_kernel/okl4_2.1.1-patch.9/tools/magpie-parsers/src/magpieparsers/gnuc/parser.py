import sys, types
from cStringIO import StringIO

from magpieparsers.antlr import antlr
sys.modules['antlr'] = antlr

from StdCLexer import Lexer as CLexerBase
from StdCParser import Parser as CParserBase

from magpieparsers.parser_common import Node, UnfinishedNode
from magpieparsers.cpreprocessor.preprocess import cpp_file

class AntlrASTFactory(object):
	def create(self, token):
		if isinstance(token, int):
			return Node(None, token)
		elif isinstance(token, Node):
			# it's one of ours...
			return token
		else:
			node_name = token.getText()
			return Node(None, node_name)
	
	def dupList(self, node):
		assert isinstance(node, Node)
		return node.copy()

class Parser(CParserBase):
	def __init__(self, *args, **kwargs):
		CParserBase.__init__(self, *args, **kwargs)
		# FIXME: dodgy?
		self.astFactory = AntlrASTFactory()

	def addASTChild(self, currentAST, child):
		if child is None:
			# don't bother
			return
		if currentAST.root is None:
			root_name = sys._getframe(1).f_code.co_name
			currentAST.root = Node(None, root_name)
		currentAST.root.add_child(child)
		#print "called from", sys._getframe(1).f_code.co_name
		#print 'add ast child', currentAST, child
	
	def makeASTRoot(self,currentAST,root) :
		raise NotImplementedException()
	
	def symtable_add(self, name, *nodes):
		node = Node(None, 'unknown', children = nodes)
		self.symbolTable.add(name, node)

class Lexer(CLexerBase):
	pass

def _parse_data(filename, data):
	print data
	inputhandle = StringIO(data)
	#line_handler = LineDirectiveHandler()
	lexer = Lexer(inputhandle)#, line_directive_handler = line_handler)
	parser = Parser(lexer)
	parser.setFilename(filename)
	#line_handler.set_parser(parser) # Messy
	#line_handler.set_lexer(lexer)

	parser.translationUnit()

	return parser.returnAST

def parse(filename, cpp_options = None, use_cpp = True):
	if use_cpp:
		if cpp_options is None:
			cpp_options = {}

		data = cpp_file(filename, **cpp_options)
		pt = _parse_data(filename, data)
	else:
		data = file(filename).read()
		pt = _parse_data(filename, data)
	
	return pt

def main(filename):
	result = parse(sys.argv[1])
	result.print_tree()

if __name__ == '__main__':
	main (sys.argv[1])

