/*-
if function.get_is_pagefault():
	# No UUID for pagefaults.
	msg_params_in = []
else:
	msg_params_in = [('long', '_uuid')]

msg_params_in.extend([(item['typename'], '%s%s' % (item['indirection'], item['name'])) for item in function.get_params_in()])

msg_params_out = [(item['typename'], '%s%s' % (item['indirection'], item['name'])) for item in function.get_params_out()]

msg_params_inout = [(item['typename'], '%s%s' % (item['indirection'], item['name'])) for item in function.get_params_inout()]

if function.get_return_type() != 'void':
	msg_params_out.append( (function.get_return_type(), '__retval') )
-*/
