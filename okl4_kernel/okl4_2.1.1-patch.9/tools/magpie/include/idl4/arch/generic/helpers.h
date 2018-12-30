#ifndef __idl4_arch_generic_helpers_h__
#define __idl4_arch_generic_helpers_h__

#ifndef __idl4_have_size_t__
#define __idl4_have_size_t__

#if defined(CONFIG_ARCH_ALPHA)
typedef unsigned long idl4_size_t;
#else
typedef unsigned int idl4_size_t;
#endif 

#endif

static inline void idl4_memcpy(void *dest, const void *src, int numMwords)
{
	long *dest_long = (long*)dest;
	long *src_long = (long*)src;

	for (;numMwords>0;numMwords--)
		*(dest_long)++ = *(src_long)++;
}

static inline int idl4_strlen1(const char *str)

{
  int len = 1;
  while (*str++) len++;
  return len;
}

static inline int idl4_strlen2(const short *str)

{
  int len = 1;
  while (*str++) len++;
  return len;
}

#endif /* defined(__idl4_arch_generic_helpers_h__) */
