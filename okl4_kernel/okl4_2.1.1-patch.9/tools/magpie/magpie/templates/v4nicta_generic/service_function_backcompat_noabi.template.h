!!explicit_linebreaks
\n
/*-
if function.get_is_pagefault():
	unmarshal_start = 1
else:
	unmarshal_start = 2
-*/
/*-run(templates.get('clientservice_function_params_maker'))-*/

/*-function_implementation_name = function.get_name() + '_implementation' -*/

/*-func_signature= ', '.join (['L4_ThreadId_t _caller'] + ['%s %s%s%s' % (param['typename'], param['c_impl_indirection'], param['indirection'], param['name']) for param in function.get_call_params()] + ['idl4_server_environment *env'])-*/
/* Prototype for the function which implements this service. */\n
/*-?function.get_return_type()-*/ /*-?function_implementation_name-*/(/*-?func_signature-*/);\n
\n
/*-func_paramlist = []
for item in function.get_params():
	if item['direction'] == 'return':
		continue # Ignore return parameters
	dereference = item['c_impl_indirection'].replace('*', '&')
	if 'priv' in item['flags']:
		param_name = '(buf->ibuf.raw[0] >> 16) & 7'
	else:
		param_name = '%s%s' % (dereference, item['name'])
	func_paramlist.append(param_name)
func_paramlist.append('&env')

seen_mr = {}
def seen(mr_num):
	global seen_mr
	if mr_num in seen_mr:
		return '|='
	else:
		seen_mr[mr_num] = True
		return '='

def bitmask(bits):
	return '0x%x' % ( (1L << bits) - 1)

-*/

/*-FUNCTION_NAME = function.get_name().upper() -*/
long service_/*-?function.get_name()-*/(L4_ThreadId_t _caller, idl4_msgbuf_t *buf);\n
\n
#define IDL4_PUBLISH_/*-?FUNCTION_NAME-*/(_func) \\n
long service_/*-?function.get_name()-*/(L4_ThreadId_t _caller, idl4_msgbuf_t *buf) { \\n
/*LOOP item = function.get_params(ignore = 'priv')*/
/*-if not(item['typename'] == 'void' and not item['indirection'])*/
	/*-?item['typename']-*/ /*-?item['indirection']-*//*-?item['name']-*/;\\n
/*fi-*/
/*ENDLOOP*/
	idl4_server_environment env;\\n
	env._action = 0;
\\n
	/* unmarshal */\\n
/*LOOP grid_dict = function.grid_parameters(['in', 'inout'], start = unmarshal_start)*/
/*-param_name = function.param_name_from_asts(grid_dict['asts'], client_side = False)-*/
/*-param_type = function.param_type_from_asts(grid_dict['asts'])-*/
/*-
if grid_dict['param_cast']:
	param_type_noindirect = grid_dict['param_cast']
else:
	param_type_noindirect = function.param_type_from_asts(grid_dict['asts'], indirects = False)
-*/
	/*-?param_name-*/ /*-?seen(param_name)-*/ 
(/*-?param_type-*/)
(((/*-?param_type_noindirect-*/)(buf->ibuf.raw[/*-?grid_dict['box_number']-*/] 
>> /*-?grid_dict['start_of_box']-*/) 
& /*-?bitmask(grid_dict['bits_in_box'])-*/) 
<< /*-?grid_dict['start_of_parameter']-*/);\\n
/*ENDLOOP*/
\\n
	/*-if function.get_return_type() != 'void'*/__return = /*fi-*/
	_func(/*-?', '.join(['_caller'] + func_paramlist)-*/);\\n
\\n
	if (env._action == 0) {\\n
		/* marshal */\\n
/*-mr_num = 0-*/
/*LOOP grid_dict = function.grid_parameters(['inout', 'out', 'return'], start = 1)*/
/*-param_name = function.param_name_from_asts(grid_dict['asts'], client_side = False)-*/
/*-mr_num = grid_dict['box_number']-*/
		buf->obuf.raw[/*-?mr_num-*/] /*-?seen(mr_num)-*/ 
(((
/*-if grid_dict['param_cast']*/(/*-?grid_dict['param_cast']-*/)/*fi-*/
(/*-?param_name-*/) 
>> /*-?grid_dict['start_of_parameter']-*/) 
& /*-?bitmask(grid_dict['bits_in_box'])-*/) 
<< /*-?grid_dict['start_of_box'] -*/);\\n
/*ENDLOOP*/
\\n
		/* Return function fpages + number of return MRs used */\\n
		buf->obuf.raw[0] = (/*-?2 * function.get_fpages_count()-*/ << 6) + /*-?mr_num + 1-*/;\\n
/*-if function.get_fpages_count() != 0*/
		/* Subtract fpages count */\\n
		buf->obuf.raw[0] -= (/*-?function.get_fpages_count()-*/ * sizeof(idl4_mapitem));\\n
/*fi-*/
		return buf->obuf.raw[0];\\n
	}\\n
	return -1;\\n
}\n
\n
\n
