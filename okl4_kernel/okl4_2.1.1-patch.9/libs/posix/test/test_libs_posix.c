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

/* TODO: add artistic license from Open Group */

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <l4/thread.h>
#include "test_libs_posix.h"
#include <errno.h>

void * null_pthread(void *arg);
void *
null_pthread(void *arg)
{
    pthread_exit(arg);
    return NULL;
}


/*****************************************/
/* Test creation, cancellation, and join */
/*****************************************/

void* pthread_create_1_t(void*);
void* pthread_create_1_t(void* arg)
{
    int ret;
    void *retp;
    int oldstate;
    retp = (void *) 1;
    int err = 0;

    ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldstate);
    fail_unless(ret==0, "pthread_setcanceltype failed");
    if (ret == 0) {
        fail_unless(oldstate==PTHREAD_CANCEL_DEFERRED,
                    "pthread_setcanceltype returned an old state of async");
        if (oldstate != PTHREAD_CANCEL_DEFERRED) {
            err = 1;
        }
    } else {
        err = 1;
    }
    ret = pthread_cancel(pthread_self());

    fail_unless(ret==0, "pthread_cancel failed");
    if (ret != 0) {
        err = 1;
    }

    if (err) {
        retp = (void *) 2;
    }
    pthread_exit(retp);

    return(retp);
}

START_TEST(pthread_create_1)
{
    pthread_t pthread;
    int ret;
    void *retp;

    ret = pthread_create(&pthread, NULL, &pthread_create_1_t, NULL);

    fail_unless(ret == 0, "pthread_create failed");

    ret = pthread_join(pthread, &retp);

    fail_unless(ret == 0, "pthread_join failed");

    if (ret == 0) {
        fail_unless(retp == (void *) PTHREAD_CANCELED,
                    "pthread_join didn't return cancellation value");
    } else {
        (void) pthread_cancel(pthread);
        (void) pthread_detach(pthread);
    }
}
END_TEST


/*****************************************/
/*  Test creation, join and key set/get  */
/*****************************************/

pthread_key_t pthread_key_1_key;
#define PTHREAD_KEY_1_KEY_DATA 0xBEADCAFE

void * pthread_create_2_t(void *arg);

void *
pthread_create_2_t(void *arg)
{
    void *ret = pthread_getspecific(pthread_key_1_key);
    fail_unless(ret==NULL, "pthread_getspecific returned unexpected value"
                "(expected NULL, nothing set for this thread)");
    pthread_exit(NULL);
    return NULL;
}

START_TEST(pthread_create_2)
{
    int ret;
    pthread_t pthread;
    void *retp;

    ret = pthread_key_create(&pthread_key_1_key, NULL);
    fail_unless(ret==0, "pthread_key_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_setspecific(pthread_key_1_key, (void*)PTHREAD_KEY_1_KEY_DATA);
    fail_unless(ret==0, "pthread_setspecific failed");
    if (ret != 0) {
        (void) pthread_key_delete(pthread_key_1_key);
        return;
    }

    ret = pthread_create(&pthread, NULL, pthread_create_2_t, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        (void) pthread_key_delete(pthread_key_1_key);
        return;
    }

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret == 0) {
        fail_unless(retp==NULL, "pthread_join returned unexpected value");
    } else {
        (void) pthread_cancel(pthread);
        (void) pthread_detach(pthread);
    }

    retp = pthread_getspecific(pthread_key_1_key);
    fail_unless(retp==(void*)PTHREAD_KEY_1_KEY_DATA, "pthread_getspecific returned unexpected value (expected PTHREAD_KEY_1_KEY_DATA)");

    (void) pthread_key_delete(pthread_key_1_key);
}
END_TEST


/*****************************************/
/* Test attr + thread creation and join  */
/*****************************************/

