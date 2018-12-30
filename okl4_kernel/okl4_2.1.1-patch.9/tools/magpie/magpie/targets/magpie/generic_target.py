# Magpie generic mode
# FIXME: This should go!

from generator.v4generator import V4Generator
from targets import base
from targets.idl4 import generic_biguuid_l4v4_target as v4big
import targets.idl4.generic_l4v4_target as generic

Generator = V4Generator

class Templates(base.Templates, v4big.SharedTemplates, v4big.ClientTemplates, generic.ConstantTemplates, generic.Types):
	"""
	Keep the "client" templates, redefine the "service" templates (for now)
	"""
	MAPPING = 'CORBA C' # Required for client templates.
	service = 'v4_magpie/main_service.template.c'

	public = ['client', 'service']
