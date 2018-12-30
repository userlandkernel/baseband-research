/*
 * arch/arm/libgcc/__muldi3.c
 *
 * Peter Anvin <hpa@zytor.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * Any copyright notice(s) and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "gcc/types.h"
#include "gcc/libgcc.h"

uint64_t __muldi3(uint64_t a, uint64_t b);

uint64_t __muldi3(uint64_t a, uint64_t b)
{
	uint32_t al = (uint32_t)a;
	uint32_t ah = (uint32_t)(a >> 32);
	uint32_t bl = (uint32_t)b;
	uint32_t bh = (uint32_t)(b >> 32);
	uint64_t v;

	v = (uint64_t)al * bl;
	v += (uint64_t)(al*bh+ah*bl) << 32;

	return v;
}
