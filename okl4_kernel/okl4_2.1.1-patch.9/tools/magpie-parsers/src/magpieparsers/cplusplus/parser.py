import sys, types
from cStringIO import StringIO

from magpieparsers.antlr import antlr
sys.modules['antlr'] = antlr

from CPPLexer import Lexer
from CPPSymbol import *
import CPPParser

try:
	# Attempt to import the C++ native version.
	from magpieparsers import cppparse
except ImportError:
	cppparse = None

from enum_types import *
from magpieparsers.parser_common import Node, UnfinishedNode
from magpieparsers.cpreprocessor.preprocess import cpp_file
from magpieparsers.types.builtin import create_basictype_ast
from magpieparsers.types.map import AST_TO_CPP

from astgen import gen as ast_gen

from magpieparsers import error

# FIXME: This is sort of retarded, but the generated code doesn't import exceptions before
# using them. So here we manually inject them into its namespace.
import antlr
CPPParser.SemanticException = antlr.SemanticException
# FIXME: End of manual namespace update.

def debug_warn(text):
	#print text
	pass

def debug_info(text):
	#print text
	pass

class Parser(CPPParser.Parser):
	""" This Parser class replicates the stuff in Support.cpp. Really, it shouldn't be
	an overloading class, because knows all about the inner workings of its base. But unlike
	C++, Python doesn't let you separate interface and implementation, so I do
	this this way to avoid clogging up the grammar file.
	"""
	def __init__(self, *args, **kwargs):
		self.finished = False
		self.nodeinfo = {'depth': 0, 'nodes': []}
		CPPParser.Parser.__init__(self, *args, **kwargs)
	
	def node(self, name, children = None, leaf = None, result = None, token = None):
		if not token:
			token = sys._getframe(1).f_locals.get('wt')

		if token:
			t_line = token.getLine()
		else:
			t_line = None

		t_file = self.getFilename()

		return Node(None, name, children, leaf, result = result,
				source_line = t_line, source_file = t_file)
		
	def autonode(self, children = None, leaf = None, result = None):
		name = sys._getframe(1).f_code.co_name
		token = sys._getframe(1).f_locals.get('wt')
		return self.node(name, children, leaf, result, token = token)
	
	def exprnode(self, children = None, leaf = None, result = None):
		token = sys._getframe(1).f_locals.get('wt')
		return self.node("expression", children, leaf, result, token = token)
	
	def unfinishednode(self, name = None):
		if name is None:
			name = sys._getframe(1).f_code.co_name
		return UnfinishedNode(name)
	
	def blanknode(self):
		name = sys._getframe(1).f_code.co_name
		return self.node("blank", None, name)
	
	def exprnode_acc(self, node, child, leaf, result = None):
		token = sys._getframe(1).f_locals.get('wt')
		if node is None:
			name = sys._getframe(1).f_code.co_name
			node = self.node('expression_unfinished_%s' % (name), token = token)
		if leaf != node.leaf:
			node = self.node('expression', [node, child], leaf, token = token)
		else:
			node.add_child(child)
		if result is None:
			# Calculate result based on children.
			if len(node.children) > 1:
				val_list = ['(%s)' % (child.result) for child in node.children]
				operator = AST_TO_CPP.get(node.leaf, node.leaf)
				expr = operator.join(val_list)
				node.result = expr
		else:
			node.result = result
		return node

	def enterExternalScope(self):
		self.functionDefinition = 0
	
	def beginFunctionDefinition(self):
		self.functionDefinition = 1
	
	def declarationSpecifier(self, td, fd, storageclass, typequalifier,
			typespecifier, declspecifier):
		self._td = td
		self._fd = fd
		self._sc = storageclass
		self._ts = typespecifier
		self._ds = declspecifier
	
	def declaratorID(self, id_str, qualifieditem):
		""" Add a symbol from a declarator to the symbol table in the current scope. """
		# Type declaration
		if self._td or qualifieditem == qiType:
			if qualifieditem != qiType or not self._td:
				debug_warn("%d declaratorID warning qi %d, _td %d inconsistent for %s" % \
						(self.LT(1).getLine(), qualifieditem, _td, id_str))
			c = CPPSymbol(id_str, otTypedef)
			debug_info ("ADD TYPE DECL - %s" % (id_str))
			self.symbols.defineInScope(id_str, c, self.externalScope)
			# FIXME: Debugging here?
		# Function
		elif qualifieditem == qiFun:
			c = CPPSymbol(id_str, otFunction)
			self.symbols.define(id_str, c)
			debug_info ("ADD FUN DECL - %s" % (id_str))
		# Variable
		else:
			if qualifieditem != qiVar:
				debug_warn("%d declaratorID warning qi (%d) not qiVar (%d) for %s" % \
						(self.LT(1).getLine(), qualifieditem, qiVar, id_str))
			c = CPPSymbol(id_str, otVariable)
			self.symbols.define(id_str, c)
			debug_info ("ADD VAR DECL - %s" % (id_str))
	
	def qualifiedItemIsOneOf(self, qiFlags, lookahead_offset = 0):
		qi = self.qualifiedItemIs(lookahead_offset)
		if qi & qiFlags != 0:
			return True
		return False
	
	def qualifiedItemIs(self, lookahead_offset = 0):
		k = lookahead_offset + 1
		final_type_idx = 0
		scope_found = False

		#print "QII", k, self.LT(k)
		if self.LT(k).getType() == CPPParser.SCOPE:
			k+= 1
			scope_found = True
		while self.LT(k).getType() == CPPParser.ID and self.isTypeName(self.LT(k).getText()):
			if final_type_idx != 0 and self.LT(final_type_idx).getText() == self.LT(k).getText():
				if self.LT(k+1).getType == CPPParser.SCOPE:
					return qiInvalid
				else:
					return qiCtor

			final_type_idx = k
			k += 1

			# Skip over template qualifiers
			if self.LT(k).getType() == CPPParser.LESSTHAN:
				if not self.skipTemplateQualifiers(k):
					return qiInvalid
			if self.LT(k).getType == CPPParser.SCOPE:
				# Skip "::" and keep going
				k += 1
				scope_found = True
			else:
				# Last ID in the sequence was a type.
				if self.enclosingClass == self.LT(final_type_idx).getText():
					return qiCtor
				else:
					return qiType
		the_type = self.LT(k).getType()
		if the_type == CPPParser.ID:
			if self.LT(k+1).getType() == CPPParser.SCOPE:
				return qiInvalid
			if self.enclosingClass == self.LT(k).getText():
				return qiCtor
			else:
				if scope_found:
					return qiType
				else:
					return qiVar
		elif the_type == CPPParser.TILDE:
			if self.LT(k+1).getType() == CPPParser.ID \
					and self.isTypeName(self.LT(k+1).getText()) \
					and self.LT(k+2).getType() != CPPParser.SCOPE:
				return qiDtor
			else:
				return qiInvalid
		elif the_type == CPPParser.STAR:
			if final_type_idx == 0:
				return qiInvalid
			else:
				return qiPtrMember
		elif the_type == CPPParser.OPERATOR:
			return qiOperator
		else:
			return qiInvalid
	
	def scopedItem(self, k = 1):
		if self.LT(k).getType()==CPPParser.SCOPE \
				or (self.LT(k).getType()==CPPParser.ID and not self.finalQualifier(k)):
			return True
		else:
			return False

	def finalQualifier(self, k = 1):
		if self.LT(k).getType() == CPPParser.ID:
			if self.isTypeName(self.LT(k).getText()) and self.LT(k+1).getType == CPPParser.LESSTHAN:
				# starts with "T<". Skip <...>
				k += 1
				self.skipTemplateQualifiers(k)
			else:
				# skip ID
				k += 1
			return self.LT(k).getType() != CPPParser.SCOPE
		else:
			# Not an ID
			return False
			
	def isTypeName(self, s):
		#print "IS TYPE NAME", [s],
		symbol = self.symbols.lookup(s)
		if symbol is None:
			#print "NO"
			return False
		if symbol.getType() in (otTypedef, otEnum, otClass, otStruct, otUnion):
			#print "YES"
			return True
		#print "NO"
		return False

	def functionParameterList(self):
		debug_info("FUNCTION PARAMETER LIST")
		self.symbols.saveScope()
		self.functionDefinition = 2
	
	def beginDeclaration(self):
		pass
	
	def endDeclaration(self):
		pass
	
	def beginFieldDeclaration(self):
		pass

	def beginParameterDeclaration(self):
		debug_info("BEGIN PARAMETER DECLARATION")
	
	def declaratorParameterList(self, declaration):
		self.symbols.saveScope()
	
	def declaratorEndParameterList(self, declaration):
		if not declaration:
			self.symbols.removeScope()
			self.symbols.restoreScope()
	
	def functionEndParameterList(self, definition):
		if not definition:
			debug_info("function end parameter list - no defn")
			self.symbols.removeScope()
			self.symbols.restoreScope()
		else:
			debug_info("function end parameter list - in body")
			# Change flag from parameter list to body of definition (ie 2 to 3)
			self.functionDefinition = 3
	
	def end_of_stmt(self):
		self.myCode_end_of_stmt()

	def enterNewLocalScope(self):
		self.symbols.saveScope()
	
	def exitLocalScope(self):
		self.symbols.removeScope()
		self.symbols.restoreScope()
	
	def endFunctionDefinition(self):
		self.symbols.removeScope()
		self.symbols.restoreScope()
		self.functionDefinition = 0
	
	def exitExternalScope(self):
		self.symbols.removeScope()
		self.symbols.restoreScope()
		if self.symbols.getCurrentScopeIndex() == 0:
			debug_info("exitExternalScope: scope now %d" % (self.symbols.getCurrentScopeIndex()))
		else:
			debug_info("exitExternalScope: scope now %d, should be 0" %\
					(self.symbols.getCurrentScopeIndex()))

	def beginClassDefinition(self, type_specifier, tag):
		if self.symbols.lookup(tag) is not None:
			self.symbols.saveScope()
			# FIXME: statementTrace debugging
			debug_warn("beginClassDefinition: class already defined (?)")
			return
		if type_specifier == "struct":
			c = CPPSymbol(tag, otStruct)
		elif type_specifier == "union":
			c = CPPSymbol(tag, otUnion)
		elif type_specifier == "class":
			c = CPPSymbol(tag, otClass)
		else:
			raise Exception("Unknown type specifier")
		self.symbols.defineInScope(tag, c, self.externalScope)
		# FIXME: statementTrace debugging
		self.qualifierPrefix += '%s::' % (tag)
		self.symbols.saveScope()
	
	def endClassDefinition(self):
		self.symbols.removeScope()
		self.symbols.restoreScope()
		# Remove final T:: from A::B::C::T:: ...
		second_last_scope = self.qualifierPrefix.rfind('::', 0, -3)
		self.qualifierPrefix = self.qualifierPrefix[ : second_last_scope + 2]
	
	def myCode_function_direct_declarator(self, id_str):
		debug_info("direct declarator for id %s" % (id_str))
	
	def myCode_end_of_stmt(self):
		pass
	
	def declaratorArray(self):
		pass
	
	def classForwardDeclaration(self, ts, ds, id):
		pass

	def classForwardDeclaration(self, a, b, c):
		pass
	
	def enumElement(self, txt):
		pass
		
	def beginEnumDefinition(self, txt):
		pass
	
	def endEnumDefinition(self):
		pass
	
	def beginTemplateParameterList(self):
		pass
	
	def templateTypeParameter(self, ident):
		pass
	
	def endTemplateParameterList(self):
		pass
	
	def line_directive(self, included_file, included_line_num):
		self.setFilename(included_file)

	def reportError(self, x):
		raise error.from_antlr(x, self.getFilename())

