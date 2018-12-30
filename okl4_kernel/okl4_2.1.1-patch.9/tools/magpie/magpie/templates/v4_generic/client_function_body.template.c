/*-run(templates.get('clientservice_function_params_maker'))-*/
/*-
if function.get_is_pagefault():
	msgtag_special = '+(%s & 7)' % (function.get_priv_param_name())
else:
	msgtag_special = ''
-*/
	long _exception;
	L4_MsgTag_t _result;
/*-run(templates.get('client_function_body_pre_ipc_defs'))-*/
	/* NB: This union is declared volatile to work around a bug in gcc 3.3.3 */
#ifdef __cplusplus
	union _buf {
#else
	volatile union _buf {
#endif
		struct {
			long _msgtag;
			/*LOOP msg_params_in*//*-?'%s %s' % (LOOPITEM[0], LOOPITEM[1])-*/;
			/*ENDLOOP*/
		} _in;
		struct {
			long _msgtag;
			/*LOOP msg_params_out*//*-?'%s %s' % (LOOPITEM[0], LOOPITEM[1])-*/;
			/*ENDLOOP*/
		} _out;
	} _pack;
	/* marshal */
	/*-if function.get_is_pagefault()*/L4_Accept(_env->_rcv_window);/*fi-*/
/*-run(templates.get('client_function_marshal'))-*/
	/* invoke IPC: NB we subtract the size of _msgtag. */
	_pack._in._msgtag = ((sizeof(_pack._in) - sizeof(long)) / MAGPIE_BYTES_PER_WORD) + ((/*-?function.get_defable_name()-*/_CALL_ID/*-?msgtag_special-*/) << 16);
	L4_MsgLoad((L4_Msg_t *)((void *)&_pack._in));
	/*-if function.get_is_pagefault()*/L4_Accept(_env->_rcv_window);/*fi-*/
	/*-run(templates.get('client_function_body_pre_ipc'))-*/
	_result = L4_Call(_service);
	/*-run(templates.get('client_function_body_post_ipc'))-*/
	L4_MsgStore(_result, (L4_Msg_t *)((void *)&_pack));
	/*-if function.get_is_pagefault()*/L4_Accept(L4_UntypedWordsAcceptor);/*fi-*/

	_exception = L4_Label(_result);

	/*-if function.get_params_out()*/if (IDL4_EXPECT_TRUE((L4_IpcSucceeded(_result)) && !_exception)) {
		/* unmarshal */
		/*LOOP function.get_params_out()*/
			/*-?LOOPITEM['c_impl_indirection']-*//*-?LOOPITEM['name']-*/ = _pack._out./*-?LOOPITEM['name']-*/;
		/*ENDLOOP*/
	}
	/*fi-*/
	if (_env != 0) {
		if (!L4_IpcSucceeded(_result)) {
			*(L4_Word_t *)_env = CORBA_SYSTEM_EXCEPTION + (L4_ErrorCode() << 8);
		} else {
			*(L4_Word_t *)_env = _exception;
		}
	}

	/*-if function.get_return_type() != 'void'*//* Return normally. */
	return _pack._out.__retval;
	/*fi-*/
