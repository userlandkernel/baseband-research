/*-
function.rename_args({'fpage': 'idl4_fpage_t'})
# The C interface dictates we start with a service pointer and end with environment.
call_param_list = ['%s _service' % (interface.get_name())]
for param in function.get_call_params():
	call_param_list.append('%s %s' % (param.c_typename, param['name']))
call_param_list.append('CORBA_Environment *_env')
-*/
#if !defined(/*-?function.get_ifdef_name()-*/)
#define /*-?function.get_ifdef_name()-*/
/*-run(templates.get('client_function_create_id'))-*/
static inline /*-?function.get_return_type('client')-*/ /*-?function.get_name()-*/ (/*-?', '.join (call_param_list)-*/)
{
/*-run(templates.get('client_function_body'))-*/
}
#endif // !defined(/*-?function.get_ifdef_name()-*/)
