Generator = 'v4'

#InheritFrom('idl4_generic_l4v4')
class Templates(OldTemplates):
	"""
	Biguuid templates share almost everything with generic.
	"""
	MAPPING = 'MIG-IDL to C'

	client = 'mig_generic/main_client.template.h'

	service = 'mig_generic/main_service.template.h'
	
	mig_typedefs = 'mig_generic/mig_typedef.template.c'
	mig_client_typedefs = 'mig_generic/mig_client_typedef.template.c'
	mig_server_typedefs = 'mig_generic/mig_server_typedef.template.c'
	
	public = ['client', 'service']
