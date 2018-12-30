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
#include <bench/bench.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct index_type mem_size = { "Memory size", "bytes" };
struct index_type iterations = { "Iterations", "count" };

#define ITER 5
#define MAX_COUNTERS 50

int
add_fn(int cur, int step)
{
    return cur + step;
}

int
mul_fn(int cur, int step)
{
    return cur * step;
}

static void
inner_test(struct counter **counters, struct bench_test *test,
           int *arg_array, int depth)
{
    static volatile uint64_t count[MAX_COUNTERS];
    int num_counters = 0;
    int num_subcounts = 0;
    int i, j, k;
    struct counter ** counter;
    struct index *idx = &test->indices[depth];

    void (*bench_fn) (struct bench_test *, int[]) = test->test;

    for (counter = counters; *counter != NULL; counter++) {
        num_counters++;
        num_subcounts += (*counter)->get_num_counters(*counter);
    }
    assert(num_subcounts < MAX_COUNTERS);

    for (j = idx->min; j <= idx->max; j = idx->step_fn(j, idx->step)) {
        arg_array[depth] = j;
        if (test->indices[depth + 1].type == NULL) {
            /* do the test */
            printf(" <run ");
            for (i = 0; i < depth + 1; i++) {
                if (i != 0) {
                    printf(" ");
                }
                printf("value=\"%d\"", arg_array[i]);
            }
            printf(">\n");
            for (i = 0; i < ITER; i++) {
                k = 0;

                test->init(test, arg_array);
                for (counter = counters; *counter != NULL; counter++)
                    (*counter)->start();

                bench_fn(test, arg_array);
                for (counter--, k = num_counters - 1; k >= 0; counter--, k--)
                    (*counter)->stop();

                k = 0;
                for (counter = counters; *counter != NULL; counter++)
                    for (int l = 0; l < (*counter)->get_num_counters(*counter);
                         l++)
                        count[k++] = (*counter)->get_count(l);
                test->teardown(test, arg_array);
                printf("  <data>");
                for (k = 0; k < num_subcounts; k++) {
                    if (k != 0) {
                        printf(", ");
                    }
                    printf("%" PRId64, count[k]);
                }
                printf("</data>\n");
            }
            printf(" </run>\n");
        } else {
            /* recurse */
            inner_test(counters, test, arg_array, depth + 1);
        }
    }
}

static void
new_inner_test(struct new_bench_test *test, int *arg_array, int depth)
{
    static volatile uint64_t count[MAX_COUNTERS];
    int num_counters = 0;
    int num_subcounts = 0;
    struct counter ** counter;
    int i, j, k;
    struct index *idx = &test->indices[depth];

    void (*bench_fn) (struct new_bench_test *, int[], volatile uint64_t *) = test->test;

    for (counter = test->counters; *counter != NULL; counter++) {
        num_counters++;
        num_subcounts += (*counter)->get_num_counters(*counter);
    }
    assert(num_subcounts < MAX_COUNTERS);

    for (j = idx->min; j <= idx->max; j = idx->step_fn(j, idx->step)) {
        arg_array[depth] = j;
        if (test->indices[depth + 1].type == NULL) {
            /* do the test */
            printf(" <run ");
            for (i = 0; i < depth + 1; i++) {
                if (i != 0) {
                    printf(" ");
                }
                printf("value=\"%d\"", arg_array[i]);
            }
            printf(">\n");
            for (i = 0; i < ITER; i++) {
                k = 0;
                
                test->init(test, arg_array);
                
                bench_fn(test, arg_array, count);

                test->teardown(test, arg_array);
                printf("  <data>");
                for (k = 0; k < num_subcounts; k++) {
                    if (k != 0) {
                        printf(", ");
                    }
                    printf("%" PRId64, count[k]);
                    count[k] = 0;
                }
                printf("</data>\n");
            }
            printf(" </run>\n");
        } else {
            /* recurse */
            new_inner_test(test, arg_array, depth + 1);
        }
    }
}

void
run_test(struct counter **counters, struct bench_test *test)
{
    static int arg_array[20];   /* FIXME: arg_array */
    struct index *idx;
    struct counter ** counter;
    int l;

    for (counter = counters; *counter != NULL; counter++)
        (*counter)->setup(*counter);
    printf("<test name=\"%s\" date=\"%s\" time=\"%s\" >\n", 
            test->name, __DATE__, __TIME__);
    for (counter = counters; *counter != NULL; counter++) {
        for (l= 0; l < (*counter)->get_num_counters(*counter); l++) {
            printf(" <counter name=\"%s\" unit=\"%s\" />\n",
                   (*counter)->get_name(*counter, l),
                   (*counter)->get_unit(*counter, l));
        }
    }

    for (idx = test->indices; idx->type != NULL; idx++) {
        printf(" <index name=\"%s\" unit=\"%s\"/>\n", idx->type->name,
               idx->type->unit);
    }

    inner_test(counters, test, arg_array, 0);
    printf("</test>\n");
}

void
run_new_test(struct new_bench_test *test)
{
    static int arg_array[20];
    struct index *idx;
    struct counter ** counter;
    for (counter = test->counters; *counter != NULL; counter++)
            (*counter)->setup(*counter);
    printf("<test name=\"%s\" date=\"%s\" time=\"%s\" >\n", test->name, __DATE__, __TIME__);
    for (counter = test->counters; *counter != NULL; counter++) {
        for (int l = 0; l < (*counter)->get_num_counters(*counter); l++) {
            printf(" <counter name=\"%s\" unit=\"%s\" />\n",
                    (*counter)->get_name(*counter, l),
                    (*counter)->get_unit(*counter, l));
        }
    }
    for (idx = test->indices; idx->type != NULL; idx++) {
        printf(" <index name=\"%s\" unit=\"%s\"/>\n", idx->type->name,
                idx->type->unit);
    }

    new_inner_test(test, arg_array, 0);
    printf("</test>\n");
}

bool
always_false(void)
{
    return false;
}

void
null_fn(struct bench_test *test, int args[])
{
}

int
return_one(void)
{
    return 1;
}
