#ifndef __idl4_api_v2_interface_h__
#define __idl4_api_v2_interface_h__

#define IDL4_FID_BITS 6

#include "l4/sys/types.h"
#include "l4/sys/ipc.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

#define idl4_nil (0xFFFFFFFFu)
#define idl4_nilthread L4_NIL_ID
#define idl4_nilpage ((l4_fpage_t) { fpage: 0 })
#define idl4_raw_fpage(fp) ((fp).fpage)
#define idl4_threads_equal(a, b) (((a).lh.low == (b).lh.low) && ((a).lh.high == (b).lh.high))

typedef struct {
                 int snd_size;
                 void *snd_addr;
                 int rcv_size;
                 void *rcv_addr;
               } idl4_strdope_t;  

typedef struct {
                 unsigned base;
                 unsigned fpage;
               } idl4_fpage_t;

typedef int idl4_mappad_t[2];

typedef int idl4_msgdope_t;

/* 1.17 The CORBA_Object type is an opaque type
   Services are identified by the server's thread ID */

typedef l4_threadid_t CORBA_Object;

/* 1.22 The CORBA_Environment type is partially opaque
   We only support the necessary fields */

typedef int CORBA_exception_type;

typedef struct CORBA_Environment {
  CORBA_exception_type _major : 8;
  CORBA_exception_type _minor : 24;
  void *_data;
  l4_fpage_t _rcv_window;
} CORBA_Environment;

typedef struct {
  int _action;
  void *_data;
} idl4_server_environment;

extern inline void CORBA_exception_set(idl4_server_environment *ev, CORBA_exception_type minor, void *param)

{
  if (param)
    ev->_data = param;
  ev->_action = CORBA_USER_EXCEPTION + ((minor)<<8);
}

extern inline CORBA_exception_type CORBA_exception_id(CORBA_Environment *ev)

{
  return ev->_minor;
}

extern inline void *CORBA_exception_value(CORBA_Environment *ev)

{
  return ev->_data;
}

extern inline void CORBA_exception_free(CORBA_Environment *ev)

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

extern inline void idl4_set_rcv_window(CORBA_Environment *ev, l4_fpage_t rcv_window)

{
  ev->_rcv_window.fpage = rcv_window.fpage;
}

extern inline void idl4_set_no_response(idl4_server_environment *ev)

{
  ev->_action = IDL4_NO_RESPONSE;
}

#define idl4_default_environment { CORBA_NO_EXCEPTION, 0, NULL, L4_IPC_NEVER, { fpage: L4_WHOLE_ADDRESS_SPACE<<2 } }

#define idl4_set_receive_buffer(nr, ptr, len) do { buffer.string[nr].rcv_addr=ptr;buffer.string[nr].rcv_size=len; } while (0)

#endif /* __idl4_api_v2_interface_h__ */
