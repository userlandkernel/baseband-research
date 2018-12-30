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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fs/fs.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "test_libs_fs.h"

#include <iguana/pd.h>
#include <iguana/thread.h>
#include <iguana/memsection.h>
#include <mutex/mutex.h>

static char *glob_buf; 
static char *glob_buf2;
static char *glob_buf3;
static char *glob_buf4;

static char *glob_buf_copy;

int args[2];
int args2[2];

static void read_func(void *arg)
{
    int r;
    memcpy(args, arg, 2*sizeof(int));
    glob_buf2 = malloc(10);
    
    r = read(args[0], glob_buf2, 10);
    fail_unless(strcmp(glob_buf, glob_buf2) == 0, "read/write data don't match");
    close(args[0]);
    close(args[1]);
    
    free(glob_buf2);
    thread_delete_self();
}

static void read_func2(void *arg)
{
    int r;
    r = read(args2[0], glob_buf3, 10);
    fail_unless(strcmp(glob_buf_copy, glob_buf3) == 0, "read/write data don't match");
    
    memcpy(glob_buf3, "qwertyuio\0", 10);
    r = write(args2[0], glob_buf3, 10);

    thread_delete_self();
}

/* Create a pipe and check validity of returned file descriptors */
START_TEST(PIPE01)
{
    int fds[2] = {-1, -1};
    int r;

    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");
    fail_unless(fds[0] > 0 && fds[0] < MAX_FILES, "got invalid file descriptor in fds[0]");
    fail_unless(fds[1] > 0 && fds[1] < MAX_FILES, "got invalid file descriptor in fds[1]");
}
END_TEST

/* Create a pipe, close it, and then attempt to read from it.
 */
START_TEST(PIPE02)
{
    int fds[2];
    int r;
    char buf;

    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");
    r = read(fds[1], &buf, 1);
    fail_unless(r < 0, "reading from write-only file descriptor did not fail");
    r = write(fds[0], &buf, 1);
    fail_unless(r < 0, "writing to read-only file descriptor did not fail");
    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");

    r = read(fds[0], &buf, 1);
    fail_unless(r == -1, "reading from closed file descriptor did not fail");
    fail_unless(errno == EBADF, "errno set incorrectly");
}
END_TEST

/* Create a pipe. Write to the write-only descriptor and read the data back 
 * and check for consistency */
