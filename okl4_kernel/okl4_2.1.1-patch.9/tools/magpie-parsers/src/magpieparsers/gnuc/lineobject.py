class LineObject(object):
	def __init__(self, template = None):
		if isinstance(template, LineObject):
			self.parent = template.parent
			self.source = template.source
			self.line = template.line
			self.enteringFile = template.enteringFile
			self.returningToFile = template.returningToFile
			self.systemHeader = template.systemHeader
			self.treatAsC = template.treatAsC
		elif isinstance(template, basestring):
			self.source = template
		else:
			self.parent = None
			self.source = ''
			self.line = 1
			self.enteringFile = False
			self.returningToFile = False
			self.systemHeader = False
			self.treatAsC = False
	
	def newline(self):
		self.line += 1

