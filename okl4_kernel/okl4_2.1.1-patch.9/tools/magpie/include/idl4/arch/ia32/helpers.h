#ifndef __idl4_arch_ia32_helpers_h__
#define __idl4_arch_ia32_helpers_h__

typedef unsigned int idl4_size_t;

static inline int idl4_strlen1(const char *str)

{
  int length = 1;
  while (*(str++)) length++;
  return length;
}

static inline int idl4_strlen2(const short *str)

{
  int length = 1;
  while (*(str++)) length++;
  return length;
}

static inline void idl4_memcpy(void *dest, const void *src, int numDwords)

{
  int dummy;
  
  asm volatile (
                 "cld			\n\t"
                 "rep	movsl		\n\t"
                 : "=D" (dummy), "=S" (dummy), "=c" (dummy)
                 : "D" (dest), "S" (src), "c" (numDwords)
                 : "memory"
               );
}

#if (IDL4_OMIT_FRAME_POINTER == 1)
#else
# if defined(IDL4_OMIT_FRAME_POINTER) && (IDL4_OMIT_FRAME_POINTER == 0)
# else
# error IDL4_OMIT_FRAME_POINTER must be set to 0 or 1
# endif
#endif

#endif /* __idl4_arch_ia32_helpers_h__ */
