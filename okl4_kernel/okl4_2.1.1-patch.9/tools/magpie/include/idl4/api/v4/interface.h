#ifndef __idl4_api_v4_interface_h__
#define __idl4_api_v4_interface_h__

#include <l4/space.h>
#include <l4/ipc.h>

#define idl4_threads_equal(a, b) ((a).raw == (b).raw)
#define idl4_nilthread (L4_nilthread)

#define IDL4_FID_BITS 6
#define IDL4_IID_BITS 10

/* 1.17 The CORBA_Object type is an opaque type
   Services are identified by the server's thread ID */

typedef L4_ThreadId_t CORBA_Object;

typedef int CORBA_exception_type;

typedef struct {
  int _action;
  void *_data;
} idl4_server_environment;

typedef struct CORBA_Environment {
  CORBA_exception_type _major : 8;
  CORBA_exception_type _minor : 24;
  void *_data;
  L4_Acceptor_t _rcv_window;
} CORBA_Environment;

#define idl4_default_environment ((CORBA_Environment){ CORBA_NO_EXCEPTION, 0, NULL, L4_Timeouts(L4_Never, L4_Never), L4_UntypedWordsAcceptor })

static inline void CORBA_exception_set(idl4_server_environment *ev, CORBA_exception_type minor, void *param)

{
  if (param)
    ev->_data = param;
  ev->_action = CORBA_USER_EXCEPTION + ((minor)<<8);
}

static inline CORBA_exception_type CORBA_exception_id(CORBA_Environment *ev)

{
  return ev->_minor;
}

static inline void *CORBA_exception_value(CORBA_Environment *ev)

{
  return ev->_data;
}

static inline void CORBA_exception_free(CORBA_Environment *ev)

{
  if (ev->_data != NULL)
    {
#if (IDL4_NEED_MALLOC == 1)
      CORBA_free(ev->_data);
#endif
      ev->_data = NULL;
    }
  ev->_major = CORBA_NO_EXCEPTION;
  ev->_minor = 0;
}

static inline void idl4_set_rcv_window(CORBA_Environment *ev, L4_Fpage_t rcv_window)

{
  ev->_rcv_window.raw = rcv_window.raw;
}

static inline void idl4_set_no_response(idl4_server_environment *ev)

{
  ev->_action = IDL4_NO_RESPONSE;
}

#define IDL4_PERM_READ 		4
#define IDL4_PERM_WRITE 	2
#define IDL4_PERM_EXECUTE 	1
#define IDL4_PERM_FULL          7

#define IDL4_MODE_MAP		0
#define IDL4_MODE_GRANT		2

typedef struct {
                 unsigned long base;
                 unsigned long fpage;
               } idl4_fpage_t;

typedef struct {
                 L4_Msg_t ibuf;
                 L4_Msg_t obuf;
                 unsigned long rbuf[34];
               } idl4_msgbuf_t;

typedef idl4_fpage_t idl4_mapitem;

typedef struct {
                 unsigned long len;
                 void *ptr;
               } idl4_stringitem;

typedef struct {
                 void *ptr;
                 unsigned long len;
               } idl4_inverse_stringitem;

static inline void idl4_fpage_set_base(idl4_fpage_t *fpage, unsigned long base)

{
  fpage->base = (fpage->base & 2) + (base & (~0x3FFul)) + 8;
}

static inline unsigned idl4_fpage_get_base(idl4_fpage_t fpage)

{
  return fpage.base & (~0x3FFul);
}

static inline void idl4_fpage_set_page(idl4_fpage_t *fpage, L4_Fpage_t p)

{
  fpage->fpage = (fpage->fpage&7) + (p.raw&(~3u));
}

static inline L4_Fpage_t idl4_fpage_get_page(idl4_fpage_t fpage)

{
  L4_Fpage_t result;
  result.raw = fpage.fpage&(~3u);
  return result;
}

static inline void idl4_fpage_set_permissions(idl4_fpage_t *fpage, unsigned permissions)

{
  fpage->fpage = (fpage->fpage&(~7u)) + (permissions&7);
}

static inline unsigned idl4_fpage_get_permissions(idl4_fpage_t fpage)

{
  return fpage.fpage & 7;
}

static inline void idl4_fpage_set_mode(idl4_fpage_t *fpage, unsigned mode)

{
  fpage->base = (fpage->base&(~2ul)) + (mode&2);
}

static inline unsigned idl4_fpage_get_mode(idl4_fpage_t fpage)

{
  return fpage.base&2;
}

static inline int idl4_is_kernel_message(L4_MsgTag_t msgtag)

{
  return ((msgtag.X.label&0xFF00) == 0xFF00);
}

static inline unsigned idl4_get_kernel_message_id(L4_MsgTag_t msgtag)

{
  return ((unsigned)-(((int)msgtag.X.label)>>4));
}

static inline unsigned idl4_get_function_id(L4_MsgTag_t *msgtag)

{
  return (msgtag->X.label & ((1<<IDL4_FID_BITS)-1));
}

static inline unsigned idl4_get_interface_id(L4_MsgTag_t *msgtag)

{
  return ((msgtag->X.label>>IDL4_FID_BITS) & ((1<<IDL4_IID_BITS)-1));
}

static inline unsigned idl4_is_error(L4_MsgTag_t *msgtag)

{
  return (msgtag->X.flags & 8);
}

#endif /* __idl4_api_v4_interface_h__ */
