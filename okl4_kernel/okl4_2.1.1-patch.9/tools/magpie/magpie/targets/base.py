import os

class Templates(object):
	template_base = ''
	def get(self, name):
		template_name = getattr(self, name, None)
		if template_name:
			return os.path.join(self.template_base, template_name)
		else:
			raise KeyError("%s not found" % (name))
	public = []
