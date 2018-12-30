#ifndef __idl4_corba_h__
#define __idl4_corba_h__

#define CORBA_NO_EXCEPTION 	0
#define CORBA_SYSTEM_EXCEPTION 	1
#define CORBA_USER_EXCEPTION 	2

/* 2.3 TypeCodes
   We currently do not support the any type */

typedef int CORBA_TypeCode;

/* 1.7 Implementations are responsible for providing typedefs ... */

typedef short CORBA_short;
typedef int CORBA_long;
typedef long long CORBA_long_long;
typedef unsigned short CORBA_unsigned_short;
typedef unsigned int CORBA_unsigned_long;
typedef unsigned long long CORBA_unsigned_long_long;
typedef float CORBA_float;
typedef double CORBA_double;
typedef long double CORBA_long_double;
typedef unsigned char CORBA_unsigned_char;
typedef char CORBA_char;
typedef short CORBA_wchar;
typedef unsigned char CORBA_boolean;
typedef unsigned char CORBA_octet;
typedef struct CORBA_any { CORBA_TypeCode _type;void *_value; } CORBA_any;
typedef unsigned int CORBA_enum; /* always 32bit, even on 64bit platforms */

#if defined(IDL4_PROVIDE_MALLOC)
#if defined(__cplusplus)
extern "C" {
#endif
CORBA_wchar *CORBA_wstring_alloc(unsigned size);
CORBA_char *CORBA_string_alloc(unsigned size);
CORBA_float *CORBA_float_alloc(unsigned size);
CORBA_double *CORBA_double_alloc(unsigned size);
CORBA_long_double *CORBA_long_double_alloc(unsigned size);
void *CORBA_alloc(unsigned size);
void CORBA_free(void *what);
#if defined(__cplusplus)
}
#endif
#endif

#endif /* __idl4_corba_h__ */
