DIAG_MSGES = True

# Catch bugs but don't impose practical limitations
SCOPES_MAX = 200

class CPPDictionary(object):
	""" A scoped dictionary. """
	def __init__(self):
		self.currentScope = 0
		# The C++ implementation is a full hash table, but here I can just
		# defer to Python. This implementation implements a dictionary of dictionaries.
		# self.scopes maps <scope number> to <dictionary of symbols>. 
		self.scopes = {0: {}}
		# The latest entry for each scope
		self.endScope = {0: None}

	def dumpSymbol(output_file, dict_entry):
		if DIAG_MSGES:
			print "dumpSymbol not implemented - consult CPPDictionary.hpp for implementation"

	def getCurrentScopeIndex(self):
		return self.currentScope
	
	def saveScope(self):
		# Advance scope number (for included scope)
		self.currentScope += 1
		# Always create a new empty symbol table for the new scope.
		self.scopes[self.currentScope] = {}
		self.endScope[self.currentScope] = None
		if self.currentScope > SCOPES_MAX:
			raise OverflowError("Scope overflow")

	def define(self, key, entry):
		self.defineInScope(key, entry, self.currentScope)

	def defineInScope(self, key, entry, scope_num):
		entry.this_scope = scope_num
		entry.setKey(key)
		entry.setHashCode(0xdeadbeefL) # We don't use this
		if self.endScope[scope_num] is not None:
			self.endScope[scope_num].setScope(entry)
			self.endScope[scope_num] = entry
		self.scopes[scope_num][key] = entry
	
	def removeScope(self, scope_num = -1):
		if scope_num == -1:
			scope_num = self.currentScope
		self.scopes[scope_num] = {}
		self.endScope[scope_num] = None
	
	def restoreScope(self):
		assert self.currentScope > 0
		self.currentScope -= 1
	
	def lookup(self, key):
		check_scope = self.currentScope
		#print "Lookup for key", key
		while check_scope >= 0:
			#print "lookup in scope", check_scope
			if key in self.scopes[check_scope]:
				#print "Lookup for key found in this scope"
				return self.scopes[check_scope][key]
			check_scope -= 1
		#print "lookup failed"
		return None
			
	def dumpScope(self, handle, scope_num = -1):
		if scope_num == -1:
			scope_num = self.currentScope
		for key in self.scopes[scope_num]:
			self.dumpSymbol(handle, self.scopes[scope_num][key])

	def dumpSymbol(self, handle, symbol):
		print >>handle, "%s" % (symbol.getKey())

