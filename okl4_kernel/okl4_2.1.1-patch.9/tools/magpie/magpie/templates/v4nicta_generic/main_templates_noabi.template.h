!!explicit_linebreaks
#if !defined(/*-?generator.get_ifdefable_filename()-*/)\n
#define /*-?generator.get_ifdefable_filename()-*/\n
/*-run(templates.get('preamble'))-*/

/*-run(templates.get('basic_includes'))-*/

/*-run(templates.get('imports'))-*/

/*-run(templates.get('types_cpp_define_int_constants'))-*/

/* Magpie service main loop. */\n
\n
/*LOOP interface = generator.get_interfaces()*/
/* Interface {*-?interface.get_name()-*} */\n
/*LOOP function = interface.get_functions()*/
\n
/*-
if function.get_is_pagefault():
	unmarshal_start = 1
else:
	unmarshal_start = 2
-*/
/*-run(templates.get('clientservice_function_params_maker'))-*/

/*-function_implementation_name = function.get_name() + '_impl' -*/

/*-func_signature= ', '.join (['L4_ThreadId_t caller'] + ['%s %s' % (param.c_typename, param['name']) for param in function.get_call_params()] + ['idl4_server_environment *env'])-*/
/* Prototype for the function which implements this service. Implement this. */\n
/*-?function.get_return_type()-*/ /*-?function_implementation_name-*/(/*-?func_signature-*/);\n
\n
/* Reply-only function, for callbacks. */\n
void /*-?function.get_name()-*/_reply(CORBA_Object _client/*LOOP function.get_params_out()*/, /*-?LOOPITEM['typename']-*/ /*-?LOOPITEM['indirection']-*/ /*-?LOOPITEM['c_impl_indirection']-*/ /*-?LOOPITEM['name']-*//*ENDLOOP*/ /*-if function.get_return_type() != 'void'*/, /*-?function.get_return_type()-*/ __return /*fi-*/);
/* End of function {*-?function.get_name()-*}*/\n
/*ENDLOOP*/
\n
/*ENDLOOP*/

/*-if generator.get_irq_handler()*/
void /*-?generator.get_irq_handler_name()-*/(L4_ThreadId_t);\n
/*fi-*/

/*-if generator.get_async_handler()*/
void /*-?generator.get_async_handler_name()-*/(L4_Word_t);\n
/*fi-*/

/*-workloop_function = generator.get_workloop_function()-*/
/*-if workloop_function*/
void /*-?workloop_function-*/(void);\n
/*fi-*/

void /*-?generator.get_server_loop_name()-*/(void);\n
#endif //{*-?generator.get_ifdefable_filename()-*}\n
