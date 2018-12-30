#ifndef __idl4_glue_v4_alpha_runtime_h__
#define __idl4_glue_v4_alpha_runtime_h__

#include IDL4_INC_ARCH(helpers.h)
#include IDL4_INC_API(interface.h)

#define IDL4_INLINE inline

extern inline void idl4_process_request(L4_ThreadId_t *partner, L4_MsgTag_t __attribute__ ((unused)) *msgtag, idl4_msgbuf_t *msgbuf, long *cnt, int (*func)(L4_ThreadId_t, idl4_msgbuf_t*))

{
  *cnt = func(*partner, msgbuf);
  if (*cnt<0)
    {
      *partner = L4_nilthread;
      *cnt = 0;
    }
}

extern inline void idl4_reply_and_wait(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, idl4_msgbuf_t *msgbuf, long __attribute__ ((unused)) *cnt)

{
  L4_MsgTag_t _result;
  
  L4_MsgLoad((L4_Msg_t*)&msgbuf->obuf);
  _result = L4_ReplyWait(*partner, partner);
  L4_MsgStore(_result, (L4_Msg_t*)msgbuf);
  *msgtag = _result;
}               

extern inline void idl4_set_counter(unsigned value)

{
  /* Currently, UserDefinedHandle is used to store the dirty pointer. 
     In the future, this should be replaced by a reserved TCR. */
  
  L4_Set_UserDefinedHandle(value);
}

extern inline void idl4_msgbuf_sync(idl4_msgbuf_t *msgbuf)
{
}

extern inline void idl4_set_counter_minimum(unsigned value)

{
  unsigned oldValue = L4_UserDefinedHandle();
  
  if (oldValue<value)
    L4_Set_UserDefinedHandle(value);
}
extern inline void idl4_msgbuf_init(idl4_msgbuf_t *msgbuf)

{
  msgbuf->rbuf[32] = 0;
  idl4_set_counter(1);
}

extern inline void idl4_msgbuf_set_rcv_window(idl4_msgbuf_t *msgbuf, L4_Fpage_t wnd)

{
  msgbuf->rbuf[32] = (msgbuf->rbuf[32]&1) + (wnd.raw & 0xFFFFFFF0u);
  idl4_set_counter_minimum(1);
}

extern inline void idl4_msgbuf_add_buffer(idl4_msgbuf_t *msgbuf, void *buf, unsigned len)

{
  int i=32;
 
  while (msgbuf->rbuf[i]&1)
    i -= (i==32) ? 1 : 2;
  
  msgbuf->rbuf[i] |= 1;
  i -= (i==32) ? 1 : 2;
  
  msgbuf->rbuf[i--] = len<<10;
  msgbuf->rbuf[i--] = (unsigned long)buf;
  
  idl4_set_counter_minimum((unsigned)(32-i));
}

#endif /* !defined(__idl4_glue_v4_alpha_runtime_h__) */
