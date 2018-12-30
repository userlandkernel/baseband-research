"""
Basic "generic IDL4" target. Not designed to inherit from anything.
"""
from magpie.targets import base
from magpie.generator.v4generator import V4Generator

class Generator(V4Generator):
	pass

class Types(object):
	# Methods relating to type declaration, conversion, and marshalling
	types_cpp_define_int_constants = 'types/cpp_define_int_constants.template.c'
	# Given a type instance AST, output a declaration.
	types_declare = 'types/declare.template.c'
	types_declare_const = 'types/declare_const.template.c'
	# Given a type AST, output the name of the type.
	types_name = 'types/name.template.c'
	types_name_basic = 'types/name_basic.template.c'
	types_name_alias = 'types/name_alias.template.c'
	# Given a Python value, output a C constant representing that value.
	types_constant = 'types/constant.template.c'

class SharedTemplates(object):
	basic_includes = 'v4_generic/basic_includes.template.c'
	clientservice_function_params_maker = 'v4_generic/function_params_maker.template.c'
	# Headers
	header_comments = 'v4_generic/headercomments.template.c'
	idl4_defines = 'v4_generic/idl4_defines.template.c'
	preamble = 'v4_generic/preamble.template.c'
	interface_wrapper = 'v4_generic/interface_wrapper.template.c'
	imports = 'v4_generic/import.template.c'

	language_specific_remapping = 'v4_generic/null_remapping.template.c'

class ClientTemplates(object):
	client_function_wrapper = 'v4_generic/client_function_wrapper.template.c'
	client_function_body = 'v4_generic/client_function_body.template.c'
	client_function_body_pre_ipc_defs = 'helpers/null.template.c'
	client_function_body_pre_ipc = 'helpers/null.template.c'
	client_function_body_post_ipc = 'helpers/null.template.c'
	client_function_create_id = 'v4_generic/client_function_create_id.template.c'
	client_function_marshal = 'v4_generic/client_function_marshal.template.c'
	client = 'v4_generic/main_client.template.h'

class ServiceTemplates(object):
	service_interface_wrapper = 'v4_generic/service_interface_wrapper.template.c'
	service_function = 'v4_generic/service_function.template.c'
	service_function_reply = 'v4_generic/service_function_reply.template.c'
	service = 'v4_generic/main_service.template.h'

class ServiceTemplateTemplates(object):
	servicetemplate_create_getiid_func = 'v4_generic/servicetemplate_getiid.template.c'
	servicetemplate = 'v4_generic/main_servicetemplate.template.h'

class ConstantTemplates(object):
	constants = 'v4_generic/main_constants.template.c'

class Templates(base.Templates, SharedTemplates, ClientTemplates, ServiceTemplates, ServiceTemplateTemplates, ConstantTemplates, Types):
	MAPPING = 'CORBA C'

	types = 'v4_generic/main_types.template.h'
	
	public = ['client', 'service', 'servicetemplate', 'types']

class DataStorage(object):
	def __init__(self):
		self.data_dict = {}
