#ifndef __idl4_glue_x0_ia32_runtime_h__
#define __idl4_glue_x0_ia32_runtime_h__

#include IDL4_INC_ARCH(helpers.h)
#include IDL4_INC_API(interface.h)

#define IDL4_INLINE inline

#define IDL4_LIPC_ENTRY 	"lipc"
#define IDL4_UTCB_SIZE		64
#define IDL4_UTCB_BASE		0xb0000000u

// partner ECX, w0 EDX, w1 EAX
#define idl4_process_request(ftable, fnr, reply, message, partner, w0, w1, w2, dummy) \
  asm volatile (                                                                \
                 "xchg  %%eax, %%ebx		\n\t"				\
                 "lea 	-8+%1, %%ecx 		\n\t"                           \
                 "mov	%%esp, 0(%%ecx) 	\n\t"                           \
                 "movl	%%ebp, 4(%%ecx)		\n\t"				\
                 "mov	%%ecx, %%esp 		\n\t"                           \
                 "movl	%%esi, %%ecx		\n\t"				\
                 :                                                              \
                 : "D" (fnr), "m" ((message).rcv_window), "b" (w1), "d" (w0), 	\
                   "S" (partner), "a" (ftable)  				\
                 : "memory"							\
              );                                                                \
asm volatile (                                                                  \
                 "call	*(%%ebx,%%edi,4)	\n\t"                           \
                 "mov	4(%%esp), %%ebp		\n\t"				\
                 "mov	0(%%esp), %%esp 	\n\t"                           \
                 : "=a" (reply), "=D" (w2), "=S" (partner),			\
                   "=b" (w1), "=d" (w0), "=c" (dummy)   			\
                 :                                                              \
                 : "cc"                                               \
               );

#if (IDL4_USE_FASTCALL == 0)

#define idl4_reply_and_wait(reply, buffer, partner, msgdope, fnr, w0, w1, w2, dummy) \
  asm volatile ( 								\
                 "\n\t" 							\
                 :								\
                 : "D" (w2), "b" (w1), "d" (w0), "S" (partner)			\
               );         							\
  asm volatile (                                                                \
  		 "movl	 %%ebp, %%ecx		\n\t"				\
                 "lea    %6, %%ebp       	\n\t"  				\
  		 "push	 %%ecx			\n\t"				\
                 "mov    $0x10, %%ecx       	\n\t"     			\
                 "or     $1, %%ebp          	\n\t"        			\
                 "int    $0x30	            	\n\t"            		\
                 "pop	 %%ebp			\n\t"				\
                                                                                \
                 : "=a" (msgdope), "=c" (dummy),                		\
                   "=b" (w1), "=d" (w0), "=D" (fnr), "=S" (partner)		\
                 : "m" ((buffer).rcv_window), "a" (reply)			\
                 : "memory", "cc"                                               \
               );

#else /* IDL4_USE_FASTCALL == 1 */

#define idl4_reply_and_wait(reply, buffer, partner, msgdope, fnr, w0, w1, w2, dummy) \
  asm volatile ( 								\
                 "\n\t" 							\
                 :								\
                 : "D" (w2), "b" (w1), "d" (w0), "S" (partner)			\
               );         							\
  asm volatile (                						\
                 "lea    1+%6, %%ecx		\n\t"				\
                 "push   %%ebp			\n\t"				\
                 "push   $0x10			\n\t"				\
                 "push   %%ecx			\n\t"				\
                 "push   $0x1b			\n\t"				\
                 "push   $0f			\n\t"				\
                 "movl   %%esp, %%ecx           \n\t"				\
                 "sysenter			\n\t"				\
                 "movl   %%ebp, %%edx		\n\t"				\
                 "0:				\n\t"				\
                 "pop    %%ebp			\n\t"				\
                 : "=a" (msgdope), "=c" (dummy),                		\
                   "=b" (w1), "=d" (w0), "=D" (fnr), "=S" (partner)		\
                 : "m" ((buffer).rcv_window), "a" (reply)			\
                 : "memory", "cc"                                               \
               );

#endif /* IDL4_USE_FASTCALL == 1 */

extern inline l4_threadid_t idl4_gid2lid(l4_threadid_t t)

{
  l4_threadid_t n;
  n.raw = IDL4_UTCB_BASE + (t.id.thread * IDL4_UTCB_SIZE);
  return n;
}

#define idl4_local_process_request(ftable, fnr, partner, w0, w1, w2, dummy)  	\
  asm volatile (                                                                \
                 "call	*" #ftable "(,%%edi,4)	\n\t"                           \
                 : "=D" (w2), "=b" (w1), "=d" (w0), "=S" (partner), 		\
                   "=a" (dummy), "=c" (dummy)					\
                 : "D" (fnr), "c" (w1), "d" (w0), "a" (partner)			\
                 : "cc"                                        	\
               );

// assume __attribute__ ((regparm (3))), int partner, int w0, int w1 EAX EDX ECX

#define idl4_local_reply_and_wait(partner, fnr, w0, w1, w2, dummy) \
  asm volatile ( \
  		 "push	%%ebp			\n\t"				\
                 "xorl	%%ebp, %%ebp		\n\t"				\
                 "movl	%%gs:(0), %%eax		\n\t"				\
                 "not	%%ebp			\n\t"				\
  		 "call	" IDL4_LIPC_ENTRY      "\n\t"				\
                 "pop	%%ebp			\n\t"				\
                 : "=D" (fnr), "=b" (w1), "=d" (w0), 				\
                   "=a" (partner), "=S" (dummy)					\
                 : "D" (w2), "b" (w1), "d" (w0), "S" (partner)			\
                 : "ecx", "cc"							\
               );

#endif /* !defined(__idl4_glue_x0_ia32_runtime_h__) */
