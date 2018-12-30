from generator import simple_params
from generator import v4generator
import targets.idl4.generic_l4v4_target as idl4

class NICTASimpleParam(simple_params.SimpleParam):
	"""
	NICTA uses a slightly different way of determining whether to pass
	parameters by value or by reference. 
	See Mothra Issue #692 for more information.
	"""
	def create(self, parameter_decl):
		attributes_dict = simple_params.SimpleParam.create(self, parameter_decl)
		# Do our own handling of c_impl_indirection: we pass by value only if
		# the type is basic.
		the_type = parameter_decl.maybe_walk_to('target', 'type')
		if attributes_dict['direction'] in ('out', 'inout') or not self._is_type_basic(the_type):
			attributes_dict['c_impl_indirection'] = '*'
		else:
			attributes_dict['c_impl_indirection'] = ''
		return attributes_dict
	
	def _is_type_basic(self, type_ast):
		metatype = type_ast.get_attribute('meta_type')[0]
		if metatype == 'basic':
			return True
		elif metatype == 'alias':
			target = type_ast.maybe_walk_to('target', 'type')
			return self._is_type_basic(target)
		else:
			return False
	
class Function(v4generator.V4Function):
	def make_simple_param_list(self, decl_ast):
		return v4generator.V4Function.make_simple_param_list(self, decl_ast,
				simpleparam_cls = NICTASimpleParam)

class Interface(v4generator.V4Interface):
	def _my_function_class(self):
		return Function

class Generator(idl4.Generator):
	def _my_interface_class(self):
		return Interface

class Templates(idl4.Templates):
	MAPPING = 'CORBA C'
	client_function_body_pre_ipc_defs = 'v4nicta_generic/client_function_body_asynchmask_defs.template.c'
	client_function_body_pre_ipc = 'v4nicta_generic/q1api_client_function_body_save_asynchmask.template.c'
	client_function_body_post_ipc = 'v4nicta_generic/q1api_client_function_body_restore_asynchmask.template.c'
