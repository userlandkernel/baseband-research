#ifndef __idl4_glue_v4_generic_runtime_h__
#define __idl4_glue_v4_generic_runtime_h__

#include <idl4biguuid/arch/generic_biguuid/helpers.h>
#include <idl4biguuid/api/v4/interface.h>
#include <idl4biguuid/glue/v4-generic_biguuid/msgbuf.h>

#include <l4/ipc.h>
#include <l4/message.h>

#define IDL4_INLINE inline

typedef long (*idl4_function)(L4_ThreadId_t, idl4_msgbuf_t*);

static inline void
idl4_process_request(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, 
		     idl4_msgbuf_t *msgbuf, long *cnt, 
		     idl4_function func)
{
	*cnt = func(*partner, msgbuf);
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
	L4_MsgStore(_result, &(msgbuf->ibuf));
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
	L4_MsgStore(_result, &(msgbuf->ibuf));
	*msgtag = _result;
}*/              

#endif /* !defined(__idl4_glue_v4_generic_runtime_h__) */
