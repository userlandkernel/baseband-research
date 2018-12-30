!!explicit_linebreaks
/*-run(templates.get('preamble'))-*/

/*-run(templates.get('basic_includes'))-*/
#include <stdio.h>\n
\n
/*-run(templates.get('imports'))-*/

/*-run(templates.get('types_cpp_define_int_constants'))-*/
#define L4_REQUEST_MASK		( ~((~0UL) >> ((sizeof (L4_Word_t) * 8) - 20)))\n
#define L4_PAGEFAULT		(-(2UL << 20))\n
/*-if generator.get_word_size() == 32*/
#define L4_IRQ  0xFFF00000\n
#define L4_EXCEPTION 0xFFB00000\n
/*fi-*/
/*-if generator.get_word_size() == 64*/
#define L4_IRQ 0xFFFFFFFFFFF00000\n
#define L4_EXCEPTION 0xFFFFFFFFFFB00000\n
/*fi-*/
/*-if generator.get_word_size() not in (32, 64)*/
#define L4_IRQ (((~(0UL)) << 20))\n
#define L4_EXCEPTION (-(5UL << 20))\n
/*fi-*/
\n
#if defined(__arm) && __ARMCC_VERSION >= 200000 && !defined(__GNUC__)\n
/* rvct doesn't support inline, only __inline. */\n
#define inline __inline\n
#endif\n
\n
/* Magpie service main loop. */\n
\n
/*LOOP interface = generator.get_interfaces()*/
/* Interface {*-?interface.get_name()-*} */\n
/*LOOP function = interface.get_functions()*/
\n
/*-run(templates.get('clientservice_function_params_maker'))-*/

/*-function_implementation_name = function.get_name() + '_impl' -*/

/*-func_signature= ', '.join (['L4_ThreadId_t caller'] + ['%s %s' % (param.c_typename, param['name']) for param in function.get_call_params()] + ['idl4_server_environment *env'])-*/
/* Prototype for the function which implements this service. Implement this. */\n
/*-?function.get_return_type()-*/ /*-?function_implementation_name-*/(/*-?func_signature-*/);\n
\n
/*-func_paramlist = []
for item in function.get_params():
	if item['direction'] == 'return':
		continue # Ignore return parameters
	dereference = item['c_impl_indirection'].replace('*', '&')
	if 'priv' in item['flags']:
		param_name = '(mr[0] >> 16) & 7'
	else:
		param_name = '%s%s' % (dereference, item['name'])
	func_paramlist.append(param_name)
func_paramlist.append('&env')

-*/

/*-FUNCTION_NAME = function.get_name().upper() -*/
\n
/*-if function.get_is_pagefault()*/
/* Pagefault handler */
/*-run(templates.get('service_pagefault'))-*/
/*fi-*/
/*-if not function.get_is_pagefault()*/
static inline int\n
handle_/*-?function.get_name()-*/(L4_ThreadId_t *caller, byte *mr)\n
{\n
/*LOOP item = function.get_params(ignore = 'priv')*/
/*-if not (item['typename'] == 'void' and not item['indirection'])*/
	/*-?item.c_base_typename-*/ /*-?item['indirection']-*/ /*-?item['name']-*/;\n
/*fi-*/
/*ENDLOOP*/
	idl4_server_environment env;\n
/*-if function.has_varlength_params_in()*/
	byte *mr_varptr = mr; /* Variable-length param buffer pointer */\n
/*fi-*/
	env._action = 0;\n
\n
	/* Unmarshal */\n
/*LOOP cmd, args = function.unmarshal('in', startword = 2, client_side = False)*/
	/*-run(templates.get(cmd), args = args)-*/
/*ENDLOOP*/
\n
	/*-if function.get_return_type() != 'void'*/__return = /*fi-*/
/*-?function_implementation_name-*/(/*-?', '.join(['*caller'] + func_paramlist)-*/);\n
\n
	if(env._action == 0) {\n
		/* Marshal */\n
/*LOOP cmd, args = function.marshal('out', startword = 1, client_side = False)*/
		/*-run(templates.get(cmd), args = args)-*/
/*ENDLOOP*/
		/* Return number of return MRs used */\n
/*-if function.has_varlength_params_out()*/
		/* Number of registers used depends on length of variable-length portion */\n
		*((L4_Word_t *)(void *)(&mr[0])) = ((L4_Word_t)(mr_varptr - mr) / sizeof(L4_Word_t));\n
	/*fi-*/
/*-if not function.has_varlength_params_out()*/
		*((L4_Word_t *)(void *)(&mr[0])) = 1 + /*-? function.marshal_size_inwords_out()-*/;\n
/*fi-*/
	} else {\n
		*caller = L4_nilthread;\n
		mr[0] = 0;\n
	}\n
	return 0;\n
}\n
\n
/* End of function {*-?function.get_name()-*}*/\n

