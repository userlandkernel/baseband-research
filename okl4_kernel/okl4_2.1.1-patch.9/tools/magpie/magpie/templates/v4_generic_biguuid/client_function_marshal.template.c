	/*LOOP function.get_params_in()*/
		/*-TYPECAST = LOOPITEM['typename'] + LOOPITEM['c_impl_indirection'] + LOOPITEM['indirection'] -*/
		/*-DEREF = LOOPITEM['c_impl_indirection']-*/
		_pack._in./*-?LOOPITEM['name']-*/ = /*-?DEREF-*/(/*-?TYPECAST-*/)/*-?LOOPITEM['name']-*/;
	/*ENDLOOP*/
	/*-if not function.get_is_pagefault()*/_pack._in._uuid = /*-?interface.get_uuid()-*/;/*fi-*/
