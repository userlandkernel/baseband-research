!!explicit_linebreaks isolated_scopes
/*-if function.get_is_pagefault()*/
	/* Page-fault handler */\n
	long _exception;\n
	L4_MsgTag_t _result;\n
	L4_Word_t _asynch_mask;\n
\n
	/* NB: This union is declared volatile to work around a bug in gcc 3.3.3 */\n
	volatile union _buf {\n
		struct {\n
			long _msgtag;\n
			signed int addr;\n
			signed int ip;\n
		} _in;\n
		struct {\n
			long _msgtag;\n
			idl4_mapitem fp;\n
		} _out;\n
	} _pack;\n
	/* marshal */\n
	L4_Accept(_env->_rcv_window);\n
	_pack._in.addr = (int)addr;\n
	_pack._in.ip = (int)ip;\n
\n
	/* invoke IPC: NB we subtract the size of _msgtag. */\n
	_pack._in._msgtag = ((sizeof(_pack._in) - sizeof(long)) / MAGPIE_BYTES_PER_WORD) + ((/*-?function.get_defable_name()-*/_CALL_ID+(priv & 7)) << 16);\n
	L4_MsgLoad((L4_Msg_t *)((void *)&_pack._in));\n
	L4_Accept(_env->_rcv_window);\n
	_asynch_mask = L4_Get_NotifyMask();\n
	L4_Set_NotifyMask(0);\n
\n
	_result = L4_Call(_service);\n
	L4_Set_NotifyMask(_asynch_mask);\n
\n
	L4_MsgStore(_result, (L4_Msg_t *)((void *)&_pack));\n
	L4_Accept(L4_UntypedWordsAcceptor);\n
\n
	_exception = L4_Label(_result);\n
\n
	if (IDL4_EXPECT_TRUE((L4_IpcSucceeded(_result)) && !_exception)) {\n
		*fp = _pack._out.fp;\n
	}\n
\n
	if (_env != 0) {\n
		if (!L4_IpcSucceeded(_result)) {\n
			*(L4_Word_t *)_env = CORBA_SYSTEM_EXCEPTION + (L4_ErrorCode() << 8);\n
		} else {\n
			*(L4_Word_t *)_env = _exception;\n
		}\n
	}\n
/*fi-*/
/*-if not function.get_is_pagefault()*/
	/*- # Function parameters -*/\n
	long _exception;\n
	L4_MsgTag_t __result; /* Result of L4_Call */ \n
	byte *mr = (void *)L4_MRStart(); /* Beginning of message registers */ \n
/*-if function.is_nonvoid()*/
	/*-?function.get_return_type()-*/ __return;\n
/*fi-*/
/*-if function.has_varlength_params_in()*/
	byte *mr_varptr = mr; /* Variable-length param buffer pointer */\n
/*fi-*/
/*-run(templates.get('client_function_body_pre_ipc_defs'))-*/
	\n


	/* Set the interface ID  - second word*/\n
	*((L4_Word_t *)(void *)(&mr[MAGPIE_BYTES_PER_WORD])) = /*-?interface.get_uuid()-*/;\n
	\n


	/* Marshal */\n
/*LOOP cmd, args = function.marshal('in', startword = 2)*/
	/*-run(templates.get(cmd), args = args)-*/
/*ENDLOOP*/

	/* Set the message tag = function number, number of message registers used.*/\n
/*-if function.has_varlength_params_in()*/
	/* Number of registers used depends on length of variable-length portion */\n
	*((L4_Word_t *)(void *)(&mr[0])) = ((L4_Word_t)(mr_varptr - mr) / sizeof(L4_Word_t)) + ((/*-?function.get_defable_name()-*/_CALL_ID) << 16);\n
/*fi-*/
/*-if not function.has_varlength_params_in()*/
	*((L4_Word_t *)(void *)(&mr[0])) = 2 + /*-? function.marshal_size_inwords_in()-*/ + ((/*-?function.get_defable_name()-*/_CALL_ID) << 16);\n
/*fi-*/



	/* Do the IPC */\n
/*-run(templates.get('client_function_body_pre_ipc'))-*/
	__result = L4_Call(_service);\n
/*-run(templates.get('client_function_body_post_ipc'))-*/
	\n

	_exception = L4_Label(__result);\n
	\n


	/* Unmarshal */\n
/*LOOP cmd, args = function.unmarshal('out', startword = 1)*/
	/*-run(templates.get(cmd), args = args)-*/
/*ENDLOOP*/
	\n



/*- # Generic clag for exception handling -*/
	if (_env != 0) {\n
		if (!L4_IpcSucceeded(__result)) {\n
			*(L4_Word_t *)_env = CORBA_SYSTEM_EXCEPTION + (L4_ErrorCode() << 8);\n
		} else {\n
			*(L4_Word_t *)_env = _exception;\n
		}\n
	}\n
	\n



	/*-if function.get_return_type() != 'void'*//* Return normally. */\n
	return __return;\n
	/*fi-*/
/*fi-*/
