Generator = "v4_arm"

class Templates(OldTemplates):
	MAPPING = 'CORBA C'
	client_function_body = 'v4_arm_biguuid/client_function_body.template.c'
	client_function_body_fallback = 'v4_generic/client_function_body.template.c'
	basic_includes = 'v4_arm_biguuid/basic_includes.template.c'
	preamble = 'v4_arm_biguuid/preamble.template.c'
