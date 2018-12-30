#ifndef __idl4_glue_v2_ia32_runtime_h__
#define __idl4_glue_v2_ia32_runtime_h__

#include IDL4_INC_ARCH(helpers.h)
#include IDL4_INC_API(interface.h)

#define IDL4_INLINE inline

#define idl4_process_request(ftable, fnr, reply, message, partner, w0, w1, w2, dummy) \
  asm volatile (                                                                \
  		 "xchg	%%esi, %%eax		\n\t"				\
                 "lea 	-8+%1, %%ecx 		\n\t"                           \
                 "movl	%%esp, 0(%%ecx) 	\n\t"                           \
                 "movl	%%ebp, 4(%%ecx)		\n\t"				\
                 "movl	%%ecx, %%esp 		\n\t"                           \
                 "movl	%%edx, %%ecx		\n\t"				\
                 "movl	%%edi, %%edx		\n\t"				\
                 :                                                              \
                 : "b" (fnr), "m" ((message).rcv_window), "d" (w0),	 	\
                   "S" ((partner).lh.low), "a" (ftable),  			\
                   "D" ((partner).lh.high)					\
                 : "memory"							\
              );                                                                \
asm volatile (                                                                  \
                 "call	*(%%esi,%%ebx,4)	\n\t"                           \
                 "mov	4(%%esp), %%ebp		\n\t"				\
                 "mov	0(%%esp), %%esp 	\n\t"                           \
                 : "=a" (reply), "=d" (w1), "=S" ((partner).lh.low),		\
                   "=b" (w0), "=D" ((partner).lh.high), "=c" (dummy)   		\
                 :                                                              \
                 : "cc"								\
               );

#define idl4_reply_and_wait(reply, buffer, partner, msgdope, fnr, w0, w1, w2, dummy) \
  asm volatile ( 								\
                 "\n\t" 							\
                 :								\
                 : "D" (partner.lh.high), "S" (partner.lh.low), 		\
                   "b" (w0), "d" (w1)						\
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
                   "=b" (fnr), "=d" (w0), "=D" (partner.lh.high), 		\
                   "=S" (partner.lh.low)					\
                 : "m" ((buffer).rcv_window), "a" (reply)			\
                 : "memory", "cc"                                               \
               );

#endif /* !defined(__idl4_glue_v2_ia32_runtime_h__) */
