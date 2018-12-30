from magpie.targets.nicta import generic_n2_target as nictageneric_n2
from magpie.targets.idl4 import generic_biguuid_l4v4_target as idl4_biguuid

Generator = nictageneric_n2.Generator

class Templates(nictageneric_n2.Templates, idl4_biguuid.Templates):
	client_function_body = 'v4nicta_generic/client_function_body_noabi.template.c'
	# Note: the following is incompatible with previous servers.
	service = 'v4nicta_generic/main_service_noabi.template.c'
	service_pagefault = 'v4nicta_generic/main_service_noabi_pagefault.template.c'
	serviceheaders = 'v4nicta_generic/main_templates_noabi.template.h'