START_TEST(PIPE03)
{
    int fds[2];
    int r;
    char *buf = "abcdefghi\0";
    char *buf2;

    buf2 = malloc(10);
    r = pipe(fds); 
    fail_unless(r == 0, "pipe creation failed");
    r = write(fds[1], buf, 10);
    fail_unless(r == 10, "writing buffer to pipe failed");
    r = read(fds[0], buf2, 10);
    fail_unless(r == 10, "reading from pipe returned wrong number of bytes");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe does not match data written to pipe");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

/* Write to the entire pipe region and read it back, then attempt to write past
 * the end of the pipe */
START_TEST(PIPE04)
{
    int fds[2];
    int r;
    char buf[PIPE_REGION_SIZE+1];
    char buf2[PIPE_REGION_SIZE+1];

    for (r=0; r<=PIPE_REGION_SIZE; r++) {
        buf[r] = (int)'a' + (r % 26);
    }
    buf[r-2] = '\0';

    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");
    r = write(fds[1], buf, PIPE_REGION_SIZE);
    fail_unless(r == PIPE_REGION_SIZE, "did not write all data to pipe");
    r = read(fds[0], buf2, PIPE_REGION_SIZE);
    fail_unless(r == PIPE_REGION_SIZE, "did not read all data from pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");
    buf[PIPE_REGION_SIZE-1] = '7';
    buf[PIPE_REGION_SIZE] = '\0';
    r = write(fds[1], buf, PIPE_REGION_SIZE+1);
    fail_unless(r == PIPE_REGION_SIZE, "wrote past end of pipe");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

/* Test wrapping around back to the start of the pipe */
START_TEST(PIPE05)
{
    int fds[2];
    int r;
    char buf[PIPE_REGION_SIZE/2];
    char buf2[PIPE_REGION_SIZE/2];

    for (r=0; r<PIPE_REGION_SIZE/2; r++) {
        buf[r] = (int)'a' + (r % 26);
    }
    buf[PIPE_REGION_SIZE/3 - 1] = '\0';

    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");
    r = write(fds[1], buf, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not write all data to pipe");
    r = write(fds[1], buf, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not write all data to pipe");
    r = read(fds[0], buf2, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not read all data from pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");
    r = read(fds[0], buf2, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not read all data from pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");
    buf[PIPE_REGION_SIZE/2 - 1] = '\0';
    r = write(fds[1], buf, PIPE_REGION_SIZE/2);
    fail_unless(r == PIPE_REGION_SIZE/2, "did not write all data to pipe");
    r = read(fds[0], buf, PIPE_REGION_SIZE/2);
    fail_unless(r == PIPE_REGION_SIZE/2, "did not write all data to pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

/* Test setting a file descriptor to non-blocking using fcntl*/
START_TEST(PIPE06)
{
    int fds[2];
    int r;
    uint32_t flags;

    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");
    r = fcntl(fds[0], F_SETFL, O_NONBLOCK);
    fail_unless(r == 0, "failed to set O_NONBLOCK flag on file descriptor");

    flags = fcntl(fds[0], F_GETFL);
    fail_unless(flags > 0, "fcntl failed to return flags");
    fail_unless((flags & O_NONBLOCK) == O_NONBLOCK, "O_NONBLOCK flags not set correctly");
    fail_unless((flags & O_ACCMODE) == O_RDONLY, "fcntl returned flags with incorrect access mode");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

/* Test that non-blocking operations don't block */
START_TEST(PIPE07)
{
    int fds[2];
    int r;
    char buf[PIPE_REGION_SIZE];
    
    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");
    r = fcntl(fds[0], F_SETFL, O_NONBLOCK);
    fail_unless(r == 0, "fcntl failed");
    r = fcntl(fds[1], F_SETFL, O_NONBLOCK);
    fail_unless(r == 0, "fcntl failed");
    r = read(fds[0], buf, 10);
    fail_unless(r < 0, "non-blocking read did not fail");
    fail_unless(errno == EAGAIN, "errno set incorrectly");

    for (r=0; r<PIPE_REGION_SIZE; r++) {
        buf[r] = (int)'a' + (r % 26);
    }

    r = write(fds[1], buf, PIPE_REGION_SIZE);
    fail_unless(r == PIPE_REGION_SIZE, "did not write whole buffer to pipe");
    r = write(fds[1], buf, PIPE_REGION_SIZE);
    fail_unless(r == -1, "non-blocking write did not fail");
    fail_unless(errno = EAGAIN, "errno set incorrectly");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

START_TEST(PIPE08)
{
    int fds[2];
    int r;

    glob_buf = malloc(10);
    memcpy(glob_buf, "abcdefghi\0", 10);
    r = pipe(fds);
    fail_unless(r == 0, "pipe creation failed");

    args[0] = fds[0];
    args[1] = fds[1];

    thread_create_simple(read_func, (void *)args, 180);

    if (!glob_buf2)
        glob_buf2 = malloc(10);
    memcpy(glob_buf2, glob_buf, 10);
    r = write(fds[1], glob_buf, 10);
    fail_unless(r == 10, "write failed");
}
END_TEST

TCase *
pipe_tests()
{
    TCase *tc;
    
    tc = tcase_create("Pipe tests");
    tcase_add_test(tc, PIPE01);
    tcase_add_test(tc, PIPE02);
    tcase_add_test(tc, PIPE03);
    tcase_add_test(tc, PIPE04);
    tcase_add_test(tc, PIPE05);
    tcase_add_test(tc, PIPE06);
    tcase_add_test(tc, PIPE07);
    tcase_add_test(tc, PIPE08);

    return tc;
}

START_TEST(SOCKETPAIR01)
{
    int fds[2] = {-1, -1};
    int r;

    r = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    fail_unless(r == 0, "socketpair creation failed");
    fail_unless(fds[0] > 0, "got invalid file descriptor in fds[0]");
    fail_unless(fds[1] > 0, "got invalid file descriptor in fds[1]");
}
END_TEST

START_TEST(SOCKETPAIR02)
{
    int fds[2];
    int r;
    char buf;

    r = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    fail_unless(r == 0, "socketpair creation failed");
    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");

    r = read(fds[0], &buf, 1);
    fail_unless(r == -1, "reading from closed file descriptor did not fail");
    fail_unless(errno == EBADF, "errno set incorrectly");
    r = read(fds[1], &buf, 1);
    fail_unless(r == -1, "reading from closed file descriptor did not fail");
    fail_unless(errno == EBADF, "errno set incorrectly");
}
END_TEST

START_TEST(SOCKETPAIR03)
{
    int fds[2];
    int r;
    char *buf = "abcdefghi\0";
    char *buf2 = "qwertyui\0";
    char buf3[10];
    char buf4[10];

    r = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    fail_unless(r == 0, "socketpair creation failed");
    r = write(fds[0], buf, 10);
    fail_unless(r == 10, "writing buffer to pipe failed");
    r = write(fds[1], buf2, 10);
    fail_unless(r == 10, "writing buffer to pipe failed");
    r = read(fds[1], buf3, 10);
    fail_unless(r == 10, "reading from pipe returned wrong number of bytes");
    fail_unless(strcmp(buf, buf3) == 0, "first set of data read from pipe does not match data written to pipe");
    r = read(fds[0], buf4, 10);
    fail_unless(r == 10, "reading from pipe returned wrong number of bytes");
    fail_unless(strcmp(buf2, buf4) == 0, "second set of data read from pipe does not match data written to pipe");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

START_TEST(SOCKETPAIR04)
{
    int fds[2];
    int r;
    char buf[PIPE_REGION_SIZE+1];
    char buf2[PIPE_REGION_SIZE+1];

    for (r=0; r<=PIPE_REGION_SIZE; r++) {
        buf[r] = (int)'a' + (r % 26);
    }
    buf[r-2] = '\0';

    errno = 0;
    r = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    fail_unless(r == 0, "socketpair creation failed");
    r = write(fds[0], buf, PIPE_REGION_SIZE);
    fail_unless(r == PIPE_REGION_SIZE, "did not write all data to pipe");
    r = read(fds[1], buf2, PIPE_REGION_SIZE);
    fail_unless(r == PIPE_REGION_SIZE, "did not read all data from pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");
    r = write(fds[1], buf, PIPE_REGION_SIZE+1);
    fail_unless(r == PIPE_REGION_SIZE, "wrote past end of pipe");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

START_TEST(SOCKETPAIR05)
{
    int fds[2];
    int r;
    char buf[PIPE_REGION_SIZE/2];
    char buf2[PIPE_REGION_SIZE/2];

    for (r=0; r<PIPE_REGION_SIZE/2; r++) {
        buf[r] = (int)'a' + (r % 26);
    }
    buf[PIPE_REGION_SIZE/3 - 1] = '\0';

    r = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    fail_unless(r == 0, "socketpair creation failed");
    r = write(fds[1], buf, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not write all data to pipe");
    r = write(fds[1], buf, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not write all data to pipe");
    r = read(fds[0], buf2, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not read all data from pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");
    r = read(fds[0], buf2, PIPE_REGION_SIZE/3);
    fail_unless(r == PIPE_REGION_SIZE/3, "did not read all data from pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");
    buf[PIPE_REGION_SIZE/2 - 1] = '\0';
    r = write(fds[1], buf, PIPE_REGION_SIZE/2);
    fail_unless(r == PIPE_REGION_SIZE/2, "did not write all data to pipe");
    r = read(fds[0], buf, PIPE_REGION_SIZE/2);
    fail_unless(r == PIPE_REGION_SIZE/2, "did not write all data to pipe");
    fail_unless(strcmp(buf, buf2) == 0, "data read from pipe did not match data written to pipe");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

START_TEST(SOCKETPAIR06)
{
    int fds[2];
    int r;

    glob_buf3 = malloc(10);
    glob_buf_copy = malloc(10);
    glob_buf3[0] = '\0';
    r = socketpair(PF_UNIX, SOCK_STREAM, 0, fds);
    fail_unless(r == 0, "socketpair creation failed");

    args2[0] = fds[0];
    args2[1] = fds[1];
    memcpy(glob_buf_copy, "abcdefghi\0", 10);
    thread_create_simple(read_func2, (void *)args, 180);

    r = write(fds[1], glob_buf_copy, 10);
    fail_unless(r == 10, "write failed");

    glob_buf4 = malloc(10);
    r = read(fds[1], glob_buf4, 10);
    fail_unless(strcmp(glob_buf4, glob_buf3) == 0, "read/write data don't match");

    r = close(fds[0]);
    fail_unless(r == 0, "close failed");
    r = close(fds[1]);
    fail_unless(r == 0, "close failed");
}
END_TEST

#ifdef __USE_POSIX
START_TEST(SERIAL01)
{
    int fd;

    fd = open("serial", O_RDWR);
    fail_unless(fd >= 0, "failed to open serial device");
}
END_TEST

START_TEST(REGULAR01)
{
    int fd = open("test_file", O_RDONLY);
    fail_unless(fd >= 0, "failed to open test_file\n");
    
    struct stat buf;
    fstat(fd, &buf);
    fail_unless(buf.st_size > 0, "stat failed");
    
    char read_buf[100];
    int r = read(fd, read_buf, buf.st_size);
    fail_unless(r == buf.st_size, "did not read entire file");
    read_buf[buf.st_size] = '\0';

    off_t o = lseek(fd, 0, SEEK_SET);
    fail_unless(o == 0, "lseek failed");
    r = read(fd, read_buf, (size_t)5);
    fail_unless(r == 5, "second read failed");
    r = read(fd, read_buf + 5, buf.st_size - 5);
    fail_unless(r == buf.st_size - 5, "third read failed");

    r = close(fd);
    fail_unless(r == 0, "close failed");
}
END_TEST

TCase *
other_tests()
{
    TCase *tc;

    tc = tcase_create("Other tests");
    tcase_add_test(tc, SERIAL01);
    tcase_add_test(tc, REGULAR01);

    return tc;
}
#endif

START_TEST(ALLOC01)
{
    memsection_ref_t ms;
    uintptr_t base;
    char *c, *d, *e, *f, *g;
    okl4_mutex_t mutex;

    ms = memsection_create(4096, &base);
    mutex = (okl4_mutex_t)memsection_base(ms);
    okl4_mutex_init(mutex);
    
    c = memsec_malloc(ms, 100);
    c[99] = 'a';
    
    e = d = memsec_malloc(ms, 250);
    d[0] = 'b';
    assert(c[99] == 'a');

    f = memsec_malloc(ms, 77);
    
    g = memsec_malloc(ms, PIPE_REGION_SIZE);
    fail_unless(g == NULL, "memsec_malloc should have failed");
    
    g = memsec_malloc(ms, 512);

    memsec_free(ms, d);
    d = memsec_malloc(ms, 250);
    fail_unless(d == e, "memsec_malloc didn't work as expected");
      
    memsec_free(ms, f);    
    memsec_free(ms, d);
    c = memsec_malloc(ms, 3128);
    fail_unless(c == NULL, "malloc should have failed");
    memsec_free(ms, g);

    memsection_delete(ms);
}
END_TEST

TCase *
socketpair_tests()
{
    TCase *tc;

    tc = tcase_create("Socketpair tests");
    tcase_add_test(tc, SOCKETPAIR01);
    tcase_add_test(tc, SOCKETPAIR02);
    tcase_add_test(tc, SOCKETPAIR03);
    tcase_add_test(tc, SOCKETPAIR04);
    tcase_add_test(tc, SOCKETPAIR05);
    tcase_add_test(tc, SOCKETPAIR06);
    tcase_add_test(tc, ALLOC01);

    return tc;
}

Suite *
make_test_libs_fs_suite(void)
{
    Suite *suite;

    suite = suite_create("FS tests\n");
    suite_add_tcase(suite, pipe_tests());
    suite_add_tcase(suite, socketpair_tests());
#ifdef __USE_POSIX
    suite_add_tcase(suite, other_tests());
#endif

    printf("Done creating suites...\n");
    return suite;
}
