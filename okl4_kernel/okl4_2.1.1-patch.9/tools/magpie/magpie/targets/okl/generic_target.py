from magpie.generator.okl_generator import Generator
from magpie.targets import base

class Templates(base.Templates):
	MAPPING = 'CORBA C'

	client = 'okl/client.template.c'
	service = 'okl/service.template.c'
	serviceheaders = 'okl/service_headers.template.h'

	# Helpers
	service_pagefault = 'okl/service_pagefault.template.c'

	# Marshalling and unmarshalling
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

