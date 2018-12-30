/*-template_version = 2 -*/
/*-#Backjump:
#	special = 2 * (number of fpages in output)
#	num_untyped_words = number of untyped words in output not including
#		msgtag: this means we need to know the number of words per type. 
#	msgtag = (special << 6) 

# The meat of this function is in L4_MsgLoad and L4_Send_Nonblocking
# L4_Send_Nonblocking(to)
# is equivalent to
# L4_Ipc(to, L4_nilthread, L4_ZeroTime.raw << 16, 0);
# In registers this is
# r0 = to
# r1 = L4_nilthread
# r2 = (L4_ZeroTime.raw) << 16
# ... and we do not copy r0 to a variable on the way out because "from" is 0
#
-*/
#define IDL4_PUBLISH_/*-?function.func_name-*/ IDL4_PUBLISH_/*-?FUNC_NAME-*/
static inline void /*-?function.func_name-*/_reply(CORBA_Object _client/*LOOP _out_params*/, /*-?LOOPITEM['typename']-*/ * /*-?LOOPITEM['name']-*/ /*ENDLOOP*/ /*-if function.return_type != 'void'*/, /*-?function.return_type-*/ __retval /*fi-*/)
{
	struct _reply_buffer {
		struct {
			long _msgtag;
			/*LOOP _out_params*/ /*-?'%s %s%s' % (_struct_out_magic.get(LOOPITEM['typename'], LOOPITEM['typename']), LOOPITEM['indirection'], LOOPITEM['name'])-*/;
			/*ENDLOOP*/
			/*-if function.return_type != 'void'*//*-?function.return_type-*/ __retval;/*fi-*/
		} _out;
	} _buf;
	struct _reply_buffer *_par = &_buf;

	/* Marshal reply */
	/*LOOP _out_params*/_par->_out./*-?LOOPITEM['name']-*/ = * /*-?LOOPITEM['name']-*/;
	/*ENDLOOP*/
	/*-if function.return_type != 'void'*/_par->_out.__retval = __retval;/*fi-*/
	
	/* send message */

	_buf._out._msgtag = /*-?function.num_untyped_output_words-*/ + (/*-?2 * function.fpages_count-*/ << 6);
	L4_MsgLoad((L4_Msg_t *)((void *)_par));
	L4_Send_Nonblocking(_client);
}
