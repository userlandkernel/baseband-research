#ifndef __idl4_glue_v4_generic_runtime_h__
#define __idl4_glue_v4_generic_runtime_h__

#include IDL4_INC_ARCH(helpers.h)
#include IDL4_INC_API(interface.h)
#include IDL4_INC_GLUE(msgbuf.h)

#include <l4/ipc.h>
#include <l4/message.h>

#define IDL4_INLINE inline

static inline void
idl4_process_request(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, 
		     idl4_msgbuf_t *msgbuf, long *cnt, void *func)
{
	*cnt = ((int (*)(L4_ThreadId_t, idl4_msgbuf_t*))func)(*partner, msgbuf);
	if (*cnt<0) {
		*partner = L4_nilthread;
		*cnt = 0;
	}
}

static inline void
idl4_reply_and_wait(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, 
		    idl4_msgbuf_t *msgbuf, long *cnt)
{
	L4_MsgTag_t _result;
	
	if (!L4_IsThreadEqual(*partner, L4_nilthread)) {
		L4_MsgLoad((L4_Msg_t*)&msgbuf->obuf);
	}
	_result = L4_ReplyWait(*partner, partner);
	L4_MsgStore(_result, (L4_Msg_t*)msgbuf);
	*msgtag = _result;
}               

/*
static inline void
idl4_reply_and_wait_timeout(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, 
			    idl4_msgbuf_t *msgbuf, long *cnt, L4_Time_t timeout)
{
	L4_MsgTag_t _result;
	
	L4_MsgLoad((L4_Msg_t*)&msgbuf->obuf);
	_result = L4_ReplyWait_Timeout(*partner, timeout, partner);
	L4_MsgStore(_result, (L4_Msg_t*)msgbuf);
	*msgtag = _result;
}*/              

#endif /* !defined(__idl4_glue_v4_generic_runtime_h__) */
