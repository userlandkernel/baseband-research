/*
 * Copyright (c) 2006, National ICT Australia
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

#include <check/check.h>
#include <stdlib.h>
#include <stdio.h>
#include "test_libs_c.h"
#include <string.h>
#include <setjmp.h>
#include <unistd.h>
#include <errno.h>

START_TEST(strcmp_equal)
{
    static char foobar[] = "Foo, bar";

    fail_unless(strcmp("Hello World", "Hello World") == 0,
                "Two equal inline strings");
    fail_unless(strcmp("", "") == 0, "Emptry strings equal");
    fail_unless(strcmp(foobar, foobar) == 0, "The string equal itself");
    fail_unless(strcmp(foobar, "Foo, bar") == 0,
                "Out of line string equal inline string");
}
END_TEST

START_TEST(strcmp_nequal)
{
    static char foobar[] = "Foo, bar";
    static char Hello[] = "Hello";
    static char Helloo[] = "Helloo";
    static char Hell[] = "Hell";
    static char dot[] = ".";
    static char empty[] = "";

    fail_unless(strcmp(Hello, Helloo) != 0, "Two different out of line string");
    fail_unless(strcmp(dot, empty) != 0, "Empty string and non-empty string");
    fail_unless(strcmp(empty, dot) != 0, "Non-empty string and empty string");
    fail_unless(strcmp(foobar, "Foo, ba") != 0,
                "Static string with different inline string");
    fail_unless(strcmp(Hello, Hell) != 0, "String with shorter string");
}
END_TEST

START_TEST(strcmp_different)
{
    char a[] = "a";
    char b[] = "b";
    char AA[] = "AA";
    char A[] = "A";

    fail_unless(strcmp(a, b) < 0, "strcmp(\"a\", \"b\")");
    fail_unless(strcmp(b, a) > 0, "strcmp(\"b\", \"a\")");
    fail_unless(strcmp(AA, A) > 0, "strcmp(\"AA\", \"A\")");
    fail_unless(strcmp(A, AA) < 0, "strcmp(\"A\", \"AA\")");
}
END_TEST

START_TEST(strncmp_equal)
{
    int i;
    char Foo_Bar[] = "Foo, bar";
    char Hello_World[] = "Hello World";

    fail_unless(strncmp(Hello_World, Hello_World, 5) == 0,
                "Start of two string equal");
    for (i = 0; i < 4; i++)
        fail_unless(strncmp("", "", i) == 0, "Empty strings equal");

    fail_unless(strncmp(Foo_Bar, Foo_Bar, strlen(Foo_Bar)) == 0,
                "The string equal itself");
    fail_unless(strncmp(Foo_Bar, "Foo, bar", strlen(Foo_Bar)) == 0,
                "Out of line string equal inline string");
}
END_TEST

START_TEST(strncmp_partially_equal)
{
    char Hello[] = "Hello";
    char HellO[] = "HellO";
    char Hell[] = "Hell";

    fail_unless(strncmp(Hello, HellO, 4) == 0,
                "strncmp(\"Hello\", \"HellO\", 4) == 0");
    fail_unless(strncmp(HellO, Hello, 4) == 0,
                "strncmp(\"HellO\", \"Hello\", 4) == 0");
    fail_unless(strncmp(Hell, Hello, 4) == 0,
                "strncmp(\"Hell\", \"Hello\", 4) == 0");
    fail_unless(strncmp(Hello, Hell, 4) == 0,
                "strncmp(\"Hello\", \"Hell\", 4) == 0");
}
END_TEST

START_TEST(strncmp_partially_different)
{
    char Hello[] = "Hello";
    char HellO[] = "HellO";
    char Hell[] = "Hell";

    fail_unless(strncmp(Hello, HellO, 5) != 0,
                "strncmp(\"Hello\", \"HellO\", 5) != 0");
    fail_unless(strncmp(HellO, Hello, 5) != 0,
                "strncmp(\"HellO\", \"Hello\", 5) != 0");
    fail_unless(strncmp(Hell, Hello, 5) != 0,
                "strncmp(\"Hell\", \"Hello\", 5) != 0");
    fail_unless(strncmp(Hello, Hell, 5) != 0,
                "strncmp(\"Hello\", \"Hell\", 5) != 0");
}
END_TEST

// extern char *__strstr(const char *s, const char *substring);
START_TEST(strstr_exists)
{
    char Foo_bar[] = "Foo bar";
    char bar[] = "bar";
    char Foo[] = "Foo";
    char oo_b[] = "oo b";
    char Hell[] = "Hell";
    char Hello[] = "Hello";

    fail_unless(strstr(Foo_bar, bar) != NULL,
                "strstr(\"Foo bar\", \"bar\") != NULL");
    fail_unless(strstr(Foo_bar, Foo) != NULL,
                "strstr(\"Foo bar\", \"Foo\") != NULL");
    fail_unless(strstr(Foo_bar, oo_b) != NULL,
                "strstr(\"Foo bar\", \"oo b\") != NULL");
    fail_unless(strstr(Hello, Hell) != NULL,
                "strstr(\"Foo bar\", \"Hell\") != NULL");
}
END_TEST

START_TEST(strstr_empty)
{
    static char foobar[] = "Foo, bar";

    fail_unless(strstr(foobar, "") == foobar, "Second string empty");
}
END_TEST

START_TEST(strstr_nexists)
{
    fail_unless(strstr("Foo bar", "baz") == NULL,
                "strstr(\"Foo bar\", \"baz\") == NULL");
    fail_unless(strstr("Foo bar", "quz") == NULL,
                "strstr(\"Foo bar\", \"quz\") == NULL");
}
END_TEST

START_TEST(sprintf_test)
{
    char dest[100];

    fail_unless(sprintf(dest, "foo") == 3, "incorrect size returned");
    fail_unless(strcmp(dest, "foo") == 0, "printf no formatting");

    fail_unless(sprintf(dest, "%p", NULL) == 5, "incorrect size returned");
    fail_unless(strcmp(dest, "(nil)") == 0, "printf NULL pointer");
}
END_TEST

START_TEST(format_left_justify)
{
    char dest[100];

    sprintf(dest, "X: %-5d", 0);
    fail_unless(strcmp(dest, "X: 0    ") == 0, "Failed to left justify");
}
END_TEST

START_TEST(snprintf_test)
{
    char dest[100];

    memset(dest, '%', 100);     /* Make sure the buffer isn't zeroed */

    fail_unless(snprintf(dest, 2, "foo") == 3, "incorrect size returned");
    fail_unless(strcmp(dest, "f") == 0, "printf no formatting");
}
END_TEST

START_TEST(stdout_fputs_test)
{
    char *foo = "Test\n";
    int c;

    printf("IGNORE OUTPUT\n");
    c = fputs(foo, stdout);
    fail_unless(c != 0, "fputs variable");
    c = fputs("Test2\n", stdout);
    fail_unless(c != 0, "fputs constant");
}
END_TEST

START_TEST(ungetc_test)
{
    FILE *tmp = tmpfile();

    fail_unless(ungetc('x', tmp) == 'x', "Unget failed");
    fail_unless(getc(tmp) == 'x', "Getting after unget failed");
}
END_TEST

START_TEST(ungetc_multiple_test)
{
    FILE *tmp = tmpfile();

    fail_unless(ungetc('x', tmp) == 'x', "unget failed");
    fail_unless(ungetc('y', tmp) == 'y', "unget (2) failed");
    fail_unless(getc(tmp) == 'y', "getc after unget failed");
    fail_unless(getc(tmp) == 'x', "getc (2) after unget failed");
}
END_TEST

START_TEST(memcmp_equal)
{
    int i;
    char s[4][10];

    for (i = 0; i < 10; i++) {
        s[0][i] = s[1][i] = i;
        if (i < 9) {
            s[2][i] = s[3][i] = i;
        } else {
            s[2][i] = 0;
            s[3][i] = 1;
        }
    }

    fail_unless(memcmp(s[0], s[1], 10) == 0,
                "equal, when two objects have the same content");
    fail_unless(memcmp(s[2], s[3], 9) == 0,
                "equal, when only compare the parts containing the "
                "same content of the two objects");
}
END_TEST