START_TEST(pthread_create_3)
{
    int ret;
    pthread_attr_t attr;
    pthread_t pthread;
    void *retp;

    ret = pthread_attr_init(&attr);
    fail_unless(ret==0, "pthread_attr_init failed");

    ret = pthread_create(&pthread, &attr, &null_pthread, (void *)123);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret == 0) {
        fail_unless(retp == (void*)123, "pthread_join returned unexpected value from joining null_pthread");
    } else {
        (void) pthread_cancel(pthread);
        (void) pthread_detach(pthread);
    }

    (void) pthread_attr_destroy(&attr);
}
END_TEST


/*****************************************/
/*     Test detach state attribute       */
/*****************************************/

START_TEST(pthread_create_4)
{
    int ret;
    pthread_t pthread;
    void *retp;
    pthread_attr_t attr;

    ret = pthread_attr_init(&attr);
    fail_unless(ret==0, "pthread_attr_init failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    fail_unless(ret==0, "pthread_attr_setdetachstate failed");
    if (ret != 0) {
        pthread_attr_destroy(&attr);
        return;
    }

    ret = pthread_create(&pthread, &attr, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        pthread_attr_destroy(&attr);
        return;
    }

    /* This join should fail */
    ret = pthread_join(pthread, &retp);
    fail_unless(ret != 0, "pthread_join succeeded unexpectedly");

    pthread_attr_destroy(&attr);
}
END_TEST


/*****************************************/
/*  Test default detach state attribute  */
/*****************************************/

START_TEST(pthread_create_5)
{
    int ret;
    pthread_t pthread;
    void *valp;

    ret = pthread_create(&pthread, NULL, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread, &valp);
    fail_unless(ret==0, "pthread_join failed - is default detachstate not PTHREAD_CREATE_JOINABLE?");
}
END_TEST


/*****************************************/
/*  Test default cancel state and type   */
/* and setting/getting cancel state/type */
/*****************************************/

START_TEST(pthread_create_6)
{
    int ret, cancelstate, canceltype;

    ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cancelstate);
    fail_unless(ret==0, "pthread_setcancelstate failed");
    fail_unless(cancelstate == PTHREAD_CANCEL_ENABLE, "Unexpected default cancel state");
    ret = pthread_setcancelstate(cancelstate, &cancelstate);
    fail_unless(cancelstate == PTHREAD_CANCEL_DISABLE, "Unexpected cancel state after set");

    ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &canceltype);
    fail_unless(ret==0, "pthread_setcanceltype failed");
    fail_unless(canceltype == PTHREAD_CANCEL_DEFERRED, "Unexpected default cancel type");
    ret = pthread_setcanceltype(canceltype, &canceltype);
    fail_unless(canceltype == PTHREAD_CANCEL_ASYNCHRONOUS, "Unexpected cancel type after set");
}
END_TEST


/*****************************************/
/*        pthread_equal tests            */
/*****************************************/

START_TEST(pthread_equal_1)
{
    int ret;
    pthread_t pthread;
    pthread = pthread_self();
    ret = pthread_equal(pthread, pthread);
    fail_unless(ret != 0, "pthread_equal failed");
}
END_TEST

START_TEST(pthread_equal_2)
{
    int ret;
    void *retp;
    pthread_t pthread1, pthread2;

    pthread1 = pthread_self();

    ret = pthread_create(&pthread2, NULL, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread2, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret == 0) {
        fail_unless(retp==NULL, "pthread_join returned unexpected exit value");
    }

    ret = pthread_equal(pthread1, pthread2);
    fail_unless(ret==0, "pthread_equal returned true on a unequal threads");
}
END_TEST


/******************************************/
/* Test detach fails on a detached thread */
/******************************************/

START_TEST(pthread_detach_1)
{
    int ret;
    pthread_t pthread;
    void *retp;

    ret = pthread_create(&pthread, NULL, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret == 0) {
        fail_unless(retp==NULL, "pthread_join returned unexpected exit value");

        /* Detach should not succeed - the thread has already detached */
        ret = pthread_detach(pthread);
        fail_unless(ret==ESRCH, "pthread_detach did not return ESRCH on an un-detachable thread");

    } else {
        pthread_cancel(pthread);
        pthread_detach(pthread);
    }
}
END_TEST


