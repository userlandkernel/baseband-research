/*-
if function.get_is_pagefault():
	msgtag_special = '+(%s & 7)' % (function.get_priv_param_name())
else:
	msgtag_special = ''

input_register_list = ['r_mr%d' % (mapitem['mr']) for mapitem in function.get_register_map(function.directions_in)]
output_register_list = ['r_mr%d' % (mapitem['mr']) for mapitem in function.get_register_map(function.directions_in)]
max_register = max(1, function.get_max_register(function.directions_all))
-*/
/*-if max_register > 4*/
/*-run(templates.get('client_function_body_fallback')) # We don't support > 4 registers yet (should be easy to add, just pack extras into the appropriate MRs...)-*/
/*fi-*/
/*-if max_register <= 4*/
	register L4_ThreadId_t ipc_dest asm("r0") = _service;
	register L4_ThreadId_t ipc_FromSpecifier asm("r1") = _service;
	register L4_Word_t ipc_Timeouts asm("r2") = L4_Timeouts(L4_Never, L4_Never);
	register L4_MsgTag_t client_msgtag asm("r3");
	/*-if max_register >= 1*/register L4_Word_t r_mr1 asm("r4"); /* UUID on input and (sometimes) result on output */ /*fi-*/
	/*-if max_register >= 2*/register L4_Word_t r_mr2 asm("r5"); /*fi-*/
	/*-if max_register >= 3*/register L4_Word_t r_mr3 asm("r6"); /*fi-*/
	/*-if max_register >= 4*/register L4_Word_t r_mr4 asm("r7"); /*fi-*/
	register __L4_Ipc_t r_syscall asm("r12") = __L4_Ipc;
	L4_Word_t _asynch_mask;
	/*-if len(function.get_param_return()['message_registers']) > 1*//*-?function.get_param_return()['typename']-*/ __return; /*fi-*/

#if 0
	/* Self-check code */
	union /*-?function.get_defable_name().upper()-*/_arm_buf {
		struct {
			long _msgtag;
			/*-if not function.get_is_pagefault()*/long _uuid;/*fi-*//*LOOP param = function.get_params_in()*/
			/*-?'%s %s%s;' % (param['typename'], param['indirection'], param['name'])-*//*ENDLOOP*/
		} _in;
		struct {
			long _msgtag;/*LOOP param = function.get_params_out()*/
			/*-?'%s %s%s;' % (param['typename'], param['indirection'], param['name'])-*//*ENDLOOP*/
			/*-if function.get_return_type() != 'void'*//*-?function.get_return_type()-*/ __retval;/*fi-*/
		} _out;
	} _/*-?function.get_defable_name().upper()-*/_arm_pack;
	assert( ((sizeof(_/*-?function.get_defable_name().upper()-*/_arm_pack._in) - sizeof(long)) / MAGPIE_BYTES_PER_WORD) == (/*-?len(function.get_params_in())-*/ + 1));
	assert( ((sizeof(_/*-?function.get_defable_name().upper()-*/_arm_pack._out) - sizeof(long)) /
MAGPIE_BYTES_PER_WORD) == (/*-?function.get_max_register(function.directions_out)-*/) );
	/* End of self-check code */
#endif

	/* Calculate the message tag for parameter 1*/
	client_msgtag.raw = ((/*-?function.get_defable_name().upper()-*/_CALL_ID/*-?msgtag_special-*/) << 16) + /*-?len(function.get_params_in())-*/ + 1; /* Add 1 for UUID */
	/* Copy all inputs into registers, starting with UUID. NB the pass-by-reference case is evil*/ 
	r_mr1 = /*-?interface.get_uuid()-*/;/*LOOP function.get_register_map(function.directions_in)*/
/*-ASSIGN = '='
if LOOPITEM['bits'] != 0:
	ASSIGN = '|='
-*/
	r_mr/*-?LOOPITEM['mr']-*/ /*-?ASSIGN-*/ /*-LOOPITEM['bits']-*/ /*-?LOOPITEM['param']['c_impl_indirection']-*/((L4_Word_t/*-?LOOPITEM['param']['c_impl_indirection']-*/)/*-?LOOPITEM['param']['name']-*/)/*-?function._shift_bits(LOOPITEM['bits']) -*/;
	/*ENDLOOP*/
	/* Call L4 IPC directly using the __L4_SYSCALL macro
	* defined in include/l4/arm/syscalls.h */
	/*-def squish(regmap, intro, prefix):
		result = ', '.join(['%s (%s%s)' % (intro, prefix, item['mr']) for item in regmap])
		if result:
			return ', ' + result
		return ''
	-*//*-def squish_clobber(reglist):
		result = ', '.join(['"r%d"' % item for item in reglist])
		if result:
			return result + ', '
		return ''
	-*/
	/*-run(templates.get('client_function_body_pre_ipc'))-*/
	__asm__ __volatile (
		MAGPIE__L4_SYSCALL 
		: "=r" (ipc_dest), "=r" (client_msgtag)/*-?squish(function.get_registers_map_out(), '"=r"', 'r_mr')-*/,
			"=r" (ipc_FromSpecifier), "=r" (ipc_Timeouts), "=r" (r_syscall)
		: "r" (client_msgtag), "r" (r_mr1)/*-?squish(function.get_registers_map_in(), '"r"', 'r_mr')-*/,
			"r" (r_syscall), "r" (ipc_dest), "r" (ipc_FromSpecifier), "r" (ipc_Timeouts)
		: /*-?squish_clobber(function.get_registers_clobbered())-*/"lr", "r8", "r9", "r10" /* r8, r9, r10 constitute __L4_ALWAYS_CLOBBER */
	);
	/*-run(templates.get('client_function_body_post_ipc'))-*/
	/* Copy results from output registers. */ /*LOOP zip(['1', '2', '3', '4'], function.get_params_out())*/
	/*-?LOOPITEM[1]['c_impl_indirection']-*//*-?LOOPITEM[1]['name']-*/ = (/*-?LOOPITEM[1]['typename']-*/)r_mr/*-?LOOPITEM[0]-*/;
	/*ENDLOOP*/
	/*-if len(function.get_param_return()['message_registers']) == 1*/
	/* Result fits in a single register. */
	return (/*-?function.get_return_type()-*/)r_mr/*-?len(function.get_params_out()) + 1 -*/;
	/*fi-*/
	/*-if len(function.get_param_return()['message_registers']) > 1*/
	/* Return result (which doesn't fit in a single register)*/
	/*LOOP range(len(function.get_param_return()['message_registers']))*/
		*( ((L4_Word_t *)(void *)&__return) + /*-?LOOPITEM-*/) = r_mr/*-?function.get_param_return()['message_registers'][LOOPITEM][0]-*/;
	/*ENDLOOP*/
	return __return;
	/*fi-*/
/*fi-*/