START_TEST(memcmp_unequal)
{
    int i;
    char s[4][10];

    for (i = 0; i < 10; i++) {
        s[0][i] = i;
        s[1][i] = i + 1;

        if (i < 9) {
            s[2][i] = s[3][i] = i;
        } else {
            s[2][i] = 0;
            s[3][i] = 1;
        }
    }

    fail_unless(memcmp(s[0], s[1], 10) != 0,
                "unequal, when two memory have completed " "different content");

    fail_unless(memcmp(s[2], s[3], 10) != 0,
                "unequal, when compare the two objects with "
                "partly equal content from the start to the end");
}
END_TEST

START_TEST(memcpy_nooverlap)
{
    int i;
    char s1[10], s2[10];

    for (i = 0; i < 10; i++) {
        s2[i] = i;
    }

    fail_unless(memcpy(s1, s2, 10) == s1, "check memcpy return value");
    fail_unless(memcmp(s1, s2, 10) == 0, "check memcpy performed correctly");
}
END_TEST

START_TEST(memmove_noorwithoverlap)
{
    int i;
    char s1[10], s2[20], s4[20];
    char *s3 = &s2[5];

    for (i = 0; i < 10; i++) {
        s2[i] = s4[i] = i;
    }

    fail_unless(memmove(s1, s2, 10) == s1, "memmove non overlap, return value");
    fail_unless(memcmp(s1, s2, 10) == 0, "memmove non overlap, data correct");

    fail_unless(memmove(s3, s2, 10) == s3, "moving between twooverlap objects");
    fail_unless(memcmp(s3, s4, 10) == 0, "moving between twooverlap objects");
}
END_TEST

START_TEST(strcpy_test)
{
    char s1[10];
    char s2[] = "bigger";

    fail_unless(strcpy(s1, s2) == s1,
                "copying from small to big, return value");
    fail_unless(strcmp(s1, s2) == 0, "copying from small to big, data correct");
}
END_TEST

START_TEST(strncpy_oversized_destination)
{
    char s1[] = "thebiggest";
    char s2[] = "bigger";
    char s4[] = "\0\0\0\0";     /* four null characters */

    fail_unless(strncpy(s1, s2, 10) == s1,
                "copying size is bigger than the source but smaller than the destination, return value");
    fail_unless(strcmp(s1, s2) == 0,
                "copying size is bigger than the source but smaller than the destination, data correct");
    fail_unless(strncmp(&s1[6], s4, 4) == 0,
                "copying size is bigger than the source but smaller than the destinnation, nul padding");
}
END_TEST

START_TEST(strlcpy_test)
{
    char target[80];
    char s1[] = "A short string";
    char s2[] = "A longer string.  This should come in at one character less than target's cap..";
    char s3[] = "A longer string again; big enough to overflow the size of target by 1 character.";

    fail_unless(strlcpy(target, s1, sizeof(target)) == strlen(s1),
                "strlcpy() returned wrong string size for succesful copy");
    fail_unless(strlen(target) == strlen(s1),
                "Length of string copied by strlcpy() wrong");
    fail_unless(strcmp(target, s1) == 0, "String copied by strlcpy() is wrong");
    fail_unless(strlcpy(target, s2, sizeof(target)) == strlen(s2),
                "strlcpy() returned wrong string size for succesful copy");
    fail_unless(strlen(target) == strlen(s2),
                "Length of string copied by strlcpy() wrong");
    fail_unless(strcmp(target, s2) == 0, "String copied by strlcpy() is wrong");
    fail_unless(strlcpy(target, s3, sizeof(target)) == strlen(s3),
                "strlcpy() returned wrong string size for overlength copy");
    fail_unless(strlen(target) == sizeof(target) -1,
                "strlcpy() didn't fill buffer up before truncating on overlength copy");
    fail_unless(strlen(target) < strlen(s3),
                "Overlength copy test invalid; string didn't overflow buffer");
}
END_TEST

START_TEST(strlcpy_pathological)
{
    char target1[1] = "";
    char *target2 = NULL;

    char s1[] = "";
    char s2[] = "Hello";

    fail_unless(strlcpy(target1, s1, sizeof(target1)) == strlen(s1),
                "strlcpy() didn't handle empty string - wrong return value");
    fail_unless(strlen(target1) == strlen(s1) && strlen(target1) == 0,
                "strlcpy() didn't handle empty string - length wrong");
    fail_unless(strlcpy(target1, s2, sizeof(target1)) == strlen(s2),
                "strlcpy() didn't handle size 1 target string - wrong return val");
    fail_unless(strlen(target1) == strlen(s1) && strlen(target1) == 0,
                "strlcpy() didn't handle size 1 target string - wrong length");

    /* Note about the next two: The logic in strlcpy() should mean
       that target2 never gets touched */
    fail_unless(strlcpy(target2, s1, 0) == strlen(s1),
                "strlcpy() didn't handle 0-length target");
    fail_unless(strlcpy(target2, s2, 0) == strlen(s2),
                "strlcpy() didn't handle 0-length target");
}
END_TEST

START_TEST(memchr_findorno)
{
    char s1[10], s2[10];
    int i;

    for (i = 0; i < 10; i++) {
        s1[i] = s2[i] = i;
        if (i == 9)
            s1[i] = 'a';
    }

    fail_unless(memchr(s1, 'a', 10) == (s1 + 9),
                "find a character in a memory with this character");
    fail_unless(memchr(s2, 'a', 10) == NULL,
                "find a character in a memory without this character");
    fail_unless(memchr(s1, 'a', 8) == NULL,
                "find a character in the range without this character ");
}
END_TEST

START_TEST(strchr_findorno)
{
    char s1[] = "sheep";
    char s2[] = "ship";

    fail_unless(strchr(s1, 'e') == (s1 + 2),
                "find a non null character in a string with this character");
    fail_unless(strchr(s2, 'e') == NULL,
                "find a non null character in a string without this character");
    fail_unless(strchr(s2, '\0') == (s2 + strlen(s2)),
                "find a null character in a string");
}
END_TEST

START_TEST(strcat_stickatend)
{
    char s1[20] = "ineed";
    char s2[] = "bread";

    fail_unless(strcat(s1, s2) == s1,
                "stick s2 on the end of s1, return value");
    fail_unless(strcmp(s1, "ineedbread") == 0,
                "stick s2 on the end of s1, data correct");
}
END_TEST

START_TEST(strncat_stickatend)
{
    static char s1[40] = "ineed";
    static char s2[] = "bread";
    static char s3[10] = "br ea";

    s3[2] = '\0';

    fail_unless(strncat(s1, s2, strlen(s2)) == s1,
                "n is equal to the length of s2, return value");
    fail_unless(strcmp(s1, "ineedbread") == 0,
                "n is equal to the length of s2, data correct");

    fail_unless(strncat(s1, s2, 10) == s1,
                "n is greater to the length of s2, return value");
    fail_unless(strcmp(s1, "ineedbreadbread") == 0,
                "n is greater to the length of s2, data correct");

    fail_unless(strncat(s1, s3, 10) == s1,
                "s2 is a region with a hole, return value");
    fail_unless(strcmp(s1, "ineedbreadbreadbr") == 0,
                "s2 is a region with a hole, data correct");
}
END_TEST

START_TEST(strlcat_stickatend)
{
  char target[20] = "123456789";
  char s1[]  = "AAAA";
  char s2[]  = "BB BB ";
  char s3[]  = "C C C C";
  char r1[]  = "123456789AAAA";
  char r2[]  = "123456789AAAABB BB ";
  char fr3[] = "123456789AAAABB BB C C C C";

  fail_unless(strlcat(target, s1, sizeof(target)) == strlen(r1),
              "strlcat() returned incorrect size");
  fail_unless(strcmp(target, r1) == 0,
              "strlcat() produced incorrect string");
  fail_unless(strlcat(target, s2, sizeof(target)) == strlen(r2),
              "strlcat() returned incorrect size");
  fail_unless(strcmp(target, r2) == 0,
              "strlcat() produced incorrect string");
  fail_unless(strlcat(target, s3, sizeof(target)) == strlen(fr3),
              "strlcat() returned incorrect size on truncated cat");
  fail_unless(strcmp(target, r2) == 0,
              "strlcat() produced incorrect string on truncated cat");
}
END_TEST