/********************************************/
/* Test detach succeeds on an exited thread */
/********************************************/

START_TEST(pthread_detach_2)
{
    int ret;
    pthread_t pthread;

    ret = pthread_create(&pthread, NULL, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }
    ret = pthread_detach(pthread);
    fail_unless(ret==0, "pthread_detach failed");
}
END_TEST


/*************************************************/
/* Test cancellation handlers are called on exit */
/*************************************************/
int pthread_exit_1_val;

void * pthread_exit_1_t(void *arg);
void pthread_exit_1_c(void *arg);

START_TEST(pthread_exit_1)
{
    int ret;
    void *retp;
    pthread_t pthread;

    pthread_exit_1_val = 0;

    ret = pthread_create(&pthread, NULL, pthread_exit_1_t, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret == 0) {
        fail_unless(retp==NULL, "pthread_join returned unexpected exit value");
    }

    fail_unless(pthread_exit_1_val==1, "cleanup handler failed to execute");
}
END_TEST

void *
pthread_exit_1_t(void *arg)
{
    pthread_cleanup_push(pthread_exit_1_c, NULL);
    pthread_exit(NULL);
    pthread_cleanup_pop(0);

    return NULL;
}

void
pthread_exit_1_c(void *arg)
{
    pthread_exit_1_val = 1;
}


/**********************************************************/
/* Test cancellation handlers and key set/get/destructors */
/**********************************************************/

int pthread_exit_2_cflag;
int pthread_exit_2_dflag;
pthread_key_t pthread_exit_2_key1;
pthread_key_t pthread_exit_2_key2;

void * pthread_exit_2_t(void *arg);
void pthread_exit_2_c1(void *arg);
void pthread_exit_2_c2(void *arg);
void pthread_exit_2_d(void *arg);

START_TEST(pthread_exit_2)
{
    int ret;
    void *retp;
    pthread_t pthread;

    pthread_exit_2_cflag = 0;
    pthread_exit_2_dflag = 0;

    ret = pthread_create(&pthread, NULL, &pthread_exit_2_t, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret != 0) {
        pthread_cancel(pthread);
        pthread_detach(pthread);
    }

    pthread_key_delete(pthread_exit_2_key1);
    pthread_key_delete(pthread_exit_2_key2);
}
END_TEST

START_TEST(pthread_exit_2_after)
{
    fail_unless(pthread_exit_2_cflag == 2, "pthread_exit_2_cflag is wrong value");
    fail_unless(pthread_exit_2_dflag == 2, "pthread_exit_2_dflag is wrong value");
}
END_TEST

void *
pthread_exit_2_t(void *arg)
{
    int ret;
    void *retp = NULL;

    pthread_cleanup_push(&pthread_exit_2_c1, NULL);
    pthread_cleanup_push(&pthread_exit_2_c2, NULL);

    ret = pthread_key_create(&pthread_exit_2_key1, &pthread_exit_2_d);
    fail_unless(ret==0, "pthread_key_create failed");
    if (ret == 0) {
        ret = pthread_setspecific(pthread_exit_2_key1, (void*)&pthread_exit_2_key1);
        fail_unless(ret==0, "pthread_setspecific failed");
        if (ret == 0) {
            ret = pthread_key_create(&pthread_exit_2_key2, &pthread_exit_2_d);
            fail_unless(ret==0, "pthread_key_create failed");
            if (ret == 0) {
                ret = pthread_setspecific(pthread_exit_2_key2, (void*) &pthread_exit_2_key2);
                fail_unless(ret==0, "pthread_setspecific failed");
            }
        }
    }

    (void) pthread_exit(retp);

    /* should not be reached */
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return retp;
}

