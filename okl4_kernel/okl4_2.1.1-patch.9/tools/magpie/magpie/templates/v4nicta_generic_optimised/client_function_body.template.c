/*-run(templates.get('clientservice_function_params_maker'))-*/
/*-
if function.get_is_pagefault():
        msgtag_special = '+(%s & 7)' % (function.get_priv_param_name())
else:
        msgtag_special = ''
-*/

	L4_MsgTag_t _result;
	L4_MsgTag_t _msgtag;

	/*-if function.get_is_pagefault()*/L4_Accept(_env->_rcv_window);/*fi-*/
	_msgtag = ((sizeof(_pack._in) - sizeof(long)) / MAGPIE_BYTES_PER_WORD) + ((/*-?function.get_defable_name()-*/_CALL_ID/*-?msgtag_special-*/) << 16);
	/*LOOP param=function.get_params_in()*/
	/*-?param['type'].the('info').get_attribute('size')[0]-*/
	/*ENDLOOP*/

Client body