START_TEST(strlcat_emptytarget)
{
    char target1[1] = "";  
    char *target2   = NULL;
    char target3[11];  /* Deliberately not initialised */

    char s1[] = "";
    char s2[] = "Hello";

    char r3[] = "HelloHello";

  fail_unless(strlcat(target1, s1, sizeof(target1)) == strlen(s1),
              "strlcat() returned incorrect size on empty string");
  fail_unless(strcmp(target1, s1) == 0,
              "strlcat() produced incorrect string");
  fail_unless(strlcat(target1, s1, sizeof(target1)) == strlen(s1),
              "strlcat() returned incorrect size on empty string");
  fail_unless(strcmp(target1, s1) == 0,
              "strlcat() produced incorrect string");
  fail_unless(strlcat(target1, s2, sizeof(target1)) == strlen(s2),
              "strlcat() returned incorrect size on empty string");
  fail_unless(strcmp(target1, s1) == 0,
              "strlcat() produced incorrect string on empty string");
  fail_unless(strlcat(target1, s2, sizeof(target1)) == strlen(s2),
              "strlcat() returned incorrect size on empty string");
  fail_unless(strcmp(target1, s1) == 0,
              "strlcat() produced incorrect string on empty string");

  fail_unless(strlcat(target2, s1, 0) == strlen(s1),
              "strlcat() returned incorrect size on NULL target string");
  fail_unless(strlcat(target2, s2, 0) == strlen(s2),
              "strlcat() returned incorrect size on NULL target string");
  
  memset(target3, '0', sizeof(target3));

  fail_unless(strlcat(target3, s2, sizeof(target3)) == strlen(s2),
              "strlcat() didn't handle uninitialised string properly");
  fail_unless(strlcat(target3, s2, sizeof(target3)) == strlen(r3),
              "strlcat() didn't handle formerly uninitialised string properly");
  fail_unless(strcmp(target3, r3) == 0,
              "strlcat() produced incorrect string on formally uninitialised string");
    
}
END_TEST

START_TEST(strcspn_inornot)
{
    char s1[] = "ineedbread";
    char s2[] = "bread";
    char s3[] = "cut";

    fail_unless(strcspn(s1, s2) == 2, "there are members of s2 in s1");
    fail_unless(strcspn(s3, s2) == 3, "there is no member of s2 in s3");
}
END_TEST

START_TEST(strpbrk_inornot)
{
    char s1[] = "ineedbread";
    char s2[] = "bread";
    char s3[] = "cut";

    fail_unless(strpbrk(s1, s2) == (s1 + 2), "there are members of s2 in s1");
    fail_unless(strpbrk(s3, s2) == NULL, "there is no member of s2 in s3");
}
END_TEST

START_TEST(strrchr_inornot)
{
    char s1[] = "ineedbread";

    fail_unless(strrchr(s1, 'e') == (s1 + 7),
                "there is non-null character 'e' in s1");
    fail_unless(strrchr(s1, 'c') == NULL,
                "there is no non-null character 'c' in s3");
    /*
     * Guess what, the follow doesn't actually end up testing strrchr, because
     * gcc wonderfully optomises calls to strrchar(x, '\0') to actually call
     * strchr(x, '\0') 
     */
    fail_unless(strrchr(s1, '\0') == (s1 + strlen(s1)),
                "the searched is null character");
}
END_TEST

START_TEST(strspn_inornot)
{
    char s1[] = "ididbread";
    char s2[] = "ineed";

    fail_unless(strspn(s1, s2) == 4, "there are members of s2 in s1");
    fail_unless(strspn(s2, s2) == 5, "there is no member of s2 in s3");
}
END_TEST

START_TEST(strtok_split)
{
    char test1[] = "want a break!";
    char test2[] = "continues";
    char test3[] = "--";

    char delims[] = " .,;:!-";

    fail_unless((strtok(test1, delims) == test1
                 && strtok(NULL, delims) == test1 + 5
                 && strtok(NULL, delims) == test1 + 7
                 && strtok(NULL, delims) == NULL),
                "there are members of delims within test1");
    fail_unless(strtok(test2, delims) == test2 &&
                strtok(NULL, delims) == NULL,
                "there is no member of delims in test2");
    fail_unless(strtok(test3, delims) == NULL,
                "there is only member of delims in test3");
}
END_TEST

START_TEST(strtok_r_split)
{
    char test1[] = "want a break!";
    char test2[] = "continues";
    char test3[] = "--";

    char delims[] = " .,;:!-";

    char *tmp = NULL;

    fail_unless((strtok_r(test1, delims, &tmp) == test1
                 && strtok_r(NULL, delims, &tmp) == test1 + 5
                 && strtok_r(NULL, delims, &tmp) == test1 + 7
                 && strtok_r(NULL, delims, &tmp) == NULL),
                "there are members of delims within test1");
    fail_unless(strtok_r(test2, delims, &tmp) == test2 &&
                strtok_r(NULL, delims, &tmp) == NULL,
                "there is no member of delims in test2");
    fail_unless(strtok_r(test3, delims, &tmp) == NULL,
                "there is only member of delims in test3");
}
END_TEST

START_TEST(memset_simple_test)
{
    char buf[10];
    unsigned int i;

    for (i = 0; i < 10; i++)
        buf[i] = 0;

    fail_unless(memset(buf, 'I', 9) == buf, "memset: error on simple test");
    fail_unless(strspn(buf, "I") == 9, "memset: bad data");

}
END_TEST

START_TEST(memset_wordsize_alignment)
{
    char buf1[17], *buf2;
    unsigned int i, count, align;

    for (i = 0; i < 17; i++)
        buf1[i] = 0;

#if UINTPTR_MAX == UINT32_MAX
    align = 4;
#elif UINTPTR_MAX == UINT64_MAX
    align = 8;
#endif
    for (i = 1; i < align; i++) {

        buf2 = buf1 + i;
        count = align - i;

        fail_unless(memset(buf1, 'I', 16) == buf1,
                    "memset: error on aligned case");
        fail_unless(strspn(buf1, "I") == 16,
                    "memset: bad data or memory corruption by writing to wrong address");

        fail_unless(memset(buf2, 'A', count) == buf2,
                    "memset: error on unaligned start & aligned end");
        fail_unless(strspn(buf2, "A") == count,
                    "memset: bad data or memory corruption by writing to wrong address");

        fail_unless(memset(buf2, 'B', 8) == buf2,
                    "memset: error on unaligned start & unaligned end");
        fail_unless(strspn(buf2, "B") == 8,
                    "memset: bad data or memory corruption by writing to wrong address");

        fail_unless(memset(buf1, 'C', 4) == buf1,
                    "memset: error on aligned start & unaligned end");
        fail_unless(strspn(buf1, "C") == 4,
                    "memset: bad data or memory corruption by writing to wrong address");
    }
}
END_TEST

#define PAGESIZE 4096
#define PAGEBITS 12
START_TEST(memset_pagesize_alignment)
{
    char *buf1, *buf2;
    unsigned int i;

    buf1 = (char *)malloc(PAGESIZE + 16);
    fail_unless(buf1 != 0, "not enough memory on malloc");

    for (i = 0; i < PAGESIZE + 16; i++)
        buf1[i] = 0;
    /*
     * find the page boundary and make buf2 points to a few bytes before this
     * boundary 
     */
    buf2 =
        (char *)(((((uintptr_t)buf1 + PAGESIZE) >> PAGEBITS) << PAGEBITS) - 8);

    fail_unless(memset(buf2, 'I', 16) == buf2,
                "memset: error on setting a region cross page boundary");
    fail_unless(strspn(buf2, "I") == 16,
                "memset: bad data or memory corruption by writing to wrong address");

    free(buf1);
}
END_TEST

START_TEST(strlen_length)
{
    char s1[] = "aaaaa";

    fail_unless(strlen(s1) == 5, "returned the length of a string");
}
END_TEST

/* 2008-01-14: Test broken on PC99. See Mothra Issue #2311. */
#if 0

/* setjmp_simple globals and helper functions */
static jmp_buf env;
static int expect = 0;

static void
jump(int val)
{
    // printf("jump(%d)\n", val);
    longjmp(env, val);
}

START_TEST(setjmp_simple)
{
    int val;

    val = setjmp(env);
    // printf("%d <- setjmp()\n", val);
    fail_unless(val == expect, "setjmp returned wrong value");

    /*
     * FIXME: need a more systematic way of testing 
     */
    if (val == 0) {
        expect = 1;
        jump(0);
    } else if (val == 1) {
        jump(expect = 5);
    } else if (val == 5) {
        jump(expect = 12);
    } else if (val == 12) {
        jump(expect = 13);
    }
}
END_TEST

#endif

