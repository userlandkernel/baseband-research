/*
 * Copyright (c) 2007 Open Kernel Labs, Inc. (Copyright Holder).
 * All rights reserved.
 *
 * 1. Redistribution and use of OKL4 (Software) in source and binary
 * forms, with or without modification, are permitted provided that the
 * following conditions are met:
 *
 *     (a) Redistributions of source code must retain this clause 1
 *         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
 *         (Licence Terms) and the above copyright notice.
 *
 *     (b) Redistributions in binary form must reproduce the above
 *         copyright notice and the Licence Terms in the documentation and/or
 *         other materials provided with the distribution.
 *
 *     (c) Redistributions in any form must be accompanied by information on
 *         how to obtain complete source code for:
 *        (i) the Software; and
 *        (ii) all accompanying software that uses (or is intended to
 *        use) the Software whether directly or indirectly.  Such source
 *        code must:
 *        (iii) either be included in the distribution or be available
 *        for no more than the cost of distribution plus a nominal fee;
 *        and
 *        (iv) be licensed by each relevant holder of copyright under
 *        either the Licence Terms (with an appropriate copyright notice)
 *        or the terms of a licence which is approved by the Open Source
 *        Initative.  For an executable file, "complete source code"
 *        means the source code for all modules it contains and includes
 *        associated build and other files reasonably required to produce
 *        the executable.
 *
 * 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
 * LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
 * IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
 * EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
 * THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
 * BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
 * THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
 * PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
 * PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
 * THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
 *
 * 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __GNUC__
int a __attribute__((section("SECTION_A"))) = 1;
int b __attribute__((section("SECTION_B"))) = 2;
int c __attribute__((section("SECTION_C"))) = 3;
int d __attribute__((section("SECTION_D"))) = 4;
int e __attribute__((section("SECTION_E"))) = 5;
int f __attribute__((section("SECTION_F"))) = 6;
int g __attribute__((section("SECTION_G"))) = 7;
int h __attribute__((section("SECTION_H"))) = 8;
int i __attribute__((section("SECTION_I"))) = 9;
int j __attribute__((section("SECTION_J"))) = 10;
#else
#include <rt_misc.h>

#pragma arm section rwdata = "SECTION_A"
int a = 1;
#pragma arm section rwdata = "SECTION_B"
int b = 2;
#pragma arm section rwdata = "SECTION_C"
int c = 3;
#pragma arm section rwdata = "SECTION_D"
int d = 4;
#pragma arm section rwdata = "SECTION_E"
int e = 5;
#pragma arm section rwdata = "SECTION_F"
int f = 6;
#pragma arm section rwdata = "SECTION_G"
int g = 7;
#pragma arm section rwdata = "SECTION_H"
int h = 8;
#pragma arm section rwdata = "SECTION_I"
int i = 9;
#pragma arm section rwdata = "SECTION_J"
int j = 10;
#pragma arm section
#endif

int m(void){return a;}
int n(void){return b;}
int o(void){return c;}
int p(void){return d;}
int q(void){return e;}
int r(void){return f;}
int s(void){return g;}
int t(void){return h;}
int u(void){return i;}
int v(void){return j;}

#ifndef __GNUC__
__align(8) char __initial_heap[0x100000];
__align(8) char __initial_stack[0x2000];

__value_in_regs struct __initial_stackheap
__user_initial_stackheap(unsigned r0, unsigned sp, unsigned r2, unsigned sl)
{
        struct __initial_stackheap config;

        config.heap_base = (unsigned)__initial_heap;
        config.stack_base = (unsigned)__initial_stack + sizeof(__initial_stack) - 1;
        config.heap_limit = (unsigned)__initial_heap + sizeof(__initial_heap) - 1;
        config.stack_limit = (unsigned)__initial_stack;
        return config;
}
#endif

int main (int argc, char **argv)
{
    int foo = m() + n() + o() 
            + p() + q() + r() 
            + s() + t() + u()
            + v();

    return foo - 55;
}
