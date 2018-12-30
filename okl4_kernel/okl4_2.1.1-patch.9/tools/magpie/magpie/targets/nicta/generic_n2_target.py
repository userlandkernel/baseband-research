from magpie.generator import simple_params
from magpie.generator import v4generator
import magpie.targets.idl4.generic_l4v4_target as idl4

class Function(v4generator.V4Function):
	BYREF_STYLE = 'nicta_n2'

class Interface(v4generator.V4Interface):
	def _my_function_class(self):
		return Function

class Generator(idl4.Generator):
	def _my_interface_class(self):
		return Interface

class Templates(idl4.Templates):
	MAPPING = 'CORBA C'
	#
	# Marshalling
	#
	set_varptr = 'v4nicta_generic/set_varptr.template.c'
	marshal_8 = 'v4nicta_generic/marshal_8.template.c'
	marshal_16 = 'v4nicta_generic/marshal_16.template.c'
	marshal_32 = 'v4nicta_generic/marshal_32.template.c'
	marshal_64 = 'v4nicta_generic/marshal_64.template.c'
	marshal_smallstring = 'v4nicta_generic/marshal_smallstring.template.c'

	unmarshal_8 = 'v4nicta_generic/unmarshal_8.template.c'
	unmarshal_16 = 'v4nicta_generic/unmarshal_16.template.c'
	unmarshal_32 = 'v4nicta_generic/unmarshal_32.template.c'
	unmarshal_64 = 'v4nicta_generic/unmarshal_64.template.c'
	unmarshal_smallstring = 'v4nicta_generic/unmarshal_smallstring.template.c'

	client_function_body_pre_ipc_defs = 'v4nicta_generic/client_function_body_asynchmask_defs.template.c'
	client_function_body_pre_ipc = 'v4nicta_generic/n2api_client_function_body_save_asynchmask.template.c'
	client_function_body_post_ipc = 'v4nicta_generic/n2api_client_function_body_restore_asynchmask.template.c'
