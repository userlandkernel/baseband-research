Generator = 'v4'

#InheritFrom('idl4_generic_l4v4')
class Templates(OldTemplates):
	"""
	"""
	MAPPING = 'C++ Experimental'
	client_function_wrapper = 'cplusplus/client_function_wrapper.template.cpp'

	interface_wrapper = 'cplusplus/interface_wrapper.template.cpp'

	imports = 'cplusplus/import.template.cpp'
	language_specific_remapping = 'cplusplus/cplusplus_remapping.template.c'
