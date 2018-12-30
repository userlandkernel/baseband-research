/*
 * Copyright (c) 2004, National ICT Australia
 */
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

/*
 * Authors: Ben Leslie
 * Description: Complex arithmetic as per 7.3
 * Status: Functions defined, not implemented
 */

#error The complex library is incomplete

#define complex _Complex
#define _Complex_I (const float _Complex) foo

#define I _Complex_I

/* 7.3.5 Trigonometric functions */

/* 7.3.5.1 cacos */
double complex cacos(double complex z);
float complex cacosf(float complex z);
long double complex cacosl(long double complex z);

/* 7.3.5.2 casin */
double complex casin(double complex z);
float complex casinf(float complex z);
long double complex casinl(long double complex z);

/* 7.3.5.3 catan */
double complex catan(double complex z);
float complex catanf(float complex z);
long double complex catanl(long double complex z);

/* 7.3.5.4 ccos */
double complex ccos(double complex z);
float complex ccosf(float complex z);
long double complex ccosl(long double complex z);

/* 7.3.5.5 csin */
double complex csin(double complex z);
float complex csinf(float complex z);
long double complex csinl(long double complex z);

/* 7.3.5.6 ctan */
double complex ctan(double complex z);
float complex ctanf(float complex z);
long double complex ctanl(long double complex z);

/* 7.3.6 Hyperbolic functions */

/* 7.3.6.1 cacosh */
double complex cacosh(double complex z);
float complex cacoshf(float complex z);
long double complex cacoshf(long double complex z);

/* 7.3.6.2 casinh */
double complex casinh(double complex z);
float complex casinhf(float complex z);
long double complex casinhl(long double complex z);

/* 7.3.6.3 catanh */
double complex catanh(double complex z);
float complex catanhf(float complex z);
long double complex catanhl(long double complex z);

/* 7.3.6.4 ccosh */
double complex ccosh(double complex z);
float complex ccoshf(float complex z);
long double complex ccoshl(long double complex z);

/* 7.3.6.5 csinh */
double complex csinh(double complex z);
float complex csinhf(float complex z);
long double complex csinhl(long double complex z);

/* 7.3.6.6 ctanh */
double complex ctanh(double complex z);
float complex ctanhl(float complex z);
long double complex ctanhl(long double complex z);

/* 7.3.7 Exponential and logarithmic functions */

/* 7.3.7.1 cexp */
double complex cexp(double complex z);
float complex cexpf(float complex z);
long double complex cexpl(long double complex z);

/* 7.3.7.2 clog */
double complex clog(double complex z);
float complex clogf(float complex z);
long double complex clogl(long double complex z);

/* 7.3.8 Power and absolute value functions */

/* 7.3.8.1 cabs */
double complex cabs(double complex z);
float complex cabsf(float complex z);
long double complex cabsl(long double complex z);

/* 7.3.8.2 cpow */
double complex cpow(double complex z);
float complex cpowf(float complex z);
long double complex cpowl(long double complex z);

/* 7.3.8.3 csqrt */
double complex csqrt(double complex z);
float complex csqrtf(float complex z);
long double complex csqrtl(long double complex z);

/* 7.3.9 Manipulation functions */

/* 7.3.9.1 carg */
double complex carg(double complex z);
float complex cargf(float complex z);
long double complex cargl(long double complex z);

/* 7.3.9.2 cimag */
double complex cimag(double complex z);
float complex cimagf(float complex z);
long double complex cimagl(long double complex z);

/* 7.3.9.3 conj */
double complex conj(double complex z);
float complex conjf(float complex z);
long double complex conjl(long double complex z);

/* 7.3.9.4 cproj */
double complex cproj(double complex z);
float complex cprojf(float complex z);
long double complex cprojl(long double complex z);

/* 7.3.9.5 creal */
double complex creal(double complex z);
float complex crealf(float complex z);
long double complex creall(long double complex z);