START_TEST(malloc_simple)
{
    void *mem = malloc(213);

    free(mem);
}
END_TEST

START_TEST(malloc_lots)
{
    void *mem[1024];
    int i;

    for (i = 0; i < 1024; i++) {
        mem[i] = malloc(123);
        fail_unless(mem[i] != NULL, "malloc: returned NULL.");
        if (mem[i] == NULL) {
            break;
        }
    }

    for (i--; i >= 0; i--) {
        free(mem[i]);
    }
}
END_TEST

START_TEST(free_simple)
{
    /* check we can free NULL */
    free(NULL);
}
END_TEST

START_TEST(calloc_simple)
{
    char *mem = calloc(128, 1);
    int i;

    /* check that calloc returns zeroed memory */
    for (i = 0; i < 10; i++)
        fail_unless(mem[i] == 0, "calloc didn't zero memory");

    free(mem);
}
END_TEST

START_TEST(strtoul_hex)
{
    char *str = "0x06040000&0xffff0000";
    char *next;
    unsigned long val;

    val = strtoul(str, &next, 0);
    fail_unless(val == 0x06040000,
                "Value not calculated correctly. Guess base.");

    val = strtoul(str, &next, 0x10);
    fail_unless(val == 0x06040000,
                "Value not calculated correctly. Explicit base.");
}
END_TEST

START_TEST(strtoul_upper_hex)
{
    char *str = "0xaBcDeF";
    char *next;
    unsigned long val;

    val = strtoul(str, &next, 0);
    fail_unless(val == 0xabcdef, "Value not calculated correctly.");
}
END_TEST

START_TEST(strtoul_decimal)
{
    char *str = "6040000&37";
    char *next;
    unsigned long val;

    val = strtoul(str, &next, 0);
    fail_unless(val == 6040000, "Value not calculated correctly");

    val = strtoul(str, &next, 10);
    fail_unless(val == 6040000, "Value not calculated correctly");
}
END_TEST

START_TEST(fflush_stdout)
{
    if (stdout->buffer != NULL) {
        char hello[] = "hello";
        for (int i=0; i<strlen(hello) + 1; i++) {
            fputc(hello[i], stdout);
        }
        fail_unless(strcmp(stdout->buffer, hello)==0, "Buffer doesn't contain correct value");
    
        fflush(stdout);
        fail_unless(stdout->buffer == stdout->buffer_end, "Buffer not flushed correctly");

        printf("ignore this\n");
        fail_unless(stdout->buffer == stdout->buffer_end, "Line buffering not working on stdout");

    } else {
        printf("buffering not enabled, fflush test not run\n");
    }
}
END_TEST

START_TEST(setvbuf_stdout)
{
    if (stdout->buffer != NULL) {

        /* Cache their buffer details */
        char* theirbuf = stdout->buffer;
        size_t theirbufsiz = stdout->buffer_size;

        /* Set to our buffer */
        int ourbufsiz = 200;
        char* ourbuf = (char*)malloc(sizeof(char) * ourbufsiz);
        setvbuf(stdout, ourbuf, _IOLBF, ourbufsiz);

        /* Ensure setting worked correctly */
        fail_unless(stdout->buffering_mode == _IOLBF, "Buffering mode not set correctly");
        fail_unless(stdout->buffer == ourbuf, "Buffer not set correctly");
        fail_unless(stdout->buffer_size == ourbufsiz, "Buffer size not set correctly");

        /* fputc a word and ensure it hits our buffer */
        char hello[] = "hello";
        for (int i=0; i<strlen(hello) + 1; i++) {
            fputc(hello[i], stdout);
        }
        fail_unless(strcmp(stdout->buffer, hello)==0, "Buffer doesn't contain correct value");

        /* Flush our buffer and ensure it empties */
        fflush(stdout);
        fail_unless(stdout->buffer == stdout->buffer_end, "Buffer not flushed correctly");
        
        /* Reset back to their existing buffer */
        setvbuf(stdout, theirbuf, _IOLBF, theirbufsiz);

        /* Free our buffer */
        free(ourbuf);

    } else {
        printf("buffering not enabled, setvbuf test not run\n");
    }
}
END_TEST

START_TEST(imaxabs_test)
{
    intmax_t x;
    intmax_t res;

    x = 123987456;
    res = imaxabs(x);
    fail_unless(res == (intmax_t)123987456, "Value not calculated correctly.");

    x = -123987456;
    res = imaxabs(x);
    fail_unless(res == (intmax_t)123987456, "Negative value not calculated correctly.");
}
END_TEST

START_TEST(div_tests)
{
    div_t dt;
    ldiv_t ldt;
    lldiv_t lldt;
    imaxdiv_t imaxdt;

    dt = div((int)11, (int)4);
    fail_unless(dt.quot == 2, "div() - quot not calculated correctly.");
    fail_unless(dt.rem == 3, "div() - rem not calculated correctly.");

    ldt = ldiv((long)11, (long)4);
    fail_unless(ldt.quot == (long)2, "ldiv() - quot not calculated correctly.");
    fail_unless(ldt.rem == (long)3, "ldiv() - rem not calculated correctly.");

    lldt = lldiv((long long)11, (long long)4);
    fail_unless(lldt.quot == (long long)2, "lldiv() - quot not calculated correctly.");
    fail_unless(lldt.rem == (long long)3, "lldiv() - rem not calculated correctly.");

    imaxdt = imaxdiv((intmax_t)11, (intmax_t)4);
    fail_unless(imaxdt.quot == (intmax_t)2, "imaxdiv() - quot not calculated correctly.");
    fail_unless(imaxdt.rem == (intmax_t)3, "imaxdiv() - rem not calculated correctly.");
}
END_TEST

START_TEST(strerror_tests)
{
    char *ret;
    char buf[50];

    ret = strerror(EDOM);
    fail_unless(strcmp(ret, "EDOM: Domain error.") == 0, "EDOM not returned correctly");
    ret = strerror(ERANGE);
    fail_unless(strcmp(ret, "ERANGE: Value out of range.") == 0, "ERANGE not returned correctly");
    (void)strerror_r(EDOM, buf, 50);
    fail_unless(strcmp(buf, "EDOM: Domain error.") == 0, "strerror_r: EDOM not returned correctly");
    (void)strerror_r(ERANGE, buf, 50);
    fail_unless(strcmp(buf, "ERANGE: Value out of range.") == 0, "strerror_r: ERANGE not returned correctly");
}
END_TEST

/* 
 * gmtime also tests localtime, at least for now,
 * with incomplete timezone support.
 */
