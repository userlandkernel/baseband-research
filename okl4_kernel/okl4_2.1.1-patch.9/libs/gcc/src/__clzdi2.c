/*
 * libgcc/__clzdi2.c
 *
 * Returns the leading number of 0 bits in the argument
 *
 * Carl van Schaik <carl@ok-labs.com>
 * from __clzsi2 by Peter Anvin <hpa@zytor.com>
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

#include "types.h"
#include "libgcc.h"

uint32_t __clzdi2(uint64_t v)
{
	int p = 63;

	if (v & 0xffffffff00000000ULL) {
		p -= 32;
		v >>= 32;
	}
	if (v & 0xffff0000UL) {
		p -= 16;
		v >>= 16;
	}
	if (v & 0xff00) {
		p -= 8;
		v >>= 8;
	}
	if (v & 0xf0) {
		p -= 4;
		v >>= 4;
	}
	if (v & 0xc) {
		p -= 2;
		v >>= 2;
	}
	if (v & 0x2) {
		p -= 1;
		v >>= 1;
	}

	return p;
}
