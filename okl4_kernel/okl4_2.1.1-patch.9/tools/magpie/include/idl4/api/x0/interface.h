#ifndef __idl4_api_x0_interface_h__
#define __idl4_api_x0_interface_h__

#define IDL4_FID_BITS 6

#include <l4/l4.h>

#define idl4_nil (0xFFFFFFFFu)
#define idl4_nilthread ((l4_threadid_t){ raw: 0 })
#define idl4_nilpage ((l4_fpage_t) { raw: 0 })
#define idl4_raw_fpage(fp) ((fp).raw)
#define idl4_threads_equal(a, b) ((a).raw == (b).raw)

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

#define IDL4_PERM_READ 		1
#define IDL4_PERM_WRITE 	2
#define IDL4_PERM_EXECUTE 	4

#define IDL4_MODE_MAP		0
#define IDL4_MODE_GRANT		1

extern inline void idl4_fpage_set_base(idl4_fpage_t *fpage, unsigned base)

{
  fpage->base = base;
}

extern inline unsigned idl4_fpage_get_base(idl4_fpage_t fpage)

{
  return fpage.base;
}

extern inline void idl4_fpage_set_page(idl4_fpage_t *fpage, l4_fpage_t p)

{
  fpage->fpage = (fpage->fpage & 3) + (p.raw & 0xFFFFFFFCu);
}

extern inline l4_fpage_t idl4_fpage_get_page(idl4_fpage_t fpage)

{
  l4_fpage_t result;
  result.raw = fpage.fpage & 0xFFFFFFFCu;
  return result;
}

extern inline void idl4_fpage_set_permissions(idl4_fpage_t *fpage, unsigned permissions)

{
  fpage->fpage = (fpage->fpage & 0xFFFFFFFDu) + (permissions & 2);
}

extern inline unsigned idl4_fpage_get_permissions(idl4_fpage_t fpage)

{
  return (fpage.fpage&2)+IDL4_PERM_READ+IDL4_PERM_EXECUTE;
}

extern inline void idl4_fpage_set_mode(idl4_fpage_t *fpage, unsigned mode)

{
  fpage->fpage = (fpage->fpage & 0xFFFFFFFEu) + (mode & 1);
}

extern inline unsigned idl4_fpage_get_mode(idl4_fpage_t fpage)

{
  return fpage.fpage&1;
}

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

static inline void idl4_set_rcv_window(CORBA_Environment *ev, l4_fpage_t rcv_window)

{
  ev->_rcv_window.raw = rcv_window.raw;
}

static inline void idl4_set_no_response(idl4_server_environment *ev)

{
  ev->_action = IDL4_NO_RESPONSE;
}

#define idl4_default_environment ((CORBA_Environment) { CORBA_NO_EXCEPTION, 0, NULL, L4_IPC_NEVER, { raw: L4_WHOLE_ADDRESS_SPACE<<2 } })

#define idl4_set_receive_buffer(nr, ptr, len) do { buffer.string[nr].rcv_addr=ptr;buffer.string[nr].rcv_size=len; } while (0)

#endif /* defined(__idl4_api_x0_interface_h__) */
