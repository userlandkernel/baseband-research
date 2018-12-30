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
 * Author: Carlos Dyonisio <medaglia@ok-labs.com>
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <threadstate.h>

#include "env.h"

#include <stdio.h>

#if 0
static void env_print(char **env);

static void
env_print(char **env)
{
    printf("{\n");
    while(*env) {
        printf("\t(%s)\n", *env);
        env++;
    }
    printf("}\n");
}
#else
#define env_print(x)
#endif

inline static int env_length(char **env);
inline static int
env_length(char **env)
{
    int length = 0;

    while(*env != NULL) {
        length++;
        env++;
    }
    return length;
}

static inline char *
env_create_str(const char *name, size_t name_size, const char *value)
{
    char *str;
    size_t value_size;

    value_size = strlen(value);

    str = (char*)malloc((name_size+value_size+2)*sizeof(char));
    memcpy(str, name, name_size);
    str[name_size++] = '=';
    memcpy(str+name_size, value, value_size);
    str[name_size+value_size] = '\0';

    return str;
}

char *env_put_equal(const char *name, size_t s);

char *
env_put_equal(const char *name, size_t s)
{
    char *str;
    str = malloc(s+2);
    snprintf(str, s+2, "%s=", name);
    return str;
}

int
env_add(const char *name, size_t s, const char *value, int overwrite)
{
    // TODO: ENOMEM
    int length;
    char **p;
    int pos, old_pos;

    env_print(environ);
    length = env_length(environ);

    if(env_lookup(&pos, name, s)) {
        if(overwrite) {
            free(environ[pos]);
            environ[pos] = env_create_str(name, s, value);
        }
    } else {
        p = realloc(environ, (length+2)*sizeof(char*));
        environ = p;

        old_pos = pos;
        pos = length+1;
        while(old_pos < pos) {
            p[pos] = p[pos-1];
            pos--;
        }

        p[pos] = env_create_str(name, s, value);
    }
    env_print(environ);
    return 1;
}

#include <stdio.h>
int
env_lookup(int *new_pos, const char *str, size_t s)
{
    int pos, old_pos, end, start, r, length;
    char *name;

    name = env_put_equal(str, s);
    s++;
    length = env_length(environ);

    start = old_pos = 0;
    end = length;

    pos = (start + end) / 2;
    do {
        old_pos = pos;
        if(environ[pos] == NULL) {
            *new_pos = 0;
            free(name);
            return 0;
        }
        if((r = strncmp(name, environ[pos], s)) == 0) {
            *new_pos = pos;
            free(name);
            return 1;
        } else if(r > 0) {
            start = pos;
        } else { /* r < 0 */
            end = pos;
        }
        pos = (start + end) / 2;
    } while(pos != old_pos);

    if(strncmp(name, environ[pos], s) > 0) {
        pos++;
    }
    free(name);
    *new_pos = pos;
    return 0;
}

void
env_del(const char *str, size_t s)
{
    int pos, length;

    length = env_length(environ);

    if(env_lookup(&pos, str, s)) {
        free(environ[pos]);

        while(pos < length) {
            environ[pos] = environ[pos+1];
            pos++;
        }

        environ = realloc(environ, length*sizeof(char*));
    }
}
