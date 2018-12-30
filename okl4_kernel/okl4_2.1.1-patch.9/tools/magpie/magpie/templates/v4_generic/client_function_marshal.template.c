!!explicit_linebreaks
/*LOOP function.get_params_in()*/
/*-TYPECAST = LOOPITEM['typename'] + LOOPITEM['c_impl_indirection'] + LOOPITEM['indirection'] -*/
/*-DEREF = LOOPITEM['c_impl_indirection']-*/
/*-if 'marshal' in LOOPITEM['flags']*/
	_pack._in./*-?LOOPITEM['name']-*/ = /*-?DEREF-*/(/*-?TYPECAST-*/)/*-?LOOPITEM['name']-*/;\n
/*fi-*/
/*ENDLOOP*/
