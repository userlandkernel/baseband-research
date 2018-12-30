class Error(Exception):
	def __init__(self, filename = None, line = None, column = None, text = None, node = None):
		self._filename = filename
		self._line = line
		self._column = column
		self._text = text
		if node:
			self._filename = node.source_file
			self._line = node.source_line
			self._column = -1
		Exception.__init__(self, "Parser error")
	
	def __str__(self):
		result = '%s:%s:%s' % (self.filename, self.line, self.text)
		return result
		
	def get_filename(self):
		if self._filename:
			return self._filename
		else:
			return '<unknown>'
	filename = property(get_filename)

	def get_line(self):
		return self._line
	line = property(get_line)

	def get_column(self):
		return self._column
	column = property(get_column)

	def get_text(self):
		return self._text
	text = property(get_text)

class RecognitionError(Error):
	pass

class UnknownError(Error):
	pass

def from_antlr_token(t, filename = None, text = None):
	line = t.getLine()
	column = t.getColumn()
	token_name = t.getText()
	if text is None:
		text = 'Unexpected token "%s"' % (token_name)
	return RecognitionError(filename = filename, line = line, column = column,
			text = text)

def from_antlr(e, filename = None):
	if hasattr(e, 'line') and hasattr(e, 'column') and hasattr(e, 'tokenText'):
		return RecognitionError(filename = filename, line = e.line, column = e.column, text = e.tokenText)
	elif hasattr(e, 'token') and e.token:
		return from_antlr_token(e.token, filename)
	elif hasattr(e, 'recog') and e.recog:
		line = e.recog.line
		column = e.recog.column
		text = str(e.recog)
		return RecognitionError(filename = filename, line = line, column = column,
				text = text)
	else:
		return UnknownError(e)


class SemanticError(Error):
	pass

class UnknownTypeError(SemanticError):
	def __init__(self, failed_name = None, *args, **kwargs):
		kwargs['text'] = 'Unknown type "%s"' % (failed_name)
		SemanticError.__init__(self, *args, **kwargs)

class SymbolDefinedTwiceError(SemanticError):
	def __init__(self, name, node1, node2):
		self.node1 = node1
		self.node2 = node2
		self.clashname = name

		SemanticError.__init__(self, node = node2, text = 'Name "%s" defined twice in same scope' % (name))
	
	def __str__(self):
		result_1 = "%s:%s:%s" % (self.node2.source_file,
				self.node2.source_line,
				'Name "%s" defined twice in same scope' % (self.clashname))
		result_2 = "%s:%s:%s" % (self.node1.source_file,
				self.node1.source_line,
				'Name "%s" was previously defined here.' % (self.clashname))

		return '%s\n%s'%(result_1, result_2)

class AmbiguousScopedNameError(SemanticError):
	def __init__(self, name, ref_ast, defining_ast_list):
		self.ref_ast = ref_ast
		self.defining_asts = defining_ast_list
		self.clashname = name

		SemanticError.__init__(self, node = ref_ast, text = "Symbol '%s' is ambiguous without explicit scoping" % (name))
	
	def __str__(self):
		def_ast_text = ['%s:%s:%s' % (node.source_file, node.source_line,
				'Name "%s" defined here' % (self.clashname))
				for node in self.defining_asts]

		text = ['%s:%s:%s' % (self.ref_ast.source_file, self.ref_ast.source_line,
				'Name "%s" is ambiguous without explicit scoping' % (self.clashname))] \
				+ def_ast_text

		return '\n'.join(text)

class InvalidDiscriminatorError(SemanticError):
	def __init__(self, node):
		SemanticError.__init__(self, text = "Invalid type for discriminated union", node = node)

class InheritanceError(SemanticError):
	pass

class InvalidTypeUsageError(SemanticError):
	def __init__(self, typename, node):
		SemanticError.__init__(self, text = "Cannot use '%s' in this context" % (typename), node = node)

class CannotReconcileTypesError(SemanticError):
	def __init__(self, typename_from, typename_to, node = None):
		text = 'Cannot coerce "%s" to "%s"' % (typename_from, typename_to)

		SemanticError.__init__(self, text = text, node = node)

class DimensionOutOfRangeError(SemanticError):
	def __init__(self, node):
		text = 'The specified dimension is out of range'
		SemanticError.__init__(self, text = text, node = node)

class NameNotFoundError(SemanticError):
	def __init__(self, name, node):
		text = 'The name "%s" could not be found' % (name)
		SemanticError.__init__(self, text = text, node = node)
