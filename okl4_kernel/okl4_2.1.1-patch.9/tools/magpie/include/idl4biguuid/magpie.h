#ifndef BIGUUID_MAGPIE_H
#define BIGUUID_MAGPIE_H
#include <inttypes.h>
/* Magpie support */
typedef unsigned char byte;

/* Casting through a union. This may not be supported in ANSI/ISO C99. However
 * it is common and I am led to believe that it is supported by major compilers.
 * The alternative to this is combining byte-at-a-time memory access with bit
 * shifting. Do not just cast and dereference, this breaks the C strict aliasing
 * rules.
 */
typedef union {
    byte u8[8];
    uint16_t u16[4];
    uint32_t u32[2];
    uint64_t u64;
} magpie_type;

/*
 * Marshalling and unmarshalling
 *
 * By design this does not rely on L4, so it probably should be moved
 * somewhere else.
*/

static inline void
magpie_marshal_int8(uint8_t src, byte *dest)
{
    *(uint8_t *)dest = src;
}

static inline void
magpie_marshal_int16(uint16_t src, byte *dest)
{
    magpie_type *d = (magpie_type *)dest;
    d->u16[0] = src;
}

static inline void
magpie_marshal_int32(uint32_t src, byte *dest)
{
    magpie_type *d = (magpie_type *)dest;
    d->u32[0] = src;
}

static inline void
magpie_marshal_int64(uint64_t src, byte *dest)
{
    magpie_type *d = (magpie_type *)dest;
    d->u64 = src;
}

static inline void *
magpie_marshal_smallstring(char *src, byte *dest)
{
	char *d = (char *)dest;

	/* Man, you can't even write a standard strcpy() these days.
	 * Suggest parens around this, assignment in condition context that.
	 * What's the point of C if you can't do dangerous things that
	 * non-C-programmers can't read?
	*/
	while(*src) {
		*d = *src;
		d++;
		src++;
	}
	*d = '\0';
	
	return d + 1;
}

static inline uint8_t
magpie_unmarshal_int8(byte *src)
{
	return *(uint8_t *)src;
}

static inline uint16_t
magpie_unmarshal_int16(byte *src)
{
    magpie_type *s = (magpie_type *)src;
    return s->u16[0];
}

static inline uint32_t
magpie_unmarshal_int32(byte *src)
{
    magpie_type *s = (magpie_type *)src;
    return s->u32[0];
}

static inline uint64_t
magpie_unmarshal_int64(byte *src)
{
    magpie_type *s = (magpie_type *)src;
    return s->u64;
}

static inline void *
magpie_unmarshal_smallstring(byte *src, char *dest)
{
	char *s = (char *)src;

	/* Man, you can't even write a standard strcpy() these days etc etc
	 * (see above) */
	while(*s) {
		*dest = *s;
		dest++;
		s++;
	}
	*dest = '\0';
	
	return s + 1;
}

#endif
