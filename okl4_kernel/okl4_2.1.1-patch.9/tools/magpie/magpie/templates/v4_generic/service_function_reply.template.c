/*-template_version = 2-*/
/*- #Backjump:
#	special = 2 * (number of fpages in output)
#	num_untyped_words = number of untyped words in output not including
#		msgtag: this means we need to know the number of words per type. 
#	msgtag = (special << 6) 
-*/
#define IDL4_PUBLISH_/*-?function.get_name()-*/ IDL4_PUBLISH_/*-?function.get_name().upper()-*/
static inline void /*-?function.get_name()-*/_reply(CORBA_Object _client/*LOOP function.get_params_out()*/, /*-?LOOPITEM['typename']-*/ /*-?LOOPITEM['indirection']-*/ /*-?LOOPITEM['c_impl_indirection']-*/ /*-?LOOPITEM['name']-*//*ENDLOOP*/ /*-if function.get_return_type() != 'void'*/, /*-?function.get_return_type()-*/ __retval /*fi-*/)
{
	struct _reply_buffer {
		struct {
			long _msgtag;
			/*LOOP function.get_params_out()*/
			/*-?'%s %s%s' % (LOOPITEM['typename'], LOOPITEM['indirection'], LOOPITEM['name'])-*/;
			/*ENDLOOP*/
			/*-if function.get_return_type() != 'void'*//*-?function.get_return_type()-*/ __retval;/*fi-*/
		} _out;
	} _buf;
	struct _reply_buffer *_par = &_buf;

	/* Marshal reply */
	/*LOOP function.get_params_out()*/_par->_out./*-?LOOPITEM['name']-*/ = * /*-?LOOPITEM['name']-*/;
	/*ENDLOOP*/
	/*-if function.get_return_type() != 'void'*/_par->_out.__retval = __retval;/*fi-*/
	
	/* send message */
	_buf._out._msgtag = ((sizeof(_buf._out) - sizeof(long)) / MAGPIE_BYTES_PER_WORD) + (/*-?2 * function.get_fpages_count()-*/ << 6);
	L4_MsgLoad((L4_Msg_t *)((void *)_par));
	L4_Reply(_client);
}
