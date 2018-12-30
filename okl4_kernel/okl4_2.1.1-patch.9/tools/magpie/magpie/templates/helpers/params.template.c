/*-FIXME REMOVE THIS -*/
/*-template_version = 2-*/
/*-
# Input, output, and return parameters:
_in_params = [param for param in function.params if param['direction'] in ['in', 'inout'] and param['flags'] == ['call']]
_out_params = [param for param in function.params if param['direction'] in ['out', 'inout'] and param['flags'] == ['call']]
_return_param = [param for param in function.params if param['direction'] == 'return'][0]

# Indirection types:
# "indirection" is the indirection level specified in the IDL.
# "func_arg_direction" is the indirection level required for the parameter
#     when passed to a function call. It is the same as 'indirection', except
#     when 1) there is no indirection and the parameter cannot be passed by value
#     or   2) there is no indirection and the parameter is an output parameter.
# "deref_indirection" is the indirection level required to access the parameter's
#     value for input. It is usually no indirection, but if "func_arg_indirection"
#     satisfies either of the two above criteria it is set to func_arg_indirection.

msgtag_special = ''
for param in function.params:
	if 'priv' in param['flags']:
		assert msgtag_special == ''
		msgtag_special = '+(%s & 7)' % (param['name'])

	param['func_arg_indirection'] = param['indirection']
	param['deref_indirection'] = ''
	if param['func_arg_indirection'] == '':
		if param['idltype'] is not None and param['idltype'].is_value_type is not True:
			param['func_arg_indirection'] += '*'
		elif param['direction'] is not None and param['direction'] in ['out', 'inout']:
			param['func_arg_indirection'] += '*'
		param['deref_indirection'] = param['func_arg_indirection']
-*/
