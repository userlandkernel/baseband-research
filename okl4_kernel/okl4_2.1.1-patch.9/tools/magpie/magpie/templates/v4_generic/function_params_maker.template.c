/*-
msg_params_in = [(item['type'].leaf, '%s%s' % (item['indirection'], item['name'])) for item in function.get_params_in() if 'marshal' in item['flags']]

msg_params_out = [(item['type'].leaf, '%s%s' % (item['indirection'], item['name'])) for item in function.get_params_out() if 'marshal' in item['flags']]

msg_params_inout = [(item['type'].leaf, '%s%s' % (item['indirection'], item['name'])) for item in function.get_params_inout()]

if function.get_return_type() != 'void':
	msg_params_out.append( (function.get_return_type(), '__retval') )
-*/