#include <time.h>
START_TEST(gmtime_and_mktime)
{
    struct tm *pt;
    struct tm t;
    time_t tmp;

    tmp = 1191798951;
    pt = gmtime(&tmp);
    fail_unless(pt->tm_hour == 23, "gmtime 1: Incorrect tm_hour value!");
    fail_unless(pt->tm_min == 15, "gmtime 1: Incorrect tm_min value!");
    fail_unless(pt->tm_sec == 51, "gmtime 1: Incorrect tm_sec value!");
    fail_unless(pt->tm_year == 107, "gmtime 1: Incorrect tm_year value!");
    fail_unless(pt->tm_wday == 0, "gmtime 1: Incorrect tm_wday value!");
    fail_unless(pt->tm_yday == 279, "gmtime 1: Incorrect tm_yday value!");
    fail_unless(pt->tm_mon == 9, "gmtime 1: Incorrect tm_mon value!");
    fail_unless(pt->tm_mday == 7, "gmtime 1: Incorrect tm_mday value!");
    fail_unless(mktime(pt) == tmp, "mktime 1: wrong value!");

    tmp = 1000000000;
    gmtime_r(&tmp, &t);
    fail_unless(t.tm_hour == 1, "gmtime 2: Incorrect tm_hour value!");
    fail_unless(t.tm_min == 46, "gmtime 2: Incorrect tm_min value!");
    fail_unless(t.tm_sec == 40, "gmtime 2: Incorrect tm_sec value!");
    fail_unless(t.tm_mday == 9, "gmtime 2: Incorrect tm_mday value!");
    fail_unless(t.tm_mon == 8, "gmtime 2: Incorrect tm_mon value!");
    fail_unless(t.tm_year == 101, "gmtime 2: Incorrect tm_year value!");
    fail_unless(t.tm_wday == 0, "gmtime 2: Incorrect tm_wday value!");
    fail_unless(t.tm_yday == 251, "gmtime 2: Incorrect tm_yday value!");
    fail_unless(mktime(&t) == tmp, "mktime 2: wrong value!");

    tmp = -1000000000;
    gmtime_r(&tmp, &t);
    fail_unless(t.tm_hour == 22, "gmtime 3: Incorrect tm_hour value!");
    fail_unless(t.tm_min == 13, "gmtime 3: Incorrect tm_min value!");
    fail_unless(t.tm_sec == 20, "gmtime 3: Incorrect tm_sec value!");
    fail_unless(t.tm_mday == 24, "gmtime 3: Incorrect tm_mday value!");
    fail_unless(t.tm_mon == 3, "gmtime 3: Incorrect tm_mon value!");
    fail_unless(t.tm_year == 38, "gmtime 3: Incorrect tm_year value!");
    fail_unless(t.tm_yday == 113, "gmtime 3: Incorrect tm_yday value!");
    fail_unless(t.tm_wday == 0, "gmtime 3: Incorrect tm_wday value!");
    fail_unless(mktime(&t) == tmp, "mktime 3: wrong value!");

    tmp = -1191298951;
    pt = gmtime(&tmp);
    fail_unless(pt->tm_hour == 19, "gmtime 4: Incorrect tm_hour value!");
    fail_unless(pt->tm_min == 37, "gmtime 4: Incorrect tm_min value!");
    fail_unless(pt->tm_sec == 29, "gmtime 4: Incorrect tm_sec value!");
    fail_unless(pt->tm_mday == 1, "gmtime 4: Incorrect tm_mday value!");
    fail_unless(pt->tm_mon == 3, "gmtime 4: Incorrect tm_mon value!");
    fail_unless(pt->tm_year == 32, "gmtime 4: Incorrect tm_year value!");
    fail_unless(pt->tm_yday == 91, "gmtime 4: Incorrect tm_yday value!");
    fail_unless(pt->tm_wday == 5, "gmtime 4: Incorrect tm_wday value!");
    fail_unless(mktime(pt) == tmp, "mktime 4: wrong value!");
}
END_TEST

START_TEST(environment)
{
    setenv("LC_ALL", "Just a test", 0);
    setenv("CDPATH", "Just a test", 0);
    setenv("SHELL", "Just a test", 0);
    setenv("PS1", "Just a test", 0);
    setenv("YACC", "Just a test", 0);
    setenv("CFLAGS", "Just a test", 0);
    setenv("LPDEST", "Just a test", 0);
    setenv("LS_COLORS", "Just a test", 0);
    setenv("FC", "Just a test", 0);
    setenv("MANPATH", "Just a test", 0);
    setenv("PWD", "Just a test", 0);
    setenv("TERM", "Just a test", 0);
    setenv("GET", "Just a test", 0);
    setenv("COLUMNS", "Just a test", 0);
    setenv("LEX", "Just a test", 0);
    setenv("TMPDIR", "Just a test", 0);
    setenv("USER", "Just a test", 0);
    setenv("VISUAL", "Just a test", 0);
    setenv("ARFLAGS", "Just a test", 0);
    setenv("MORE", "Just a test", 0);

    setenv("LS_COLORS", "Just an updated test", 1);
    setenv("FC", "Just an updated test", 0);
    setenv("MANPATH", "Just an updated test", 1);
    setenv("PWD", "Just an updated test", 0);

    fail_unless(strcmp(getenv("LEX"), "Just a test") == 0, "Failed!");
    fail_unless(strcmp(getenv("FC"), "Just a test") == 0, "Failed!");
    fail_unless(strcmp(getenv("PWD"), "Just a test") == 0, "Failed!");
    fail_unless(strcmp(getenv("LS_COLORS"), "Just an updated test") == 0, "Failed!");
    fail_unless(strcmp(getenv("MANPATH"), "Just an updated test") == 0, "Failed!");
    fail_unless(getenv("FFFFC") == NULL, "Failed!");

    fail_unless(setenv(NULL, NULL, 0) == -1, "Failed!");
    fail_unless(setenv("", "", 0) == -1, "Failed!");
    fail_unless(setenv("LALA=A", "", 0) == -1, "Failed!");

    unsetenv("LEX");
    unsetenv("GET");
    unsetenv("LPDEST");
    unsetenv("YACC");
    fail_unless(getenv("LEX") == NULL, "Failed!");

    fail_unless(unsetenv(NULL) == -1, "Failed!");
    fail_unless(unsetenv("") == -1, "Failed!");
    fail_unless(unsetenv("LALA=A") == -1, "Failed!");

    fail_unless(strcmp(environ[0], "ARFLAGS=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[1], "CDPATH=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[2], "CFLAGS=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[3], "COLUMNS=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[4], "FC=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[5], "LC_ALL=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[6], "LS_COLORS=Just an updated test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[7], "MANPATH=Just an updated test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[8], "MORE=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[9], "PS1=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[10], "PWD=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[11], "SHELL=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[12], "TERM=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[13], "TMPDIR=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[14], "USER=Just a test") == 0, "Not ordered!");
    fail_unless(strcmp(environ[15], "VISUAL=Just a test") == 0, "Not ordered!");
    fail_unless(environ[16] == NULL, "Not ordered!");
}
END_TEST

START_TEST(strftime_weeks)
{
    struct tm t;
    char buf[50];
    int ret;

    t.tm_year = 2007 - 1900;
    t.tm_wday = 5;
    t.tm_yday = 46;
    
    ret = strftime(buf, 50, "%U %V %W", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "06 07 07") == 0, "strftime(): Error with U V W.");
    fail_unless(ret == 9, "strftime(): Wrong return value!");

    t.tm_year = 2007 - 1900;
    t.tm_wday = 0;
    t.tm_yday = 48;
    
    ret = strftime(buf, 50, "%U %V %W", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "07 07 07") == 0, "strftime(): Error with U V W.");
    fail_unless(ret == 9, "strftime(): Wrong return value!");

    t.tm_year = 2006 - 1900;
    t.tm_wday = 4;
    t.tm_yday = 4;
    
    ret = strftime(buf, 50, "%U %V %W", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "01 01 01") == 0, "strftime(): Error with U V W.");
    fail_unless(ret == 9, "strftime(): Wrong return value!");

    t.tm_year = 2006 - 1900;
    t.tm_wday = 0;
    t.tm_yday = 0;
    
    ret = strftime(buf, 50, "%U %V %W", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "01 52 00") == 0, "strftime(): Error with U V W.");
    fail_unless(ret == 9, "strftime(): Wrong return value!");

    t.tm_year = 2006 - 1900;
    t.tm_wday = 2;
    t.tm_yday = 2;
    
    ret = strftime(buf, 50, "%U %V %W", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "01 01 01") == 0, "strftime(): Error with U V W.");
    fail_unless(ret == 9, "strftime(): Wrong return value!");
}
END_TEST

START_TEST(strftime_week_month)
{
    struct tm t;
    char buf[50];
    int ret;

    t.tm_year = 2007 - 1900;
    t.tm_wday = 0;
    t.tm_yday = 27;
    t.tm_mon = 0;
    
    ret = strftime(buf, 50, "%a %A %b %B %h %m %u %w", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "Sun Sunday Jan January Jan 01 7 0") == 0,
                            "strftime(): Error with a A b B h m u w.");
    fail_unless(ret == 34, "strftime(): Wrong return value!");

    t.tm_year = 2007 - 1900;
    t.tm_wday = 1;
    t.tm_yday = 70;
    t.tm_mon = 2;
    
    ret = strftime(buf, 50, "%a %A %b %B %h %m %u %w", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "Mon Monday Mar March Mar 03 0 1") == 0,
                            "strftime(): Error with a A b B h m u w.");
    fail_unless(ret == 32, "strftime(): Wrong return value!");

    t.tm_year = 2007 - 1900;
    t.tm_wday = 6;
    t.tm_yday = 299;
    t.tm_mon = 9;
    
    ret = strftime(buf, 50, "%a %A %b %B %h %m %u %w", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "Sat Saturday Oct October Oct 10 5 6") == 0,
                            "strftime(): Error with a A b B h m u w.");
    fail_unless(ret == 36, "strftime(): Wrong return value!");
}
END_TEST

/* 
 * This is here because of some stupid versions of gcc, which warn the
 * programmer about "bad" usage of strftime. (Carlos Dyonisio)
 */
