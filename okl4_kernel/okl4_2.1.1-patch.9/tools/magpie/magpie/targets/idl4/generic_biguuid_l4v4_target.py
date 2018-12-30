import magpie.targets.idl4.generic_l4v4_target as generic
from magpie.targets import base

Generator = generic.Generator

class SharedTemplates(generic.SharedTemplates):
	basic_includes = 'v4_generic_biguuid/basic_includes.template.c'
	clientservice_function_params_maker = 'v4_generic_biguuid/function_params_maker.template.c'
	preamble = 'v4_generic_biguuid/preamble.template.c'

class ClientTemplates(generic.ClientTemplates):
	client_function_create_id = 'v4_generic_biguuid/client_function_create_id.template.c'
	client_function_marshal = 'v4_generic_biguuid/client_function_marshal.template.c'

class ServiceTemplates(generic.ServiceTemplates):
	pass

class ServiceTemplateTemplates(generic.ServiceTemplateTemplates):
	servicetemplate_create_getiid_func = 'v4_generic_biguuid/servicetemplate_getiid.template.c'

class Templates(base.Templates, SharedTemplates, ClientTemplates, ServiceTemplates, ServiceTemplateTemplates, generic.ConstantTemplates, generic.Types):
	"""
	Biguuid templates share almost everything with generic.
	"""
	MAPPING = 'CORBA C'

	public = ['client', 'service']
