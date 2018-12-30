from infogripper import *

# Errors
class Error(Exception):
	pass

# Helpers
class Undefined(object):
	pass

class Result(object):
	def __init__(self, type_ast, value):
		self.type_ast = type_ast
		self.value = value
	
class Error_AST(object):
	def __init__(self, error_type):
		self.error_type = error_type
	
	def __str__(self):
		return '<Error AST: %s>' % (self.error_type)
	
class TypeFinder(object):
	pass

class Evaluator(object):
	def __init__(self):
		pass

	def evaluate(self, ast):
		"""
		Evaluates expression nodes. Raises Error if the expression couldn't be evaluated.
		(This will happen if the expression is non-constant, or even (currently) if the
		expression is constant but is not composed of literals.)
		"""
		handler = getattr(self, "eval_%s" % (ast.name))
		return handler(ast)

	def eval_expression(self, ast):
		if ast.has_attribute('value'):
			return ast.attribute('value')
		elif ast.leaf:
			return getattr(self, "eval_%s" % (ast.leaf))(ast)
		elif len(ast['expression']) == 1:
			return self.evaluate(ast['expression'][0])
		else:
			ast.print_tree()
			raise Error()
	
	def eval_integer(self, ast):
		type_ast = infogripper.getTypenode(ast, 'int')
		value = int(ast.get_single_attribute('value'))
		return Result(type_ast, value)
	
	def eval_identifier(self, ast):
		# FIXME: Find the type of the identifier.
		type_ast = Error_AST('identifier')
		value = ast.get_single_attribute('value')
		return Result(type_ast, value)
	
	def eval_scoped_name(self, ast):
		value = ast.get_single_attribute('value')
		return value
	
	def eval_add(self, ast):
		raise NotImplementedError()

class Flattener(object):
	"""
	Flattens an AST. Does not do type-checking or basically anything else.
	"""
	def flatten(self, ast):
		handler = getattr(self, "flatten_%s" % (ast.name))
		return handler(ast)

	def flatten_expression(self, ast):
		if ast.has_attribute('value'):
			return ast.attribute('value')
		elif ast.leaf:
			return getattr(self, "flatten_%s" % (ast.leaf))(ast)
		elif ast.the('raw') is not None:
			return self.flatten_raw(ast.the('raw'))
		elif len(ast['expression']) == 1:
			return self.flatten(ast['expression'][0])
		else:
			raise Error()

	def flatten__value(self, ast):
		return ast.get_single_attribute('value')
	flatten_integer = flatten__value
	flatten_float = flatten__value
	flatten_scoped_name = flatten__value
	flatten_raw = flatten__value
	
	def flatten_add(self, ast):
		return self._fo('+', ast)
	
	def _fo(self, oper, ast):
		oper = ' %s ' % (oper)
		result = [str(self.flatten(child)) for child in ast['expression']]
		return '(%s)' % (oper.join(result))

def get_type(ast):
	typefinder = TypeFinder()
	return typefinder.go(ast)

def evaluate(ast):
	evaluator = Evaluator()
	return evaluator.evaluate(ast)

def flatten(ast):
	flattener = Flattener()
	return flattener.flatten(ast)