inline static size_t my_strftime(char *s, size_t max, const char  *fmt,
                                            const struct tm *tm) {
    return strftime(s, max, fmt, tm);
}

START_TEST(strftime_year)
{
    struct tm t;
    char buf[50];
    int ret;

    t.tm_year = 2007 - 1900;
    t.tm_wday = 0;
    t.tm_yday = 27;
    t.tm_mon = 0;
    
    ret = my_strftime(buf, 50, "%C %g %G %y %Y", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "20 07 2007 07 2007") == 0,
                            "strftime(): Error with C g G y Y.");
    fail_unless(ret == 19, "strftime(): Wrong return value!");

    t.tm_year = 2007 - 1900;
    t.tm_wday = 1;
    t.tm_yday = 364;
    t.tm_mon = 11;
    
    my_strftime(buf, 50, "%C %g %G %y %Y", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "20 08 2008 07 2007") == 0,
                            "strftime(): Error with C g G y Y.");
    fail_unless(ret == 19, "strftime(): Wrong return value!");

    t.tm_year = 2000 - 1900;
    t.tm_wday = 0;
    t.tm_yday = 1;
    t.tm_mon = 0;
    
    my_strftime(buf, 50, "%C %g %G %y %Y", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "20 99 1999 00 2000") == 0,
                            "strftime(): Error with C g G y Y.");
    fail_unless(ret == 19, "strftime(): Wrong return value!");
}
END_TEST

START_TEST(strftime_date)
{
    struct tm t;
    char buf[50];
    int ret;

    t.tm_year = 1987 - 1900;
    t.tm_wday = 3;
    t.tm_mday = 7;
    t.tm_yday = 6;
    t.tm_mon = 0;
    
    ret = my_strftime(buf, 50, "%d %D %e %F %j %x", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "07 01/07/87  7 1987-01-07 007 01/07/87") == 0,
                            "strftime(): Error with d D e F j x.");
    fail_unless(ret == 39, "strftime(): Wrong return value!");

    t.tm_year = 1996 - 1900;
    t.tm_wday = 2;
    t.tm_mday = 19;
    t.tm_yday = 323;
    t.tm_mon = 10;
    
    my_strftime(buf, 50, "%d %D %e %F %j %x", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "19 11/19/96 19 1996-11-19 324 11/19/96") == 0,
                            "strftime(): Error with d D e F j x.");
    fail_unless(ret == 39, "strftime(): Wrong return value!");

    t.tm_year = 1989 - 1900;
    t.tm_wday = 4;
    t.tm_mday = 20;
    t.tm_yday = 109;
    t.tm_mon = 3;
    
    my_strftime(buf, 50, "%d %D %e %F %j %x", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "20 04/20/89 20 1989-04-20 110 04/20/89") == 0,
                            "strftime(): Error with d D e F j x.");
    fail_unless(ret == 39, "strftime(): Wrong return value!");
}
END_TEST

START_TEST(strftime_hour)
{
    struct tm t;
    char buf[52];
    int ret;

    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    
    ret = strftime(buf, 52, "%H %I %M %p %r %R %S %T %X", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "00 12 00 AM 12:00:00 AM 00:00 00 00:00:00 00:00:00") == 0,
                            "strftime(): Error with H I M p r R S T X.");
    fail_unless(ret == 51, "strftime(): Wrong return value!");

    t.tm_hour = 7;
    t.tm_min = 23;
    t.tm_sec = 56;
    
    strftime(buf, 52, "%H %I %M %p %r %R %S %T %X", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "07 07 23 AM 07:23:56 AM 07:23 56 07:23:56 07:23:56") == 0,
                            "strftime(): Error with H I M p r R S T X.");
    fail_unless(ret == 51, "strftime(): Wrong return value!");

    t.tm_hour = 23;
    t.tm_min = 54;
    t.tm_sec = 30;
    
    strftime(buf, 52, "%H %I %M %p %r %R %S %T %X", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "23 11 54 PM 11:54:30 PM 23:54 30 23:54:30 23:54:30") == 0,
                            "strftime(): Error with H I M p r R S T X.");
    fail_unless(ret == 51, "strftime(): Wrong return value!");
}
END_TEST

START_TEST(strftime_misc)
{
    struct tm t;
    char buf[60];
    int ret;

    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    t.tm_year = 1996 - 1900;
    t.tm_wday = 2;
    t.tm_mday = 19;
    t.tm_yday = 323;
    t.tm_mon = 10;
    t.tm_isdst = 0;
    
    ret = my_strftime(buf, 60, "misc: %c abcdefg %t %z-%Z-%% test!", &t);
    printf("%s\n", buf);
    fail_unless(strcmp(buf, "misc: Tue Nov 19 00:00:00 1996 abcdefg \t +0000-GMT-% test!") == 0,
                            "strftime(): Error with c t z Z.");
    fail_unless(ret == 59, "strftime(): Wrong return value!");

    ret = my_strftime(buf, 10, "misc: %c abcdefg %t %z-%Z-%% test!", &t);
    fail_unless(ret == 0, "strftime(): Wrong return value!");

}
END_TEST

/* asctime also tests ctime, as ctime calls asctime. */
START_TEST(asctime_test)
{
    struct tm *pt;
    time_t t = 1193026983;
    char *ptr;
    char buf[26];

    pt = gmtime(&t);

    ptr = asctime(pt);
    asctime_r(pt, buf);
    fail_unless(strcmp(ptr, buf) == 0, "asctime(): error!");
    fail_unless(strcmp(ptr, "Mon Oct 22 04:23:03 2007\n") == 0, "asctime(): error!");
}
END_TEST

START_TEST(tzset_test)
{
    _tzset("GMT0");
    fail_unless(timezone == 0, "timezone != 0!");
    printf("tzname = %s\n", tzname[0]);
    fail_unless(strcmp(tzname[0], "GMT") == 0, "tzname[0] != GMT");

    _tzset("GMT+10:30");
    printf("timezone = %ld\n", timezone);
    fail_unless(timezone == 37800, "timezone != 37800!");
    printf("tzname = %s\n", tzname[0]);
    fail_unless(strcmp(tzname[0], "GMT") == 0, "tzname[0] != GMT");

    _tzset("GMT-3");
    printf("timezone = %ld\n", timezone);
    fail_unless(timezone == -10800, "timezone != -10800!");
    printf("tzname = %s\n", tzname[0]);
    fail_unless(strcmp(tzname[0], "GMT") == 0, "tzname[0] != GMT");
}
END_TEST

START_TEST(strtol_tests)
{
    long l;
    char *str = "56aaa";
    char *cocomole;
//    long long ll;

    l = strtol(str, &cocomole, 10);
    printf("cocomole = %s\n", cocomole);
    fail_unless(l = 56, "345 != 345");
}
END_TEST

#ifndef CONFIG_WITHOUT_FLOATING
struct abcd {
    int i1;
    int i2;
};

union dbu {
    double d;
    struct abcd i;
};

START_TEST(strtof_tests)
{
    float f;
    double d;
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
    long double ld;
#endif

    errno = 0;
    f = strtof("2.345", NULL);
    fail_unless(f == (float)2.345, "f != 2.345!");
    f = strtof("1.0e500", NULL);
    fail_unless(f == (float)1.0e500, "f != inf");
    fail_unless(errno == ERANGE, "errno != ERANGE");

    errno = 0;
    d = strtod("   0003.4567p", NULL);
    fail_unless((float)d == (float)3.4567, "d != 2.345!");
    d = strtod("1.0e-500", NULL);
    fail_unless(d == (double)1.0e-500, "d != inf");
    fail_unless(errno == ERANGE, "errno != ERANGE");

#ifndef CONFIG_WITHOUT_LONG_DOUBLE
    ld = strtold("0x3.456p-10", NULL);
    fail_unless(ld == (long double)0x3.456p-10, "ld != 0x3.456p-10");
#endif

    errno = 0;
    d = strtod("1.0e300", NULL);
    fail_unless(d == (double)1.0e300, "d != 1.0e300");
    fail_unless(d != (double)1.0e500, "d == inf");
    fail_unless(errno == 0, "errno != 0");

    f = strtof("1.0e300", NULL);
    fail_unless(f == (float)1.0e500, "f != inf");
    fail_unless(errno == ERANGE, "errno != ERANGE");
}
END_TEST

