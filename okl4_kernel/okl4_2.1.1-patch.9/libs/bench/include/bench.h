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
 * Last Modified: ChangHua Chen  Aug 2 2004 
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct index_type {
    char *name;
    char *unit;
};

extern struct index_type iterations;
extern struct index_type mem_size;

extern struct bench_test bench_empty;

int add_fn(int, int);
int mul_fn(int, int);

struct index {
    struct index_type *type;
    int min;
    int max;
    int step;
    int (*step_fn) (int, int);
};

struct bench_test;
struct bench_test {
    char *name;
    void (*init) (struct bench_test *, int[]);
    void (*test) (struct bench_test *, int[]);
    void (*teardown) (struct bench_test *, int[]);
#ifdef __ADS__
    struct index *indices;
#else
    struct index indices[];
#endif
};

struct counter {
    int (*get_num_counters) (struct counter *);
    const char *(*get_name) (struct counter *, int);    /* Get sub-counter name 
                                                         */
    const char *(*get_unit) (struct counter *, int);    /* Get sub-counter name 
                                                         */
    void (*init) (struct counter *);    /* Call once */
    void (*setup) (struct counter *);   /* Call before each use */
    void (*start) (void);       /* Call to start counter */
    void (*stop) (void);        /* Call to stop counter */
    uint64_t (*get_count) (int);        /* Get sub-count count */
};

struct new_bench_test;
struct new_bench_test {
    char *name;
    struct counter ** counters;
    void (*init) (struct new_bench_test *, int[]);
    void (*test) (struct new_bench_test *, int[], volatile uint64_t *);
    void (*teardown) (struct new_bench_test *, int[]);
#ifdef __ADS__
    struct index indices[1];
#else
    struct index indices[];
#endif
};

void run_test(struct counter **, struct bench_test *);
void run_new_test(struct new_bench_test *);
void null_fn(struct bench_test *, int[]);
bool always_false(void);
int return_one(void);
