/****************************************************************************
 *
 * Copyright (C) 2002-2003, Karlsruhe University
 *
 * File path:	include/piggybacker/powerpc/page.h
 * Description:	
 *
 * @LICENSE@
 *
 * $Id: page.h,v 1.1 2003/10/27 00:21:24 cvansch Exp $
 *
 ***************************************************************************/
#ifndef __PIGGYBACKER__INCLUDE__POWERPC64__PAGE_H__
#define __PIGGYBACKER__INCLUDE__POWERPC64__PAGE_H__


#define L1_CACHE_LINE_SIZE	32
#define PAGE_BITS	12
#define PAGE_SIZE	(1ul << PAGE_BITS)
#define PAGE_MASK	(~(PAGE_SIZE-1))

#warning POWERPC version
L4_INLINE void cache_partial_code_sync( L4_Word_t address )
{
    asm volatile( "dcbst 0,%0 ; sync ; icbi 0,%0" : : "r" (address) );
}

L4_INLINE void cache_complete_code_sync( void )
{
    asm volatile( "isync" );
}

L4_INLINE void memcpy_cache_flush( L4_Word_t *dst, const L4_Word_t *src, L4_Word_t size )
    /*  dst and src must be aligned by sizeof(L4_Word_t).  size must be
     *  a multiple of sizeof(L4_Word_t).
     */
{
    L4_Word_t cnt;
    int line_words = L1_CACHE_LINE_SIZE / sizeof(L4_Word_t);

    for( cnt = 0; cnt < size/sizeof(L4_Word_t); cnt++ ) {
	dst[cnt] = src[cnt];
	if( cnt && ((cnt % line_words) == 0) )
	    /*  We just started a new cache line, so flush the
	     *  prior cache line.
	     */
	    cache_partial_code_sync( (L4_Word_t)&dst[cnt-1] );
    }
    cache_partial_code_sync( (L4_Word_t)&dst[cnt-1] );
    cache_complete_code_sync();
}


#endif	/* __PIGGYBACKER__INCLUDE__POWERPC64__PAGE_H__ */
