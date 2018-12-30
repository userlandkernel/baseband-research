#ifndef __idl4_glue_v4_generic_msgbuf_h__
#define __idl4_glue_v4_generic_msgbuf_h__

static inline void idl4_set_counter(L4_Word_t value)

{
  /* Currently, UserDefinedHandle is used to store the dirty pointer. 
     In the future, this should be replaced by a reserved TCR. */
  
  L4_Set_UserDefinedHandle(value);
}

static inline void idl4_msgbuf_sync(idl4_msgbuf_t *msgbuf)

{
}

static inline void idl4_set_counter_minimum(unsigned value)

{
  unsigned oldValue = L4_UserDefinedHandle();
  
  if (oldValue<value)
    L4_Set_UserDefinedHandle(value);
}

static inline void idl4_msgbuf_init(idl4_msgbuf_t *msgbuf)

{
  msgbuf->rbuf[32] = 0;
  idl4_set_counter(1);
}

static inline void idl4_msgbuf_set_rcv_window(idl4_msgbuf_t *msgbuf, L4_Fpage_t wnd)

{
  msgbuf->rbuf[32] = (msgbuf->rbuf[32]&1) + (wnd.raw & 0xFFFFFFF0u);
  idl4_set_counter_minimum(1);
}

static inline void idl4_msgbuf_add_buffer(idl4_msgbuf_t *msgbuf, void *buf, unsigned len)

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

#endif /* !defined(__idl4_glue_v4_generic_msgbuf_h__) */
