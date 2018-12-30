import targets.idl4.generic_l4v4_target as generic
import targets.idl4.generic_biguuid_l4v4_target as biguuid
from targets import base

Generator = generic.Generator

class SharedTemplates(biguuid.SharedTemplates):
	pass

class ClientTemplates(biguuid.ClientTemplates):
	client_function_body = "v4nicta_generic_optimised/client_function_body.template.c"

class ServiceTemplates(biguuid.ServiceTemplates):
	pass

class ServiceTemplateTemplates(biguuid.ServiceTemplateTemplates):
	pass

class Templates(base.Templates, SharedTemplates, ClientTemplates, ServiceTemplates, ServiceTemplateTemplates, generic.ConstantTemplates, generic.Types):
	"""
	Biguuid templates share almost everything with generic.
	"""
	MAPPING = 'CORBA C'

	public = ['client', 'service']
