import sys
from cStringIO import StringIO

from magpieparsers.parser_common import Node
from astgen import gen as ast_gen
import antlr

import MIGParser
import MIGLexer

class AntlrASTFactory(object):
	def create(self, token):
		if isinstance(token, Node):
			# it's one of ours...
			return token
		else:
			node_name = token.getText()
			return Node(None, node_name)

class Parser(MIGParser.Parser):
	def __init__(self, *args, **kwargs):
		self.finished = False
		self.nodeinfo = {'depth': 0, 'nodes': []}
		MIGParser.Parser.__init__(self, *args, **kwargs)
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

def parse_to_pt(data):
	inputhandle = StringIO(data)
	lexer = MIGLexer.Lexer(inputhandle)
	parser = Parser(lexer)
	#print parser
	parser.statements()
	#print parser
	return parser.returnAST

def parse(filename, data):
  pt = parse_to_pt(data)
  ast = ast_gen(pt)
  return ast
