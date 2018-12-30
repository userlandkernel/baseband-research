from magpie.targets.nicta import generic_n2_target as nictageneric_n2
from magpie.targets.idl4 import generic_biguuid_l4v4_target as idl4_biguuid

Generator = nictageneric_n2.Generator

class Templates(nictageneric_n2.Templates, idl4_biguuid.Templates):
	client_function_body = 'v4nicta_generic/client_function_body_noabi.template.c'
	service_function = 'v4nicta_generic/service_function_backcompat_noabi.template.h'
