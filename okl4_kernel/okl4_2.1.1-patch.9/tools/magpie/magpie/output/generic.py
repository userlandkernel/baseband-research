import os.path
from cStringIO import StringIO

from magpie.output.wsogmm import markup
from magpie.targets.shared.options import options

def reverse(data):
	for index in range(len(data)-1, -1, -1):
		yield data[index]

class DualAccess(object):
	def __getitem__(self, key):
		return getattr(self, key)
	
	def __setitem__(self, key, value):
		setattr(self, key, value)
	
	def fromdict(self, sourcedict):
		for key, value in sourcedict.items():
			self[key] = value

	def setup(self, *args, **kwargs):
		for key, value in kwargs.items():
			self[key] = value
	
class OutputContext(dict):
	""" Class that lets me call attributes as inst['attribute'] or
		inst.attribute freely.
	"""
	def __init__(self):
		self.base_scope = {'internal': DualAccess(),
			'file': DualAccess(),
			'function': DualAccess(),
			'current': DualAccess(),
		}
	
	def get_file(self):
		return self.base_scope['file']
	file = property(get_file)

	def get_function(self):
		return self.base_scope['function']
	function = property(get_function)

	def get_current(self):
		return self.base_scope['current']
	current = property(get_current)

	def __setitem__(self, key, value):
		self.base_scope[key] = value
	
	def __getitem__(self, key):
		for scope in (self.local_scope, self.base_scope):
			if scope.has_key(key):
				return scope[key]
		raise KeyError(key)
	
	def get(self, key, default = None):
		try:
			return self.__getattr__(key)
		except KeyError:
			return default
	
class BaseOutput(object):
	def __init__(self):
		# Calculate path to templates
		self.context = OutputContext()
		self.hook_function = None
		self.output = StringIO()
	
	def fromtemplate(self, template_filename, **kwargs):
		assert template_filename is not None
		self.context.current.fromdict(kwargs)
		# Could cache these, but doubt it will become a problem.
		markup(template_filename, self.output, self.context.base_scope)
	
	def getvalue(self):
		return self.output.getvalue()
	
	def set_hook_object(self, hook_object):
		self.hook_object = hook_object
	
	def call_hook(self, func_name, *args, **kwargs):
		some_callable = getattr(self.hook_object, func_name)
		some_callable(*args, **kwargs)

class GenericOutput(BaseOutput):
	def __init__(self):
		BaseOutput.__init__(self)
	
	def go(self, template_filename):
		self.fromtemplate(template_filename)

