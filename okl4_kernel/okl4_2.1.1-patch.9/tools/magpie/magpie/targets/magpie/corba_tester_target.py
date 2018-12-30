from targets import base
from generator.v4generator import V4Generator

Generator = V4Generator

class Templates(base.Templates):
	annotations = 'corba_tester/annotations.template.txt'

	public = ['annotations']