START_TEST(scanf_basic)
{
    int i, n;
    float x = 0.0;
    char name[50];
    char *str = "25 54.32E-1 Hamster";

    n = sscanf(str, "%d%f%s", &i, &x, name);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(i == 25, "%%d not calculated correctly.");
    fail_unless(x == (float)5.432, "%%f not calculated correctly.");
    fail_unless(strcmp(name, "Hamster") == 0, "%%s not correctly parsed.");
}
END_TEST

START_TEST(scanf_basic2)
{
    int i, n;
    float x;
    char name[50];
    char *str = "56789 0123 56a72";

    n = sscanf(str, "%2d%f%*d %[0123456789]", &i, &x, name);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(i == 56, "%%d not calculated correctly.");
    fail_unless(x == (float)789.0, "%%f not calculated correctly.");
    fail_unless(strcmp(name, "56") == 0, "%%s not correctly parsed.");
}
END_TEST

START_TEST(scanf_basic3)
{
    int n;
    float x;
    char str1[50];
    char str2[50];
    char *str = "OK Labs.0765abcdef";

    n = sscanf(str, "%[^.] %f %s", str1, &x, str2);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(strcmp(str1, "OK Labs") == 0, "%%s (str1) not correctly parsed.");
    fail_unless(strcmp(str2, "abcdef") == 0, "%%s (str2) not correctly parsed.");
    fail_unless(x == (float).0765, "%%f not calculated correct.");
}
END_TEST
#endif

START_TEST(scanf_error)
{
    int i, n, j;
    char name[50];
    char *str = "25 14324 Hamster";

    n = sscanf(str, "24 %d%d%s", &i, &j, name);
    fail_unless(n == 0, "Wrong return value.");

    n = sscanf("2", "24 %d%d%s", &i, &j, name);
    fail_unless(n == EOF, "Error EOF: Wrong return value.");

    n = sscanf("  ", "%s", name);
    printf("1492: n=%d\n", n);
    fail_unless(n == EOF, "Error EOF %%s: Wrong return value.");

    n = sscanf("  ", "%5c", name);
    fail_unless(n == EOF, "Error EOF %%s: Wrong return value.");

    n = sscanf("23 2", "%d 25", &i);
    fail_unless(n == 1, "Wrong return value.");
}
END_TEST

START_TEST(scanf_suppress)
{
    int n, a, i;
    char str1[50];
    char *str = "56 504 12345 099 405 string";

    n = sscanf(str, "%d %*d %*s %*d %d %*3c %*[ri] %s", &a, &i, str1);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(a == 56, "%%d (1) not calculated correctly.");
    fail_unless(i == 405, "%%d (2) not calculated correctly.");
    fail_unless(strcmp(str1, "ng") == 0, "%%[ not correctly parsed");
}
END_TEST

START_TEST(scanf_dioux)
{
    int n, d, i, ix;
    unsigned int o, u, x;
    char *str = "123 456 0x456 7723 7723 7723";

    n = sscanf(str, "%d %i %i %o %u %x", &d, &i, &ix, &o, &u, &x);

    fail_unless(n == 6, "Wrong return value.");
    fail_unless(d == 123, "%%d not calculated correctly.");
    fail_unless(i == 456, "%%i not calculated correctly.");
    fail_unless(ix == 0x456, "%%i not calculated correctly.");
    fail_unless(o == 07723, "%%o not calculated correctly.");
    fail_unless(u == 7723, "%%u not calculated correctly.");
    fail_unless(x == 0x7723, "%%x not calculated correctly.");
}
END_TEST

#ifndef CONFIG_WITHOUT_FLOATING
START_TEST(scanf_aefg)
{
    int n;
    float a, e, f, g, x;
    char *str = "1.23e-5 23.45 0x23.45p-22.33.45";

    n = sscanf(str, "%e %a %10f %3g %a", &e, &a, &f, &g, &x);

    fail_unless(n == 5, "Wrong return value.");
    fail_unless(e == (float)1.23e-5, "%%e not calculated correctly.");
    fail_unless(a == (float)23.45, "%%a not calculated correctly.");
    fail_unless(f == (float)0x23.45p-2, "%%f not calculated correctly.");
    fail_unless(g == (float)2.3, "%%g not calculated correctly."); 
    fail_unless(x == (float)3.45, "%%a (2) not calculated correctly.");
}
END_TEST
#endif

START_TEST(scanf_s)
{
    int n;
    char str1[50];
    char str2[50];
    char str3[50];
    char *str = "OK Labs.0765abcdef";

    n = sscanf(str, "%s %2s %s", str1, str2, str3);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(strcmp(str1, "OK") == 0, "%%s (str1) not correctly parsed.");
    fail_unless(strcmp(str2, "La") == 0, "%%s (str2) not correctly parsed.");
    fail_unless(strcmp(str3, "bs.0765abcdef") == 0, "%%s (str3) not correctly parsed.");
}
END_TEST

START_TEST(scanf_bracket)
{
    int n;
    char str1[50];
    char str2[50];
    char str3[50];
    char *str = "OK Labs].0765abcdef";

    n = sscanf(str, "%[OK ] %8[]Labs.076] %[^ef]", str1, str2, str3);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(strcmp(str1, "OK ") == 0, "%%[ (str1) not correctly parsed.");
    fail_unless(strcmp(str2, "Labs].07") == 0, "%%[ (str2) not correctly parsed.");
    fail_unless(strcmp(str3, "65abcd") == 0, "%%[ (str3) not correctly parsed.");
}
END_TEST

START_TEST(scanf_c)
{
    int n;
    char str1[50];
    char str2[50];
    char str3[50];
    char *str = "OK Labs].0765abcdef";

    n = sscanf(str, "%c %5c %*4c %6c", str1, str2, str3);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(strcmp(str1, "O") == 0, "%%c (str1) not correctly parsed.");
    fail_unless(strcmp(str2, "K Lab") == 0, "%%c (str2) not correctly parsed.");
    fail_unless(strcmp(str3, "765abc") == 0, "%%c (str3) not correctly parsed.");
}
END_TEST

START_TEST(scanf_pn)
{
    int n;
    int b;
    void *p;
    char *str = "0x13A46ef qder";

    n = sscanf(str, "%p %n", &p, &b);

    fail_unless(n == 1, "Wrong return value.");
    fail_unless(p == (void*)0x13a46ef, "%%p not correctly parsed.");
    fail_unless(b == 10, "%%n not calculated correctly.");
}
END_TEST

START_TEST(scanf_match)
{
    int n;
    char str1[50];
    char str2[50];
    int a;
    char *str = "O%K Labs]   .0765abcdef";

    n = sscanf(str, "O%%K %s .076%d%s", str1, &a, str2);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(strcmp(str1, "Labs]") == 0, "%%s not correctly parsed.");
    fail_unless(a == 5, "%%d not calculated correctly.");
    fail_unless(strcmp(str2, "abcdef") == 0, "%%s not correctly parsed.");
}
END_TEST

#ifndef CONFIG_WITHOUT_FLOATING
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
struct foo {
    int a;
    int b;
    int c;
    int d;
};

union foob {
    long double d;
    struct foo stf;
};

START_TEST(scanf_Lf)
{
    int n;
    float f;
    double d;
    //long double ld;
    union foob foo;
    char *str = "0x123p-10 0x123p-10 0x123p-10";

    foo.d = (long double)0x123p-10;
    printf("sizeof(int) = %d, sizeof(long double) = %d\n", (int)sizeof(int),
                                                   (int)sizeof(long double));
    printf("scanf_Lf = %x %x %x %x\n",
                foo.stf.a,
                foo.stf.b,
                foo.stf.c,
                foo.stf.d);

    n = sscanf(str, "%f %lf %Lf", &f, &d, &foo.d);

    printf("scanf_Lf = %x %x %x %x\n",
                foo.stf.a,
                foo.stf.b,
                foo.stf.c,
                foo.stf.d);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(foo.d == (long double)0x123p-10, "%%Lf not calculated correctly.");
    fail_unless(d == (double)0x123p-10, "%%lf not calculated correctly.");
    fail_unless(f == (float)0x123p-10, "%%f not calculated correctly.");
}
END_TEST
#endif /* CONFIG_WITHOUT_LONG_DOUBLE */
#endif /* CONFIG_WITHOUT_FLOATING */

