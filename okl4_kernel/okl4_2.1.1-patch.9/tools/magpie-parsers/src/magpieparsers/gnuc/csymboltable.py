class CSymbolTable(object):
	def __init__(self):
		# Holds list of scopes
		self.scope_stack = []

		# Table where all defined names are mapped to TNode tree nodes
		self.symbol_table = {}

	def pushScope(self, s):
		self.scope_stack.push(s)
	
	def popScope(self):
		return self.scope_stack.pop()

	def currentScopeAsString(self):
		""" Return the current scope as a string """
		return '::'.join(self.scope_stack[1:])
	
	def addCurrentScopeToName(self, name):
		""" Given a name for a type, append the current scope to it. """
		current_scope = self.currentScopeAsString()

		return self.addScopeToName(current_scope, name)

	def addScopeToName(self, scope, name):
		""" Given a name for a type, append the current scope to it. """
		if scope is None or len(scope) > 0:
			return '%s::%s' % (scope, name)
		else:
			return name

	def removeOneLevelScope(self, scope_name):
		""" Remove one level of scope from name """
		if '::' in scope_name:
			return scope_name[:scope_name.rfind('::')]

		if len(scope_name) > 0:
			return ''

		return None

	def add(self, name, node):
		""" Add a node to the table with its key as the current scope and the
		name """

		old = self.symbol_table.get(name)

		self.symbol_table[name] = self.addCurrentScopeToName(name)

		return old

	def lookupScopedName(self, scoped_name):
		""" Lookup a fully-scoped name in the symbol table """
		return self.symbol_table.get(scoped_name)
	
	def lookupNameInCurrentScope(self, name):
		""" Lookup an unscoped name in the table by prepending the current
		scope. If not found, pop scopes and look again. """
		scope = self.currentScopeAsString()
		tnode = None

		while tnode is None and scope is not None:
			scoped_name = self.addScopeToName(scope, name)
			tnode = self.symbol_table.get(scoped_name)
			scope = self.removeOneLevelScope(scope)

		return tnode

	def toString(self):
		result = ['CSymbolTable { \nCurrentScope: %s\nDefinedSymbols:\n' %
				(self.currentScopeAsString())]
		for key in self.symbol_table:
			result.append('%s (%s)\n' % (key,
					TNode.getNameForType(self.symbol_table[key].getType())))
		result.append('}\n')
		return ''.join(result)

