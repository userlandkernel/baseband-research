import sys
from cStringIO import StringIO

from parser_common import Node
import antlr

import H4Parser
import H4Lexer

class AntlrASTFactory(object):
	def create(self, token):
		if isinstance(token, Node):
			# it's one of ours...
			return token
		else:
			node_name = token.getText()
			return Node(None, node_name)

class Parser(H4Parser.Parser):
	def __init__(self, *args, **kwargs):
		#self.finished = False
		#self.nodeinfo = {'depth': 0, 'nodes': []}
		H4Parser.Parser.__init__(self, *args, **kwargs)
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
	
	def makeASTRoot(self,currentAST,root):
		raise NotImplementedException()

def parse(handle):
	lexer = H4Lexer.Lexer(handle)
	parser = Parser(lexer)
	parser.declarations()
	return parser.returnAST

def test():
	filename = sys.argv[1]
	handle = file(filename)
	pt = parse(handle)
	pt.print_tree()

if __name__ == '__main__':
	test()