void
pthread_exit_2_c1(void *arg)
{
    fail_unless(pthread_exit_2_dflag==0, "Destructors were called before cleanup functions");
    fail_unless(pthread_exit_2_cflag == 1, "Cleanup handlers were called in wrong order");
    ++pthread_exit_2_cflag;
}

void
pthread_exit_2_c2(void *arg)
{
    fail_unless(pthread_exit_2_dflag==0, "Destructors were called before cleanup functions");
    fail_unless(pthread_exit_2_cflag == 0, "Cleanup handlers were called in wrong order");
    if ( pthread_exit_2_cflag == 0 ) {
        ++pthread_exit_2_cflag;
    }
}

void
pthread_exit_2_d(void *arg)
{
    int ret;
    pthread_key_t *pk;

    pk = arg;

    if (pk != NULL) {
        ret = pthread_setspecific((pthread_key_t) *pk, NULL);
        fail_unless(ret==0, "pthread_setspecific failed");
    }
    ++pthread_exit_2_dflag;
}

/**********************************************************
 * Test cancellation handlers and key set/get/destructors,
 * and test that there is an implicit call the exit() on
 * return from the start function (for threads other than
 * the thread that executed main)
 * Same as exit_2, but doesn't explicitly call exit()
 **********************************************************/

int pthread_exit_3_cflag;
int pthread_exit_3_dflag;
pthread_key_t pthread_exit_3_key1;
pthread_key_t pthread_exit_3_key2;

void * pthread_exit_3_t(void *arg);
void pthread_exit_3_c1(void *arg);
void pthread_exit_3_c2(void *arg);
void pthread_exit_3_d(void *arg);

