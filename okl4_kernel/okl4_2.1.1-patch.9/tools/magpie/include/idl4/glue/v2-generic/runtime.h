#ifndef __idl4_glue_v2_generic_runtime_h__
#define __idl4_glue_v2_generic_runtime_h__

#include IDL4_INC_ARCH(helpers.h)
#include IDL4_INC_API(interface.h)

#define IDL4_INLINE inline

#define IDL4_IPC_REG_MSG      ((void*)0)
#define IDL4_IPC_REG_OPEN     ((void*)1)
#define IDL4_IPC_REG_FPAGE    ((void*)2)

extern inline void idl4_internal_set_exception(CORBA_Environment *env, l4_msgdope_t msgdope, int resultCode)

{
  if (msgdope.md.error_code!=0)
    {
      env->_major = CORBA_SYSTEM_EXCEPTION;
      env->_minor = msgdope.md.error_code;
    } else
  if (resultCode)
    {
      env->_major = CORBA_USER_EXCEPTION;
      env->_minor = (resultCode>>8);
    } else {
             env->_major = CORBA_NO_EXCEPTION;
             env->_minor = 0;
           }
}

extern inline int idl4_process_request(l4_threadid_t partner, void *msgbuf, int *w0, int *w1, int *w2, int (*func)(l4_threadid_t, void*, int*, int*))

{
  return func(partner, msgbuf, w0, w2);
}

extern inline l4_msgdope_t idl4_reply_and_wait(l4_threadid_t *partner, void *msgbuf, int *w0, int *w1, int *w2, int send_desc)

{
  l4_threadid_t source = L4_NIL_ID;
  l4_msgdope_t result;

  l4_i386_ipc_reply_and_wait(*partner, (void*)send_desc, *w0, *w2,
    &source, msgbuf, w0, w2, L4_IPC_TIMEOUT(0, 1, 0, 0, 0, 0),
    &result
  );
  
  *partner = source;
  
  return result;
}

#endif /* !defined(__idl4_glue_v2_generic_runtime_h__) */