START_TEST(scanf_varg)
{
    int i, n, j;
    char name[50];
    char *str = "56789 0123 56.72";

    n = sscanf(str, "%2$2d%1$d%*d %3$[0123456789]", &j, &i, name);

    fail_unless(n == 3, "Wrong return value.");
    fail_unless(i == 56, "%%d not calculated correctly.");
    fail_unless(j == 789, "%%f not calculated correctly.");
    fail_unless(strcmp(name, "56") == 0, "%%[ not correctly parsed.");
}
END_TEST

/*
 * TODO: When we have full LOCALE support, this test needs to be changed.
 */
START_TEST(strcoll_basic)
{
    char *abc = "abc";
    char *Abc = "Abc";
    char *ab = "ab";
    fail_unless(strcoll(abc, abc) == 0, "strcoll() == 0");
    fail_unless(strcoll(abc, ab) != 0, "strcoll() != 0");
    fail_unless(strcoll(Abc, abc) != 0, "strcoll() != 0");
}
END_TEST

START_TEST(strxfrm_test)
{
    char *str = "This is a strxfrm test... '1 @!3 43$#% ";
    char *str2 = "This is a strxfrm test2... '1 @!3 43$#% ";
    char buf[50];
    char buf2[50];
    int ret_cmp, ret_col;

    strxfrm(buf, str, 49);
    strxfrm(buf2, str2, 49);
    ret_cmp = strcmp(buf, buf2);
    ret_col = strcoll(str, str2);

    if (ret_cmp == 0) {
        fail_unless(ret_col == 0, "ret_cmp = 0, ret_col != 0");
    } else if (ret_cmp > 0) {
        fail_unless(ret_col > 0, "ret_cmp > 0, ret_col <= 0");
    } else {
        fail_unless(ret_col < 0, "ret_cmp < 0, ret_col >= 0");
    }
}
END_TEST

Suite *
make_test_libs_c_suite(void)
{
    Suite *suite;
    TCase *tc;

    suite = suite_create("libc tests");

    tc = tcase_create("strcmp");
    tcase_add_test(tc, strcmp_equal);
    tcase_add_test(tc, strcmp_nequal);
    tcase_add_test(tc, strcmp_different);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strncmp");
    tcase_add_test(tc, strncmp_equal);
    tcase_add_test(tc, strncmp_partially_equal);
    tcase_add_test(tc, strncmp_partially_different);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strstr");
    tcase_add_test(tc, strstr_exists);
    tcase_add_test(tc, strstr_empty);
    tcase_add_test(tc, strstr_nexists);
    suite_add_tcase(suite, tc);

    tc = tcase_create("memcmp");
    tcase_add_test(tc, memcmp_equal);
    tcase_add_test(tc, memcmp_unequal);
    suite_add_tcase(suite, tc);

    tc = tcase_create("memcpy");
    tcase_add_test(tc, memcpy_nooverlap);
    suite_add_tcase(suite, tc);

    tc = tcase_create("memmove");
    tcase_add_test(tc, memmove_noorwithoverlap);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strcpy");
    tcase_add_test(tc, strcpy_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strncpy");
    tcase_add_test(tc, strncpy_oversized_destination);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strlcpy");
    tcase_add_test(tc, strlcpy_test);
    tcase_add_test(tc, strlcpy_pathological);
    suite_add_tcase(suite, tc);

    tc = tcase_create("sprintf");
    tcase_add_test(tc, sprintf_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("snprintf");
    tcase_add_test(tc, snprintf_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("format");
    tcase_add_test(tc, format_left_justify);
    suite_add_tcase(suite, tc);

    tc = tcase_create("stdout");
    tcase_add_test(tc, stdout_fputs_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("stdio");
    tcase_add_test(tc, ungetc_test);
    tcase_add_test(tc, ungetc_multiple_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("memchr");
    tcase_add_test(tc, memchr_findorno);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strchr");
    tcase_add_test(tc, strchr_findorno);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strcat");
    tcase_add_test(tc, strcat_stickatend);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strncat");
    tcase_add_test(tc, strncat_stickatend);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strlcat");
    tcase_add_test(tc, strlcat_stickatend);
    tcase_add_test(tc, strlcat_emptytarget);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strcspn");
    tcase_add_test(tc, strcspn_inornot);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strpbrk");
    tcase_add_test(tc, strpbrk_inornot);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strrchr");
    tcase_add_test(tc, strrchr_inornot);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strspn");
    tcase_add_test(tc, strspn_inornot);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strtok_r");
    tcase_add_test(tc, strtok_r_split);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strtok");
    tcase_add_test(tc, strtok_split);
    suite_add_tcase(suite, tc);

    tc = tcase_create("memset");
    tcase_add_test(tc, memset_simple_test);
    tcase_add_test(tc, memset_wordsize_alignment);
    tcase_add_test(tc, memset_pagesize_alignment);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strlen");
    tcase_add_test(tc, strlen_length);
    suite_add_tcase(suite, tc);

    /* 2008-01-14: Test broken on PC99. See Mothra Issue #2311. */
#if 0
    tc = tcase_create("setjmp");
    tcase_add_test(tc, setjmp_simple);
    suite_add_tcase(suite, tc);
#endif

    tc = tcase_create("malloc");
    tcase_add_test(tc, malloc_simple);
    tcase_add_test(tc, malloc_lots);
    tcase_add_test(tc, free_simple);
    tcase_add_test(tc, calloc_simple);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strto");
    tcase_add_test(tc, strtoul_decimal);
    tcase_add_test(tc, strtoul_hex);
    tcase_add_test(tc, strtoul_upper_hex);
    suite_add_tcase(suite, tc);

    tc = tcase_create("fflush");
    tcase_add_test(tc, fflush_stdout);
    suite_add_tcase(suite, tc);

    tc = tcase_create("setvbuf");
    tcase_add_test(tc, setvbuf_stdout);
    suite_add_tcase(suite, tc);

    tc = tcase_create("imaxabs");
    tcase_add_test(tc, imaxabs_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("div");
    tcase_add_test(tc, div_tests);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strerror");
    tcase_add_test(tc, strerror_tests);
    suite_add_tcase(suite, tc);

    tc = tcase_create("gmtime_and_mktime");
    tcase_add_test(tc, gmtime_and_mktime);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strftime_weeks");
    tcase_add_test(tc, strftime_weeks);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strftime_week_month");
    tcase_add_test(tc, strftime_week_month);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strftime_year");
    tcase_add_test(tc, strftime_year);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strftime_date");
    tcase_add_test(tc, strftime_date);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strftime_hour");
    tcase_add_test(tc, strftime_hour);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strftime_misc");
    tcase_add_test(tc, strftime_misc);
    suite_add_tcase(suite, tc);

    tc = tcase_create("asctime");
    tcase_add_test(tc, asctime_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("tzset");
    tcase_add_test(tc, tzset_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strtol");
    tcase_add_test(tc, strtol_tests);
    suite_add_tcase(suite, tc);

#ifndef CONFIG_WITHOUT_FLOATING
    tc = tcase_create("strtof");
    tcase_add_test(tc, strtof_tests);
    suite_add_tcase(suite, tc);
#endif /* CONFIG_WITHOUT_FLOATING */

    tc = tcase_create("scanf");
    tcase_add_test(tc, scanf_error);
    tcase_add_test(tc, scanf_suppress);
    tcase_add_test(tc, scanf_dioux);
    tcase_add_test(tc, scanf_s);
    tcase_add_test(tc, scanf_bracket);
    tcase_add_test(tc, scanf_c);
    tcase_add_test(tc, scanf_pn);
    tcase_add_test(tc, scanf_match);
#ifndef CONFIG_WITHOUT_FLOATING
    tcase_add_test(tc, scanf_basic);
    tcase_add_test(tc, scanf_basic2);
    tcase_add_test(tc, scanf_basic3);
    tcase_add_test(tc, scanf_aefg);
#ifndef CONFIG_WITHOUT_LONG_DOUBLE
    tcase_add_test(tc, scanf_Lf);
#endif /* CONFIG_WITHOUT_LONG_DOUBLE */
#endif /* CONFIG_WITHOUT_FLOATING */
    tcase_add_test(tc, scanf_varg);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strcoll");
    tcase_add_test(tc, strcoll_basic);
    suite_add_tcase(suite, tc);

    tc = tcase_create("strxfrm");
    tcase_add_test(tc, strxfrm_test);
    suite_add_tcase(suite, tc);

    tc = tcase_create("environment");
    tcase_add_test(tc, environment);
    suite_add_tcase(suite, tc);

    //fs_test_init(suite);

    return suite;
}
