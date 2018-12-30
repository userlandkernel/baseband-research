/*-run(templates.get('clientservice_function_params_maker'))-*/
typedef union {
	struct {
		long _msgtag;
		/*LOOP msg_params_in*//*-?'%s %s' % (LOOPITEM[0], LOOPITEM[1])-*/;
		/*ENDLOOP*/
	} _in;
	struct {
		long _spacer[__L4_NUM_MRS];
		long _msgtag;
		/*LOOP msg_params_out*//*-?'%s %s' % (LOOPITEM[0], LOOPITEM[1])-*/;
		/*ENDLOOP*/
	} _out;
	struct {
		long _spacer[__L4_NUM_MRS*2];
		idl4_inverse_stringitem _str[16];
		long _acceptor;
	} _buf;
} _param_/*-?function.get_name()-*/;

long service_/*-?function.get_name()-*/(L4_ThreadId_t _caller, _param_/*-?function.get_name()-*/ *_par);
/*-params = ', '.join (['CORBA_Object _caller'] + ['%s %s%s%s' % (param['typename'], param['c_impl_indirection'], param['indirection'], param['name']) for param in function.get_call_params()] + ['idl4_server_environment *_env'])-*/
inline /*-?function.get_return_type()-*/ /*-?function.get_name()-*/_implementation (/*-?params-*/);

/*-FUNCTION_NAME = function.get_name().upper() -*/
/* Number of output words is the size of the structure minus padding and message tag. */
/*-if function.get_fpages_count() == 0*/
#define _MAGPIE_/*-?FUNCTION_NAME-*/_OUTPUT_BYTES (sizeof(_par->_out) - (sizeof(long) * __L4_NUM_MRS) - (sizeof(long)))
/*fi-*/
/*-if function.get_fpages_count() > 0*//* Fpages (map items) also count. */
#define _MAGPIE_/*-?FUNCTION_NAME-*/_OUTPUT_BYTES (sizeof(_par->_out) - (sizeof(long) * __L4_NUM_MRS) - (sizeof(long)) - (/*-?function.get_fpages_count()-*/ * sizeof(idl4_mapitem)))
/*fi-*/

#define IDL4_PUBLISH_/*-?FUNCTION_NAME-*/(_func) long service_/*-?function.get_name()-*/(L4_ThreadId_t _caller, _param_/*-?function.get_name()-*/ *_par) { \
	idl4_server_environment _env; \
	_env._action = 0;\
\
	/* invoke service */ \/*-_func_paramlist = []
for item in function.get_params():
	if item['direction'] == 'return':
		continue # Ignore return parameters
	prefix = {None: '', 'in': '_par->_in.', 'inout': '_par->_in.', 'out': '_par->_out.'}[item['direction']]
	dereference = item['c_impl_indirection'].replace('*', '&')
	if 'priv' in item['flags']:
		param_name = '(_par->_in._msgtag >> 16) & 7'
	else:
		param_name = '%s%s%s' % (dereference, prefix, item['name'])
	_func_paramlist.append(param_name)
_func_paramlist.append('&_env')
-*/
	/*-if function.get_return_type() != 'void'*/_par->_out.__retval = /*fi-*/_func(/*-?', '.join(['_caller'] + _func_paramlist)-*/); \
\
	if (IDL4_EXPECT_TRUE(_env._action == 0)) {\
		/* Marshal */ \
		/*LOOP msg_params_inout*/_par->_out./*-?LOOPITEM[1]-*/ = _par->_in./*-?LOOPITEM[1]-*/ \
		/*ENDLOOP*//* jump back */ \
		/* the weird numbers: (2 * num fpages) << 6 + (num output words ) */ \
		_par->_out._msgtag = (/*-?2 * function.get_fpages_count()-*/ << 6) + \
			(_MAGPIE_/*-?FUNCTION_NAME-*/_OUTPUT_BYTES / MAGPIE_BYTES_PER_WORD); \
		return (/*-?2 * function.get_fpages_count()-*/ << 6) + \
			(_MAGPIE_/*-?FUNCTION_NAME-*/_OUTPUT_BYTES / MAGPIE_BYTES_PER_WORD); \
	} \
	return -1; \
}