class LineDirectiveHandler(object):
	def __init__(self):
		self.parser = None
		self.lexer = None

	def __call__(self, included_file, included_line_num):
		included_file = included_file.strip()

		opts_pos = len(included_file)
		while included_file[opts_pos - 1] in '1234 ':
			opts_pos -= 1

		opts = included_file[opts_pos:]
		included_file = included_file[:opts_pos]

		if included_file.startswith('"'):
			included_file = included_file[1:]

		if included_file.endswith('"'):
			included_file = included_file[:-1]
		
		debug_info("#line: %s, %s" % (included_file, included_line_num))
		self.lexer.setLine(int(included_line_num) - 1)
		self.parser.line_directive(included_file, included_line_num)
	
	def set_parser(self, parser):
		self.parser = parser
	
	def set_lexer(self, lexer):
		self.lexer = lexer

def walk(tree, depth = 0):
	while tree:
		print "  " * depth, tree.getText()
		walk(tree.getFirstChild(), depth + 1)
		tree = tree.getNextSibling()

def gethandle(filename):
	if filename == '-':
		handle = sys.stdin
	else:
		handle = file(filename)
	
	return handle

def parse_data_to_pt(filename, data, accelerate = True):
	if cppparse and accelerate:
		result = cppparse.get_pt(data)
	else:
		inputhandle = StringIO(data)
		line_handler = LineDirectiveHandler()
		lexer = Lexer(inputhandle, line_directive_handler = line_handler)
		parser = Parser(lexer)
		parser.setFilename(filename)
		line_handler.set_parser(parser) # Messy
		line_handler.set_lexer(lexer)
		result = parser.translation_unit()

	return result

