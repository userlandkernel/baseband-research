from antlr import CommonToken

class CToken(CommonToken):
	def __init__(self, *args, **kwargs):
		self.source = ''
		self.tokenNumber = 0
		CommonToken.__init__(self, *args, **kwargs)

	def getSource(self):
		return self.source

	def setSource(self, src):
		self.source = src

	def getTokenNumber(self):
		return self.tokenNumber

	def setTokenNumber(self, i):
		self.tokenNumber = i

	def toString(self):
		return 'CToken: (%s) [%s] %s line:%s source: %s' % (
				hash(self),
				self.getType(),
				self.getText(),
				self.getLine(),
				self.source)
	__repr__ = toString