START_TEST(pthread_exit_3)
{
    int ret;
    void *retp;
    pthread_t pthread;

    pthread_exit_3_cflag = 0;
    pthread_exit_3_dflag = 0;

    ret = pthread_create(&pthread, NULL, &pthread_exit_3_t, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");
    if (ret != 0) {
        pthread_cancel(pthread);
        pthread_detach(pthread);
    }

    pthread_key_delete(pthread_exit_3_key1);
    pthread_key_delete(pthread_exit_3_key2);
}
END_TEST

START_TEST(pthread_exit_3_after)
{
    fail_unless(pthread_exit_3_cflag == 2, "pthread_exit_3_cflag is wrong value");
    fail_unless(pthread_exit_3_dflag == 2, "pthread_exit_3_dflag is wrong value");
}
END_TEST

void *
pthread_exit_3_t(void *arg)
{
    int ret;
    void *retp = NULL;

    pthread_cleanup_push(&pthread_exit_3_c1, NULL);
    pthread_cleanup_push(&pthread_exit_3_c2, NULL);

    ret = pthread_key_create(&pthread_exit_3_key1, &pthread_exit_3_d);
    fail_unless(ret==0, "pthread_key_create failed");
    if (ret == 0) {
        ret = pthread_setspecific(pthread_exit_3_key1, (void*)&pthread_exit_3_key1);
        fail_unless(ret==0, "pthread_setspecific failed");
        if (ret == 0) {
            ret = pthread_key_create(&pthread_exit_3_key2, &pthread_exit_3_d);
            fail_unless(ret==0, "pthread_key_create failed");
            if (ret == 0) {
                ret = pthread_setspecific(pthread_exit_3_key2, (void*) &pthread_exit_3_key2);
                fail_unless(ret==0, "pthread_setspecific failed");
            }
        }
    }

    return retp;

    /* should not be reached */
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return retp;
}

void
pthread_exit_3_c1(void *arg)
{
    fail_unless(pthread_exit_3_dflag==0, "Destructors were called before cleanup functions");
    fail_unless(pthread_exit_3_cflag == 1, "Cleanup handlers were called in wrong order");
    ++pthread_exit_3_cflag;
}

void
pthread_exit_3_c2(void *arg)
{
    fail_unless(pthread_exit_3_dflag==0, "Destructors were called before cleanup functions");
    fail_unless(pthread_exit_3_cflag == 0, "Cleanup handlers were called in wrong order");
    if ( pthread_exit_3_cflag == 0 ) {
        ++pthread_exit_3_cflag;
    }
}

void
pthread_exit_3_d(void *arg)
{
    int ret;
    pthread_key_t *pk;

    pk = arg;

    if (pk != NULL) {
        ret = pthread_setspecific((pthread_key_t) *pk, NULL);
        fail_unless(ret==0, "pthread_setspecific failed");
    }
    ++pthread_exit_3_dflag;
}


/**********************************************************
 * Test pthread_join doesn't work with a detached thread
 **********************************************************/

START_TEST(pthread_join_1)
{
    int ret;
    void *retp;
    pthread_t pthread;
    pthread_attr_t attr;

    ret = pthread_attr_init(&attr);
    fail_unless(ret==0, "pthread_attr_init failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    fail_unless(ret==0, "pthread_attr_setdetachstate failed");
    if (ret != 0) {
        pthread_attr_destroy(&attr);
        return;
    }
    
    ret = pthread_create(&pthread, &attr, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }
    
    ret = pthread_join(pthread, &retp);
    fail_unless(ret == EINVAL, "pthread_join returned success on an unjoinable thread");

    pthread_attr_destroy(&attr);
}
END_TEST


/**********************************************************
 * Test pthread_join doesn't work twice on same thread
 **********************************************************/

START_TEST(pthread_join_2)
{
    int ret;
    void *retp;
    pthread_t pthread;
    
    ret = pthread_create(&pthread, NULL, &null_pthread, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }
    
    ret = pthread_join(pthread, &retp);
    fail_unless(ret == 0, "pthread_join failed");

    ret = pthread_join(pthread, &retp);
    fail_unless(ret == ESRCH, "Second pthread_join did not return ESRCH");
}
END_TEST

/**********************************************************
 * Test pthread_join with self returns EDEADLK
 **********************************************************/

START_TEST(pthread_join_3)
{
    int ret;
    void *retp;
    
    ret = pthread_join(pthread_self(), &retp);
    fail_unless(ret == EDEADLK,
                "pthread_join failed to return EDEADLK on attempt to join with self");

}
END_TEST

/**********************************************************
 * Test pthread_join deadlock detection with two threads
 **********************************************************/

void* pthread_join_4_t1(void*);
void* pthread_join_4_t2(void*);
pthread_t pthread_join_4_t1tid;
pthread_t pthread_join_4_t2tid;
int pthread_join_4_t1ret, pthread_join_4_t2ret;
volatile int pthread_join_4_t1status;
volatile int pthread_join_4_t2status;

enum pthread_join_4_status {
    PTHREAD_JOIN_4_INIT = 0,
    PTHREAD_JOIN_4_CURRENT = 1,
    PTHREAD_JOIN_4_DONE = 2,
};

START_TEST(pthread_join_4)
{
    int ret;

    /* Create two threads */
    ret = pthread_create(&pthread_join_4_t1tid, NULL, &pthread_join_4_t1, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    ret = pthread_create(&pthread_join_4_t2tid, NULL, &pthread_join_4_t2, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    /* Spin until they are done */
    while (pthread_join_4_t1status != PTHREAD_JOIN_4_DONE ||
           pthread_join_4_t2status != PTHREAD_JOIN_4_DONE) { }
    
    /* Ensure one of their joined succeeded and the other failed with an EDEADLK */
    fail_unless((pthread_join_4_t1ret == EDEADLK && pthread_join_4_t2ret == 0) ||
                (pthread_join_4_t2ret == EDEADLK && pthread_join_4_t1ret == 0),
                "pthread_join failed to detect deadlock between two threads");

}
END_TEST

void*
pthread_join_4_t1(void* arg)
{
    void* retp = NULL;

    pthread_join_4_t1status = PTHREAD_JOIN_4_CURRENT;

    /* Wait till t2 is created */
    while (pthread_join_4_t2status != PTHREAD_JOIN_4_CURRENT) { }

    /* Try to join t2 */
    pthread_join_4_t1ret = pthread_join(pthread_join_4_t2tid, &retp);

    pthread_join_4_t1status = PTHREAD_JOIN_4_DONE;
    return retp;
}

void*
pthread_join_4_t2(void* arg)
{
    void* retp = NULL;

    pthread_join_4_t2status = PTHREAD_JOIN_4_CURRENT;

    /* Wait till t1 is created */
    while (pthread_join_4_t1status != PTHREAD_JOIN_4_CURRENT) { }

    /* Try to join t1 */
    pthread_join_4_t2ret = pthread_join(pthread_join_4_t1tid, &retp);

    pthread_join_4_t2status = PTHREAD_JOIN_4_DONE;
    return retp;
}


/*****************************************************************
 * Test pthread_once calls the once function (just main thread)
 *****************************************************************/

void pthread_once_1_c(void);
int pthread_once_1_flag = 0;
pthread_once_t pthread_once_1_control = PTHREAD_ONCE_INIT;

START_TEST(pthread_once_1)
{
    int ret;
    ret = pthread_once(&pthread_once_1_control, &pthread_once_1_c);
    fail_unless(ret==0, "pthread_once failed");
    fail_unless(pthread_once_1_flag == 1, "pthread_once did not execute function");
}
END_TEST

void
pthread_once_1_c(void)
{
    pthread_once_1_flag++;
}

/************************************************************************
 * Test pthread_once calls the once function just once (just main thread)
 ************************************************************************/

void pthread_once_2_c(void);
int pthread_once_2_flag = 0;
pthread_once_t pthread_once_2_control = PTHREAD_ONCE_INIT;

START_TEST(pthread_once_2)
{
    int ret;

    /* first call, should execute */
    ret = pthread_once(&pthread_once_2_control, &pthread_once_2_c);
    fail_unless(ret==0, "pthread_once failed");

    /* second call, shouldn't execute */
    ret = pthread_once(&pthread_once_2_control, &pthread_once_2_c);
    fail_unless(ret==0, "pthread_once failed");

    fail_unless(pthread_once_2_flag == 1, "pthread_once executed function twice");
}
END_TEST

void
pthread_once_2_c(void)
{
    pthread_once_2_flag++;
}

/************************************************************************
 * Test pthread_once calls the once function just once (with two threads)
 ************************************************************************/

void pthread_once_3_c(void);
void* pthread_once_3_t(void*);
int pthread_once_3_flag = 0;
pthread_once_t pthread_once_3_control = PTHREAD_ONCE_INIT;

START_TEST(pthread_once_3)
{
    int ret;
    pthread_t pthread_1, pthread_2;
    void* retp;

    /* Create two threads, both of which will execute pthread_once */
    ret = pthread_create(&pthread_1, NULL, &pthread_once_3_t, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }
    ret = pthread_create(&pthread_2, NULL, &pthread_once_3_t, NULL);
    fail_unless(ret==0, "pthread_create failed");
    if (ret != 0) {
        return;
    }

    /* Wait until they both finish */
    ret = pthread_join(pthread_1, &retp);
    ret = pthread_join(pthread_2, &retp);

    /* Ensure only one thread executed the pthread once init function */
    fail_unless(pthread_once_3_flag == 1, "pthread_once executed function twice");
}
END_TEST

void*
pthread_once_3_t(void* arg)
{
    int ret;
    ret = pthread_once(&pthread_once_3_control, &pthread_once_3_c);
    fail_unless(ret==0, "pthread_once failed");

    return arg;
}

void
pthread_once_3_c(void)
{
    pthread_once_3_flag++;
}

/************************************************************************
 * Test that if the init_routine passed to pthread_once is a cancellation
 * point, and pthread_once is called by a thread that is then cancelled
 * whilst executing the init_routine, that pthread_once does not get
 * marked as done
 ************************************************************************/

void * pthread_once_4_t(void *);
void pthread_once_4_c(void *);
void pthread_once_4_o(void);
int pthread_once_4_flag;
pthread_once_t pthread_once_4_control = PTHREAD_ONCE_INIT;

START_TEST(pthread_once_4)
{
    int ret;
    void *retp;
    pthread_t pthread;

    pthread_once_4_flag = 0;

    ret = pthread_create(&pthread, NULL, &pthread_once_4_t, NULL);
    fail_unless(ret==0, "pthread_create failed");

    ret = pthread_join(pthread, &retp);
    fail_unless(ret==0, "pthread_join failed");

    fail_unless(retp == (void*)PTHREAD_CANCELED, "Canceled thread didn't return canceled value");

    /* If pthread_once didn't respect cancelation, the flag would be 2 */
    fail_unless(pthread_once_4_flag == 1,
                "pthread_once didn't call init function or called it twice (and didn't respect cancelation)");
}
END_TEST

void *
pthread_once_4_t(void *arg)
{
    int ret;
    void *retp;
    pthread_t mypthread;

    retp = (void *) 1;
    mypthread = pthread_self();

    pthread_cleanup_push(pthread_once_4_c, NULL);

    /* Cancel ourselves, with deferred cancelation */
    ret = pthread_cancel(mypthread);
    fail_unless(ret==0, "pthread_cancel failed");

    /* Call pthread_once with test cancel - since we previously called cancel,
     * this should cancel us and not modify the once_control */
    ret = pthread_once(&pthread_once_4_control, &pthread_testcancel);
    fail_unless(ret==0, "pthread_once failed");
    pthread_cleanup_pop(0);

    pthread_exit((void*)1);

    return((void*)1);
}

void
pthread_once_4_c(void *arg)
{
    int ret;
    ret = pthread_once(&pthread_once_4_control, &pthread_once_4_o);
    fail_unless(ret == 0, "pthread_once failed");
}

void
pthread_once_4_o(void)
{
    pthread_once_4_flag++;
}

/*****************************************/
/*        Synchronisation tests          */
/*****************************************/

/****** Create a thread, synchronize, then cancel it. Check that the thread
 was cancelled.       ******/

int synch1_flag;
pthread_mutex_t synch1_mx;
pthread_cond_t synch1_cv;

void* pthread_synch_1_t1(void *arg);

START_TEST(pthread_synch_1)
{
    int rval;
    pthread_t tid;
    void *tval;

    synch1_flag = 0;

    rval = pthread_mutex_init(&synch1_mx, NULL);
    fail_unless(rval==0, "pthread_mutex_init returned non-zero value");

    rval = pthread_cond_init(&synch1_cv, NULL);
    fail_unless(rval==0, "pthread_cond_init returned non-zero value");
    if (rval != 0) {
        (void) pthread_mutex_destroy(&synch1_mx);
        return;
    }

    rval = pthread_mutex_lock(&synch1_mx);
    fail_unless(rval==0, "pthread_mutex_lock returned non-zero value");
    if (rval != 0) {
        (void) pthread_mutex_destroy(&synch1_mx);
        (void) pthread_cond_destroy(&synch1_cv);
        return;
    }

    rval = pthread_create(&tid, NULL, pthread_synch_1_t1, NULL);
    fail_unless(rval==0, "pthread_create returned non-zero value");
    if (rval != 0) {
        (void) pthread_mutex_destroy(&synch1_mx);
        (void) pthread_cond_destroy(&synch1_cv);
        return;
    }

    while (!synch1_flag) {
        rval = pthread_cond_wait(&synch1_cv, &synch1_mx);
        fail_unless(rval==0, "pthread_cond_wait returned non-zero value");
    }

    rval = pthread_cancel(tid);
    fail_unless(rval==0, "pthread_cancel returned non-zero value");

    rval= pthread_mutex_unlock(&synch1_mx);
    fail_unless(rval==0, "pthread_mutex_unlock returned non-zero value");

    rval = pthread_join(tid, &tval);
    fail_unless(rval==0, "pthread_join returned non-zero value");
    if (rval == 0) {
        fail_unless(tval != (void*)PTHREAD_CANCELED, "pthread_join did not provide PTHREAD_CANCELED");
    } else {
        (void) pthread_cancel(tid);
        (void) pthread_detach(tid);
    }

    (void) pthread_mutex_destroy(&synch1_mx);
    (void) pthread_cond_destroy(&synch1_cv);
}
END_TEST

void *
pthread_synch_1_t1(void *arg)
{
    int rval;
    int nerrs;
    int nfails;
    void *tval;

    nerrs = 0;
    nfails = 0;
    tval = (void *) 1;

    rval = pthread_mutex_lock(&synch1_mx);
    fail_unless(rval==0, "pthread_mutex_lock returned non-zero value");

    ++synch1_flag;

    rval = pthread_cond_signal(&synch1_cv);
    fail_unless(rval==0, "pthread_cond_signal returned non-zero value");

    /* we get cancelled while blocked in this wait */
    rval = pthread_cond_wait(&synch1_cv, &synch1_mx);
    fail_unless(rval==0, "pthread_cond_wait returned non-zero value");

    rval = pthread_mutex_unlock(&synch1_mx);
    fail_unless(rval==0, "pthread_mutex_unlock returned non-zero value");

    if (nfails) {
        tval = (void *) 2;
    } else if (( nerrs == 0 ) && ( nfails == 0 )) {
        tval = NULL;
    }

    pthread_exit(tval);

    /* NOTREACHED */
    return(tval);
}


/*****************************************/
/*        Initialize all tests           */
/*****************************************/

Suite *
make_test_libs_posix_suite(void)
{
    Suite *suite;
    TCase *tc;

    suite = suite_create("libposix tests");

    tc = tcase_create("pthread_create");
    tcase_add_test(tc, pthread_create_1);
    tcase_add_test(tc, pthread_create_2);
    tcase_add_test(tc, pthread_create_3);
    tcase_add_test(tc, pthread_create_4);
    tcase_add_test(tc, pthread_create_5);
    tcase_add_test(tc, pthread_create_6);
    suite_add_tcase(suite, tc);

    tc = tcase_create("pthread_equal");
    tcase_add_test(tc, pthread_equal_1);
    tcase_add_test(tc, pthread_equal_2);
    suite_add_tcase(suite, tc);

    tc = tcase_create("pthread_detach");
    tcase_add_test(tc, pthread_detach_1);
    tcase_add_test(tc, pthread_detach_2);
    suite_add_tcase(suite, tc);

    tc = tcase_create("pthread_exit");
    tcase_add_test(tc, pthread_exit_1);
    tcase_add_test(tc, pthread_exit_2);
    tcase_add_test(tc, pthread_exit_2_after);
    tcase_add_test(tc, pthread_exit_3);
    tcase_add_test(tc, pthread_exit_3_after);
    suite_add_tcase(suite, tc);

    tc = tcase_create("pthread_join");
    tcase_add_test(tc, pthread_join_1);
    tcase_add_test(tc, pthread_join_2);
    tcase_add_test(tc, pthread_join_3);
    tcase_add_test(tc, pthread_join_4);
    suite_add_tcase(suite, tc);

    tc = tcase_create("pthread_once");
    tcase_add_test(tc, pthread_once_1);
    tcase_add_test(tc, pthread_once_2);
    tcase_add_test(tc, pthread_once_3);
    tcase_add_test(tc, pthread_once_4);
    suite_add_tcase(suite, tc);

    /* Synch fails due to incorrect handling of cancellation points */
    tc = tcase_create("pthread_synch");
    tcase_add_test(tc, pthread_synch_1);
    //suite_add_tcase(suite, tc);

    return suite;
}
