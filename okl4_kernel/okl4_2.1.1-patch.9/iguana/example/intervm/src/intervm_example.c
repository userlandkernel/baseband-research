/*
 * Copyright (c) 2008 Open Kernel Labs, Inc. (Copyright Holder).
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

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include <iguana/env.h>
#include <iguana/memsection.h>
#include <fs/fs.h>
#include <l4/kdebug.h>

int
main(int argc, char **argv)
{
    const envitem_t *ms;
    memsection_ref_t intervm_ms;
    int fd, fd2, r;
    char *buf;

    L4_KDB_SetThreadName(L4_myselfconst, "intervm");
    buf = malloc(4096);
    ms = iguana_getenv("INTERVM_CTL");
    if (ms == NULL) {
        printf("Iguana environment lookup failed\n");
        return 0;
    }
    if (env_type(ms) != ENV_MEMSECTION) {
        printf("Lookup returned item of wrong type\n");
        return 0;
    }
    intervm_ms = env_memsection(ms);
    if (intervm_ms == 0) {
        printf("Invalid memsection\n");
        return 0;
    }
    printf("INTERVM_CTL located at 0x%x\n",
           (unsigned int)memsection_base(intervm_ms));

    ms = iguana_getenv("INTERVM_FWD");
    intervm_ms = env_memsection(ms);
    printf("INTERVM_FWD located at 0x%x\n",
           (unsigned int)memsection_base(intervm_ms));

    mknod("intervm_test", S_IRWXU | S_IFIFO, 0);
    fd = open("intervm_test", O_RDWR);
    printf("intervm example got fd %d\n", fd);
    fd2 = open("test_file", O_RDONLY);   

    do {
        r = read(fd2, buf, 4096);
        r = write(fd, buf, r);
    } while (r > 0);
    close(fd);
    close(fd2);

    while (*(int *)memsection_base(intervm_ms) > 0) {
        L4_ThreadSwitch(L4_anythread);
    }

    fd = open("intervm_test", O_RDWR);
    fd2 = open("test_file", O_RDONLY);   
    do {
        r = read(fd2, buf, 4096);
        r = write(fd, buf, r);
    } while (r > 0);
    close(fd);
    close(fd2);

    ms = iguana_getenv("INTERVM_BWD");
    intervm_ms = env_memsection(ms);
    printf("INTERVM_BWD located at 0x%x\n",
           (unsigned int)memsection_base(intervm_ms));

    printf("Intervm test done\n");

    while (1) { }
    return 0;
}