/* Reply-only function, for callbacks. */\n
void /*-?function.get_name()-*/_reply(CORBA_Object _client/*LOOP function.get_params_out()*/, /*-?LOOPITEM.c_typename-*/ /*-?LOOPITEM['name']-*//*ENDLOOP*/ /*-if function.get_return_type() != 'void'*/, /*-?function.get_return_type()-*/ __return /*fi-*/);\n
void\n
/*-?function.get_name()-*/_reply(CORBA_Object _client/*LOOP function.get_params_out()*/, /*-?LOOPITEM.c_typename-*/ /*-?LOOPITEM['name']-*//*ENDLOOP*/ /*-if function.get_return_type() != 'void'*/, /*-?function.get_return_type()-*/ __return /*fi-*/)\n
{\n
	byte *mr = (void *)L4_MRStart();\n
	/* Marshal */\n
/*LOOP cmd, args = function.marshal('out', startword = 1)*/
	/*-run(templates.get(cmd), args = args)-*/
/*ENDLOOP*/
	/* Return number of return MRs used */\n
/*-if function.has_varlength_params_out()*/
	/* Number of registers used depends on length of variable-length portion */\n
	*((L4_Word_t *)(void *)(&mr[0])) = ((L4_Word_t)(mr_varptr - mr) / sizeof(L4_Word_t));\n
/*fi-*/
/*-if not function.has_varlength_params_out()*/
	*((L4_Word_t *)(void *)(&mr[0])) = 1 + /*-? function.marshal_size_inwords_out()-*/;\n
/*fi-*/
\n
	/* send message */\n
	L4_MsgLoad((L4_Msg_t *)mr);\n
	L4_Reply(_client);\n
}\n
\n
/*fi-*/
/*ENDLOOP*/
\n
/*ENDLOOP*/

/* Service template helper functions. */\n
static inline unsigned\n
msg_is_error(L4_MsgTag_t msgtag)\n
{\n
	return (msgtag.X.flags & 8);\n
}\n
\n
static inline unsigned\n
get_function_id(L4_MsgTag_t msgtag)\n
{\n
	return (msgtag.X.label & ((1<<IDL4_FID_BITS)-1));\n
}\n
\n
\n
/* Dispatchers for individual interfaces */\n
/*LOOP interface = generator.get_interfaces()*/\n
\n
\n
static inline int\n
dispatch_/*-?interface.get_name()-*/(L4_ThreadId_t *partner, byte *mr)\n
{\n
	L4_MsgTag_t *tag = (void *)mr;\n
	switch(get_function_id(*tag) ){\n
/*LOOP function = interface.get_functions()*/
/*-if not function.get_is_pagefault()*/
		case /*-?function.get_number()-*/:\n
			handle_/*-?function.get_name()-*/(partner, mr);\n
			break;\n
/*fi-*/
/*ENDLOOP*/
	}\n
	return 0;\n
}\n
/*ENDLOOP*/
/*-pagefault_handler = generator.get_pagefault_handler()-*/
/*-irq_handler =   generator.get_irq_handler()-*/
/*-exception_handler =   generator.get_exception_handler()-*/
/*-async_handler = generator.get_async_handler()-*/
/*-workloop_function = generator.get_workloop_function()-*/
/*-if pagefault_handler*/
\n
\n
static inline int\n
dispatch__kernel_pagefault(L4_ThreadId_t *partner, L4_Word_t *mr)\n
{\n
	uintptr_t addr;\n
	uintptr_t ip;\n
	uintptr_t priv;\n
	idl4_server_environment env;\n
	idl4_mapitem fp;\n
\n
	addr = (uintptr_t)mr[1];\n
	ip = (uintptr_t)mr[2];\n
	priv = (mr[0] >> 16) & 7;\n
\n
	/* Handle pagefaults, sent differently to regular messages. */\n
	/* Use V4/N2 encoding. */\n
	/*-?pagefault_handler.get_name()-*/_impl(*partner, addr, ip, priv, &fp, &env);\n
\n
	mr[0] = 2;\n
	mr[1] = fp.base;\n
	mr[2] = fp.fpage;\n
\n
	return mr[0];\n
}\n
/*fi-*/

