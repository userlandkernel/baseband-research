
class DictEntry(object):
	def __init__(self, k = None, hashCode = -1):
		self.key = k
		self.hashCode = hashCode
	
	def getKey(self):
		return self.key

	def setKey(self, key):
		self.key = key

	def getHashCode(self):
		return self.hashCode

	def setHashCode(self, hashCode):
		self.hashCode = hashCode

	def getNext(self):
		return self.next

	def setNext(self, next):
		self.next = next

	def getNextInScope(self):
		return self.scope

	def setScope(self, scope):
		self.scope = scope

