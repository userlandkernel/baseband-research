from DictEntry import DictEntry

# "enum ObjectType"
otInvalid = 0
otFunction = 1
otVariable = 2
otTypedef = 3
otStruct = 4
otUnion = 5
otEnum = 6
otClass = 7
otEnumElement = 8

# "enum ObjectFunction"
ofNormal = 0
ofAddress = 1
ofPointer = 2

class CPPSymbol(DictEntry):
	def __init__(self, k = None, ot = otInvalid, of = ofNormal):
		DictEntry.__init__(self, k)
		self.type = ot
		self.function = of
	
	def getType(self):
		return self.type
	
	def setType(self, t):
		self.type = t
	
	def getFunction(self):
		return self.function
	
	def setFunction(self, f):
		self.function = f