/*-if irq_handler*/
void /*-?generator.get_irq_handler_name()-*/(L4_ThreadId_t);\n
/*fi-*/

/*-if exception_handler*/
void /*-?generator.get_irq_exception_handler_name()-*/(L4_ThreadId_t, L4_Word_t *);\n
/*fi-*/

/*-if async_handler*/
void /*-?generator.get_async_handler_name()-*/(L4_Word_t);\n
/*fi-*/

/*-if workloop_function*/
void /*-?workloop_function-*/(void);\n
/*fi-*/
\n
\n
/* Service template mainloop. */\n
void /*-?generator.get_server_loop_name()-*/(void);\n
void\n
/*-?generator.get_server_loop_name()-*/(void)\n
{\n
	L4_Word_t *mr = (void *)L4_MRStart();\n
	L4_ThreadId_t partner = L4_nilthread; /* Our initial reply is to the nilthread. */\n
	L4_MsgTag_t tag;\n
\n
	while(1) {\n
/*-if workloop_function*/
		/* Call the server-defined workloop function which runs after every message. */\n
		/*-?workloop_function-*/();\n
/*fi-*/
		/* Send response to previous message, if any, and wait for new mesage.*/\n
		L4_ReplyWait(partner, &partner);\n
		tag = *((L4_MsgTag_t *)(void *)mr);\n
		if (msg_is_error(tag)) {\n
			/* FIXME: nfd - fix error handling */\n
			printf("server: error sending IPC reply\n");\n
			partner = L4_nilthread;\n
/*-if pagefault_handler*/
		} else if ((mr[0] & L4_REQUEST_MASK) == L4_PAGEFAULT) {\n
			dispatch__kernel_pagefault(&partner, mr);\n
/*fi-*/
/*-if async_handler*/
		} else if (partner.raw == 0) {\n
				/*-?generator.get_async_handler_name()-*/(mr[1]);\n
/*fi-*/
/*-if irq_handler*/
		} else if ((mr[0] & L4_REQUEST_MASK) == L4_IRQ) {\n
			/*-?generator.get_irq_handler_name()-*/(partner);\n
/*fi-*/
/*-if exception_handler*/
		} else if ((mr[0] & L4_REQUEST_MASK) == L4_EXCEPTION) {\n
			/*-?generator.get_exception_handler_name()-*/(partner, mr);\n
/*fi-*/
		} else {\n
			/* mr[1] contains the interface UUID - switch on that first. */\n
			switch(mr[1]) {\n
				/*LOOP interface = generator.get_interfaces()*/\n
				case /*-?interface.get_uuid()-*/:\n
					dispatch_/*-?interface.get_name()-*/(&partner, (void *)mr);\n
					break;\n
				/*ENDLOOP*/\n
			}\n
		}\n
	}\n
}\n