def parse_to_pt(filename, cpp_options = None, accelerate = True, debug_dump_cpp = None):
	if cpp_options is None:
		cpp_options = {}

	data = cpp_file(filename, **cpp_options)

	if debug_dump_cpp:
		handle = file(debug_dump_cpp, 'wb')
		handle.write(data)
		handle.close()

	return parse_data_to_pt(filename, data, accelerate = accelerate)

def parse_to_ast(basicast, pt, filename, **kwargs):
	ast = ast_gen(basicast, pt, filename, **kwargs)
	return ast

def parse_data(basicast, filename, data, **kwargs):
	pt = parse_data_to_pt(filename, data)
	parse_to_ast(basicast, pt, filename, **kwargs)
	#basicast.add_child(ast)
	return basicast

def parse(filename, cpp_options = None, use_cpp = True, accelerate = False, **kwargs):
	if use_cpp:
		pt = parse_to_pt(filename, cpp_options, accelerate = accelerate)
	else:
		data = gethandle(filename).read()
		pt = parse_data_to_pt(filename, data)
	
	# Create the standard C/C++ AST.
	basicast = create_basictype_ast('Generic 32', 'idl4')
	# Create AST and return.
	return parse_to_ast(basicast, pt, filename, **kwargs)

def test(inputhandle):
	lexer = Lexer(inputhandle)
	parser = Parser(lexer)
	parser.translation_unit().print_tree()

if __name__ == '__main__':
	handle = file(sys.argv[1])
	test(handle)
