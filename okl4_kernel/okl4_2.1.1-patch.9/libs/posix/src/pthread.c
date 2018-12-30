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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <threadstate.h>
#include <mutex/mutex.h>

#include <posix/pthread.h>
#include "pthread_internal.h"

#include <iguana/tls.h>
#include <iguana/thread.h>
#include <iguana/env.h>

#include <l4/thread.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>

/* Problem: this should be unique in the POSIX environment. */
#define PTHREAD_SIGNAL_CODE 0x2

/* Private variables */
static pthread_t pthread_list;
static okl4_mutex_t pthread_list_mutex;
static unsigned int key_generation;
static okl4_mutex_t key_mutex; 
static pthread_key_t key_free_slots;

/* Private functions */
static inline void pthread_alert_die(pthread_t pthread);
static inline void pthread_delete(pthread_t pthread);
static void pthread_exit_with_cancel(void); /* Can't be inlined: we jump to it using exreg */
static int pthread_join_deadlocksearch(pthread_t pthread, pthread_t find_joinee);
static inline int pthread_is_valid(pthread_t pthread);
static inline void pthread_list_insert(pthread_t pthread);
static inline void pthread_list_remove(pthread_t pthread);
static inline void pthread_attr_default(struct pthread_attr_i *);
static inline void pthread_lock(pthread_t pthread) { okl4_mutex_lock(pthread->mutex); }
static inline void pthread_unlock(pthread_t pthread) { okl4_mutex_unlock(pthread->mutex); }
static inline void key_op_lock(void) { okl4_mutex_lock(key_mutex); }
static inline void key_op_unlock(void) { okl4_mutex_unlock(key_mutex); }
static inline void pthread_list_lock(void) { okl4_mutex_lock(pthread_list_mutex); }
static inline void pthread_list_unlock(void) { okl4_mutex_unlock(pthread_list_mutex); }

/*************************************
 *  Pthread Library Initialization   *
 *************************************/
/* Performs any required initialization */
void
__pthread_lib_init()
{
    /* Initialize the list lock */
    pthread_list_mutex = malloc(sizeof(struct okl4_mutex));
    okl4_mutex_init(pthread_list_mutex);

    /* Initialize the key lock and lock it */
    key_mutex = malloc(sizeof(struct okl4_mutex));
    okl4_mutex_init(key_mutex);
    key_op_lock();

    /* Initialize the key generation */
    key_generation = 0;

    /* Initialize all free key slots
       This is a performance consideration - we consider
       it faster to allocate all free slots now rather
       than having to malloc/free every time a key is created */
    key_free_slots = malloc(sizeof(struct pthread_key_i) * _POSIX_THREAD_KEYS_MAX);
    pthread_key_t key_c = key_free_slots;
    for (unsigned int i=0; i<_POSIX_THREAD_KEYS_MAX; i++) {
        key_c->status = __PTHREAD_KEY_FREE;
        key_c->slot = i;
        key_c->next = (i==_POSIX_THREAD_KEYS_MAX-1) ? NULL : (key_c + 1);
        key_c++;
    }

    /* Release the lock */
    key_op_unlock();

    /* We want the main thread to have a pthread state struct in its TLS */
    struct thread_state * thread_state = thread_state_get_base();
    thread_state->pthread = __pthread_state_new(NULL);
}

//TODO: pthread_lib_destruct or similar, should free all key slots etc.

/*************************************
 *         Debug Printing            *
 *************************************/

static inline void
pthread_print(pthread_t pthread)
{
    printf("pthread %p:\n", pthread);
    printf("\t->l4tid: %x\n", (unsigned int)pthread->l4tid.raw);
    printf("\t->mutex: %p\n", pthread->mutex);
    printf("\t->termination_state: %d\n", pthread->termination_state);
    printf("\t->exit_status: %p\n", pthread->exit_status);
    printf("\t->detach_status: %d\n", pthread->detach_status);
    printf("\t->joined_stack: %p\n", pthread->joined_stack);
    printf("\t->cancel_state: %d\n", pthread->cancel_state);
    printf("\t->cancel_type: %d\n", pthread->cancel_type);
    printf("\t->cancel_request: %d\n", pthread->cancel_request);
    printf("\t->cleanup_stack: %p\n", pthread->cleanup_stack);
    printf("\t->once_exec_stack: %p\n", pthread->once_exec_stack);
    printf("\t->keys: [");
    for(int i=0; i<PTHREAD_KEYS_MAX-1; i++) printf("%p,", pthread->keys[i]);
    printf("%p]\n", pthread->keys[PTHREAD_KEYS_MAX-1]);
    printf("\t->next: %p\n", pthread->next);
    printf("\t->previous: %p\n", pthread->previous);
}

static inline void
pthread_list_print(void)
{
    printf("pthread list: ");
    pthread_list_lock();
    pthread_t pthread_last = NULL;
    for (pthread_t pthread_c = pthread_list;
         pthread_c != NULL; pthread_c = pthread_c->next) {
        printf("%p -> ", pthread_c);
        pthread_last = pthread_c;
    }
    printf("\npthread_list backwards:");
    for (pthread_t pthread_c = pthread_last;
         pthread_c != NULL; pthread_c = pthread_c->previous) {
        printf("%p <- ", pthread_c);
    }
    printf("\n");
    pthread_list_unlock();
}


/*************************************
 *       Pthread list handling       *
 *************************************/

/* Determines if the pthread exists */
static inline int
pthread_is_valid(pthread_t pthread)
{
    if (pthread == NULL) {
        return 0;
    }
    pthread_list_lock();
    for (pthread_t pthread_c = pthread_list;
         pthread_c != NULL; pthread_c = pthread_c->next) {
        if (pthread_c == pthread) {
            pthread_list_unlock();
            return 1;
        }
    }
    pthread_list_unlock();
    return 0;
}

static inline void
pthread_list_insert(pthread_t pthread)
{
    pthread_list_lock();
    pthread->next = pthread_list;
    if (pthread->next) {
        pthread->next->previous = pthread;
    }
    pthread->previous = NULL;
    pthread_list = pthread;
    pthread->in_list = 1;
    pthread_list_unlock();
}

static inline void
pthread_list_remove(pthread_t pthread)
{
    if (pthread->in_list) {
        pthread_list_lock();
        if (pthread->previous) {
            pthread->previous->next = pthread->next;
        } else {
            pthread_list = pthread->next;
        }
        if (pthread->next) {
            pthread->next->previous = pthread->previous;
        }
        pthread->in_list = 0;
        pthread_list_unlock();
    }
}

/*************************************
 *      Creation & Execution         *
 *************************************/

/* Implemented in arch/{architecture}/libs/posix/src/pthread.spp */
extern void __pthread_init(void);

/* Create a new pthread state
   This is an internal function but is defined here so that
   libc can initialize the main thread's pthread state without
   accessing pthread_internal.
   Sets the l4tid to the tid of the current thread */
pthread_t
__pthread_state_new(const pthread_attr_t * restrict attr)
{
    /* Create a default attr object if one is not provided */
    struct pthread_attr_i * attr_i;
    if (attr == NULL) {
        struct pthread_attr_i attr_it;
        attr_i = &attr_it;
        pthread_attr_default(attr_i);
    } else {
        attr_i = *attr;
    }

    /* Create a new pthread state */
    pthread_t pthread = (pthread_t)malloc(sizeof(struct pthread_i));

    /* Insert into list */
    pthread_list_insert(pthread);

    /* Initialize its mutex */
    pthread->mutex = malloc(sizeof(struct okl4_mutex));
    okl4_mutex_init(pthread->mutex);

    /* Set pthread state to defaulted values */
    pthread->termination_state = PTHREAD_STATUS_ACTIVE;
    pthread->exit_status = NULL;
    pthread->detach_status = attr_i->detachstate;
    pthread->joined_stack = NULL;
    pthread->joinee_thread = NULL;
    pthread->cancel_state = PTHREAD_CANCEL_ENABLE;
    pthread->cancel_type = PTHREAD_CANCEL_DEFERRED;
    pthread->cancel_request = PTHREAD_CANCEL_NOREQUEST;
    pthread->cleanup_stack = NULL;
    pthread->once_exec_stack = NULL;
    memset(pthread->keys, '\0', sizeof(struct pthread_key_instance *) * PTHREAD_KEYS_MAX);

    /*
     * Set the tid to the tid of the main thread, primarily so libc init
     * can initialize the pthread of the main thread.
     *
     * If the current thread is not the main thread, that's ok, because
     * pthread->l4tid will be overwritten in pthread_create().
     */
    pthread->l4tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    return pthread;
}

int
pthread_create(pthread_t * restrict thread, const pthread_attr_t * restrict attr,
               void *(*start_routine)(void *), void* restrict arg)
{
    const envitem_t *env_prio;

    /* Create a default attr object if one is not provided */
    struct pthread_attr_i * attr_i;
    if (attr == NULL) {
        struct pthread_attr_i attr_it;
        attr_i = &attr_it;
        pthread_attr_default(attr_i);
    } else {
        attr_i = *attr;
    }

    /* Create a stack if one is not provided
     * Use given stack size (which will default to PTHREAD_DEFAULT_STACK_SIZE),
     * but ensure if is at least PTHREAD_STACK_MIN.
     * Note the provided (or calculated here) stack must be the TOP of the stack. */
    void* stackaddr = attr_i->stackaddr;
    if (stackaddr == NULL) {
        size_t stacksize = attr_i->stacksize < PTHREAD_STACK_MIN ?
            PTHREAD_STACK_MIN : attr_i->stacksize;
        stackaddr = malloc(sizeof(char) * stacksize);
        if (stackaddr == NULL) {
            return EAGAIN;
        }
        /* Find the top of the stack */
        stackaddr = (char*)stackaddr + stacksize;
    }

    /* Create a pthread_t structure */
    *thread = __pthread_state_new(&attr_i);
    pthread_t pthread = *thread;

    /* Set its termination state to PTHREAD_STATUS_GENESIS.
     * It will be set to active once the thread has setup its TLS etc.
     * This is provided just in case an operation needs to access
     * the state in the TLS but cannot be sure if it has been setup yet -
     * you can simply spin on waiting for termination_state to not be equal
     * to PTHREAD_STATUS_GENESIS. */
    pthread->termination_state = PTHREAD_STATUS_GENESIS;

    /* Create a new OKL4 thread. Pass the l4tid field of the thread struct in to
     * be populated. Create thread with program-wide default priority if it is
     * availble. TODO: handle errors in creation. */
    thread_ref_t thread_ref;
    if ((env_prio = iguana_getenv("PTHREAD_DEFAULT_PRIORITY")) != NULL) {
        thread_ref = thread_create_priority(env_const(env_prio), &(pthread->l4tid));
    }
    else {
        thread_ref = thread_create(&(pthread->l4tid));
    }

    /* We want the new thread to execute pthread_begin()
       Place the arguments on the stack and call architecture-dependent ASM
       to pop off stack and branch to the function.
       Arguments need to be in this order: user start routine, user arg,
       pthread struct */
    void** stack_top = (void**)stackaddr;
    stack_top--;
    *stack_top = (void*)start_routine;
    stack_top--;
    *stack_top = (void*)arg;
    stack_top--;
    *stack_top = (void*)pthread;
    thread_start(thread_ref, (uintptr_t)__pthread_init, (uintptr_t)stack_top);

    return 0;
}

/* Called from ASM __sys_thread_entry to pthread specific stuff */
void
__pthread_thread_init(pthread_t pthread)
{
    /* Place the pthread state in TLS */
    struct thread_state * thread_state = thread_state_get_base();
    thread_state->pthread = pthread;

    /* Set its status to active */
    pthread->termination_state = PTHREAD_STATUS_ACTIVE;
}

int
pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    /* Cast the once_control to our internal struct */
    struct pthread_once_i *once_control_i = (struct pthread_once_i *)once_control;

    /* Sanity check.
     * There's not a lot we can do if they pass in an invalid pointer :( */
    if (once_control_i == NULL || once_control_i->magic != PTHREAD_ONCE_CONTROL_MAGIC) {
        return EINVAL;
    }
    
    /* Lock and load
     * We execute the function while holding the lock since we should provide a
     * guarantee to other callers of pthread_once on return that the function
     * has completed; additionally, we only set the status to done on successful return
     * of the function - if it is a cancellation point and is cancelled, we aren't done.
     * But, this means we are holding the lock on cancel: so, we keep a stack of held
     * pthread_once locks to release on pthread_exit.
     * Note we need a stack because init_routine could also call pthread_once. */
    okl4_mutex_lock(&once_control_i->mutex);
    if (once_control_i->status != PTHREAD_ONCE_CONTROL_DONE) {

        /* Obtain the current pthread and push pthread_once onto stack */
        pthread_t pthread = thread_state_get_pthread();
        pthread_lock(pthread);
        once_control_i->next = pthread->once_exec_stack;
        pthread->once_exec_stack = once_control_i;
        pthread_unlock(pthread);
        
        /* Call the provided init routine */
        init_routine();

        /* Pop the exec stack */
        pthread_lock(pthread);
        pthread->once_exec_stack = once_control_i->next;
        pthread_unlock(pthread);
        
        /* Set the status on successful completion */
        once_control_i->status = PTHREAD_ONCE_CONTROL_DONE;
    }
    okl4_mutex_unlock(&once_control_i->mutex);
    return 0;
}

/*************************************
 * Termination, Cleanup, & Deletion  *
 *************************************/

static inline void
pthread_alert_die(pthread_t pthread)
{
    /* We know the thread is about to die. Remove it from the list now, so
     * that it appears externally to be invalid */
    pthread_list_remove(pthread);

    /* IPC the thread. Should we change the death msg protocol,
     * e.g. to async notify, change this */
    L4_Msg_t msg;
    L4_MsgClear(&msg);
    L4_MsgLoad(&msg);
    L4_Send(pthread->l4tid);
}

/* Internal function to cleanup the current pthread and delete it */
static inline void
pthread_delete(pthread_t pthread)
{
    /* Remove from list.
     * We may have already been removed, e.g. if the thread had joiners and exited.
     * pthread_list_remove handles this. */
    pthread_list_remove(pthread);

    /* Free any joined threads */
    while (pthread->joined_stack != NULL) {
        struct pthread_joined_thread * next_joined_thread = pthread->joined_stack->next;
        free(pthread->joined_stack);
        pthread->joined_stack = next_joined_thread;
    }

    /* Free any cleanup functions */
    while (pthread->cleanup_stack != NULL) {
        struct pthread_cleanup_routine * next_cleanup = pthread->cleanup_stack->next;
        free(pthread->cleanup_stack);
        pthread->cleanup_stack = next_cleanup;
    }

    /* Free the mutex */
    free(pthread->mutex);

    /* Free our pthread */
    free(pthread);

    /* Cleanup the thread state and the TLS that we established */
    __tls_free();

    /* Tell Iguana to delete us */
    thread_delete_self();
}

/* Private function for pthread_cancel to jump a thread */
static void
pthread_exit_with_cancel(void)
{
    pthread_exit(PTHREAD_CANCELED);
}

void
pthread_exit(void * value)
{
    /* Obtain the pthread state from the thread state in TLS
       This function is defined in threadstate.h */
    pthread_t pthread = thread_state_get_pthread();

    pthread_lock(pthread);

    /* Set termination status and exit value
       Termination status simply indicates that the thread has finished execution */
    pthread->termination_state = PTHREAD_STATUS_TERMINATED;
    pthread->exit_status = value;

    /* Unlock any exec'ing pthread_once's.
     * We must do this before calling cleanup functions, as they
     * could themselves call pthread_once with the same control block */
    while (pthread->once_exec_stack != NULL) {
        struct pthread_once_i * once_control_i = pthread->once_exec_stack;
        okl4_mutex_unlock(&once_control_i->mutex);
        pthread->once_exec_stack = once_control_i->next;
    }

    /* Call any registered cleanup functions */
    while (pthread->cleanup_stack != NULL) {
        struct pthread_cleanup_routine * cleanup = pthread->cleanup_stack;
        cleanup->routine(cleanup->arg);
        pthread->cleanup_stack = cleanup->next;
        free(cleanup);
    }

    /* Destroy keys, calling any destructor functions first */
    // TODO: possibly keep a stack of used keys to avoid having to iterate entire list?
    for (int i=0; i<PTHREAD_KEYS_MAX; i++) {
        if (pthread->keys[i] != NULL) {
            struct pthread_key_instance *pthread_key_instance = pthread->keys[i];

            /* Call destructor, passing data as argument */
            if (pthread_key_instance->destructor != NULL) {
                pthread_key_instance->destructor(pthread_key_instance->data);
            }

            /* Free and clear */
            free(pthread_key_instance);
            pthread->keys[i] = NULL;
        }
    }

    /* IPC any joined threads, passing the returned value */
    if (pthread->joined_stack != NULL) {

        /* Set the thread to detached */
        pthread->detach_status = PTHREAD_DETACHED;

        /* Since we are now detached, we know we're about to die.
         * Remove ourselves from the list now before we wake up any joiners. 
         * This ensures that those joiners will not continue and find us
         * in an unjoinable, but still valid (existent) state */
        pthread_list_remove(pthread);

        /* Create the joined return message */
        L4_Msg_t msg;
        L4_MsgClear(&msg);
        L4_MsgAppendWord(&msg, (L4_Word_t)pthread->exit_status);
        L4_MsgLoad(&msg);

        /* IPC each joined thread in turn */
        while (pthread->joined_stack != NULL) {
            struct pthread_joined_thread * joined_thread = pthread->joined_stack;
            L4_Send(joined_thread->joined->l4tid);
            pthread->joined_stack = joined_thread->next;
        }
    }

    /* If the thread has a detach status of "detached", delete it immediately
     * Otherwise, wait to be told to die. */
    if (pthread->detach_status == PTHREAD_DETACHED) {
        pthread_unlock(pthread);
        pthread_delete(pthread);
    } else {
        pthread_unlock(pthread);
        L4_ThreadId_t sender;
        L4_Wait(&sender);
        pthread_delete(pthread);
    }
}

// TODO: pthread_join should be a cancellation point?! see pg 184
int
pthread_join(pthread_t pthread, void ** value_ptr)
{
    /* Ensure the thread is valid,
     * return ESRCH if not */
    if (pthread_is_valid(pthread) == 0) {
        return ESRCH;
    }

    /* Ensure the thread is not self,
     * return EDEADLK if so */
    pthread_t pthread_me = thread_state_get_pthread();
    if (pthread_equal(pthread_me, pthread) != 0) {
        return EDEADLK;
    }

    pthread_lock(pthread);
    pthread_lock(pthread_me);

    /* Ensure the thread is joinable,
     * return EINVAL if not */
    if (pthread->detach_status != PTHREAD_JOINABLE) {
        pthread_unlock(pthread_me);
        pthread_unlock(pthread);
        return EINVAL;
    }

    /* If the requested thread has already terminated,
     * set value-ptr to its exit status and alert it to die. */
    if (pthread->termination_state == PTHREAD_STATUS_TERMINATED) {
        if (value_ptr) {
            *value_ptr = pthread->exit_status;
        }
        pthread_unlock(pthread_me);
        pthread_unlock(pthread);
        pthread_alert_die(pthread);

    } else {

        /* Test for deadlocks.
         * Search for ourselves as a joinee of any thread joined to
         * the thread we are trying to join (and any joined to him, and so forth). */
        if (pthread_join_deadlocksearch(pthread, pthread_me) == 1) {
            pthread_unlock(pthread_me);
            pthread_unlock(pthread);
            return EDEADLK;
        }

        /* Create a joined thread struct */
        struct pthread_joined_thread * joined_thread = 
            malloc(sizeof(struct pthread_joined_thread));
        joined_thread->joined = pthread_me;

        /* Join the thread */
        joined_thread->next = pthread->joined_stack;
        pthread->joined_stack = joined_thread;
        pthread_me->joinee_thread = pthread;

        /* Wait */
        pthread_unlock(pthread_me);
        pthread_unlock(pthread);
        L4_MsgTag_t tag = L4_Receive(pthread->l4tid);

        /* Obtain the result */
        L4_Msg_t msg;
        L4_MsgStore(tag, &msg);
        if (value_ptr) {
            *value_ptr = (void*)L4_MsgWord(&msg, 0);
        }
    }

    return 0;
}

int
pthread_join_deadlocksearch(pthread_t pthread, pthread_t find_joinee)
{
    if (pthread_equal(pthread->joinee_thread, find_joinee) != 0) {
        return 1;
    }

    if (pthread->joined_stack) {
        for (struct pthread_joined_thread * pthread_j = pthread->joined_stack;
             pthread_j != NULL; pthread_j = pthread_j->next) {
            if (pthread_join_deadlocksearch(pthread_j->joined, find_joinee) != 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

int
pthread_detach(pthread_t pthread)
{
    /* Ensure the thread is valid */
    if (pthread_is_valid(pthread) == 0) {
        return ESRCH;
    }

    pthread_lock(pthread);

    /* Invalid if the thread is not joinable */
    if (pthread->detach_status != PTHREAD_JOINABLE) {
        pthread_unlock(pthread);
        return EINVAL;

    } else if (pthread->termination_state == PTHREAD_STATUS_TERMINATED) {

        /* The thread has already terminated in a joinable state
           It is awaiting being told to die: tell it to do so. */
        pthread_alert_die(pthread);
        pthread_unlock(pthread);

    } else {
        pthread->detach_status = PTHREAD_DETACHED;
        pthread_unlock(pthread);
    }

    return 0;
}

/* Adds a cleanup routine to the current pthread's cleanup stack */
void
pthread_cleanup_push(void (*routine)(void *), void *arg)
{
    pthread_t pthread = thread_state_get_pthread();
    pthread_lock(pthread);
    struct pthread_cleanup_routine * cleanup =
        malloc(sizeof(struct pthread_cleanup_routine));
    cleanup->routine = routine;
    cleanup->arg = arg;
    cleanup->next = pthread->cleanup_stack;
    pthread->cleanup_stack = cleanup;
    pthread_unlock(pthread);
}

/* Removes the top cleanup routine
   Executes it if {execute} is non-zero */
void
pthread_cleanup_pop(int execute)
{
    pthread_t pthread = thread_state_get_pthread();
    pthread_lock(pthread);
    struct pthread_cleanup_routine * cleanup = pthread->cleanup_stack;
    if (cleanup != NULL) {

        /* Execute if required */
        if (execute != 0 ) {
            cleanup->routine(cleanup->arg);
        }

        /* Pop the stack and free the cleanup routine */
        pthread->cleanup_stack = cleanup->next;
        free(cleanup);

    }
    pthread_unlock(pthread);
}


/*************************************
 *     Identity and Comparison       *
 *************************************/

int
pthread_equal(pthread_t t1, pthread_t t2)
{
    if (pthread_is_valid(t1) == 0 || pthread_is_valid(t2) == 0) {
        return 0;
    }
    return L4_IsThreadEqual(t1->l4tid, t2->l4tid);
}

pthread_t
pthread_self(void)
{
    return thread_state_get_pthread();
}


/*************************************
 *          Attributes               *
 *************************************/

static inline void
pthread_attr_default(struct pthread_attr_i * attr)
{
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->stackaddr = NULL;
    attr->stacksize = PTHREAD_DEFAULT_STACK_SIZE;
}


/* Initialize an attribute to default values */
int
pthread_attr_init(pthread_attr_t * attr)
{
    *attr = malloc(sizeof(struct pthread_attr_i));
    if (*attr == NULL) {
        return ENOMEM;
    }
    pthread_attr_default((struct pthread_attr_i *)*attr);
    return 0;
}

int
pthread_attr_destroy(pthread_attr_t * attr)
{
    if (attr == NULL || *attr == NULL) {
        return EINVAL;
    }
    free(*attr);
    *attr = NULL;
    return 0;
}

int
pthread_attr_getdetachstate(const pthread_attr_t * attr, int * detachstate)
{
    if (attr == NULL || *attr == NULL) {
        return EINVAL;
    }
    struct pthread_attr_i * attr_i = *attr;
    *detachstate = attr_i->detachstate;
    return 0;
}

int
pthread_attr_setdetachstate(pthread_attr_t * attr, int detachstate)
{
    if (attr == NULL || *attr == NULL || 
        (detachstate != PTHREAD_CREATE_JOINABLE &&
         detachstate != PTHREAD_CREATE_DETACHED)) {
        return EINVAL;
    }
    struct pthread_attr_i * attr_i = *attr;
    attr_i->detachstate = detachstate;
    return 0;
}

int
pthread_attr_getstackaddr(const pthread_attr_t * restrict attr,
                          void ** restrict stackaddr)
{
    if (attr == NULL || *attr == NULL) {
        return EINVAL;
    }
    struct pthread_attr_i * attr_i = *attr;
    *stackaddr = attr_i->stackaddr;
    return 0;
}

int
pthread_attr_setstackaddr(pthread_attr_t * attr, void *stackaddr)
{
    if (attr == NULL || *attr == NULL) {
        return EINVAL;
    }
    struct pthread_attr_i * attr_i = *attr;
    attr_i->stackaddr = stackaddr;
    return 0;
}

int
pthread_attr_getstacksize(const pthread_attr_t * restrict attr,
                          size_t * restrict stacksize)
{
    if (attr == NULL || *attr == NULL) {
        return EINVAL;
    }
    struct pthread_attr_i * attr_i = *attr;
    *stacksize = attr_i->stacksize;
    return 0;
}

int
pthread_attr_setstacksize(pthread_attr_t * attr, size_t stacksize)
{
    if (attr == NULL || *attr == NULL) {
        return EINVAL;
    }
    struct pthread_attr_i * attr_i = *attr;
    attr_i->stacksize = (stacksize < PTHREAD_STACK_MIN ? PTHREAD_STACK_MIN :
                       stacksize);
    return 0;
}

/*************************************
 *              Keys                 *
 *************************************/

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    /* Obtain an unused key
     * Return EAGAIN errno if all slots have been used.
     * Add the obtained slot to the used list (so it can be free on
     * library shutdown, if required)
     * Increment the generation counter
     */
    pthread_key_t key_c; // current key
    key_op_lock();
    key_c = key_free_slots;
    if (key_c == NULL) {
        key_op_unlock();
        return EAGAIN;
    }
    key_free_slots = key_c->next;
    key_c->status = __PTHREAD_KEY_USED;
    unsigned int generation = key_generation++;
    key_op_unlock();
    *key = key_c;
    
    /* Setup the key */
    key_c->generation = generation;
    key_c->destructor = destructor;

    return 0;
}

int
pthread_key_delete(pthread_key_t key)
{
    /* Ensure the key is valid */
    if (key->status == __PTHREAD_KEY_FREE) {
        return EINVAL;
    }

    key_op_lock();

    /* Add the key to the free list */
    key->next = key_free_slots;
    key_free_slots = key;

    /* Invalidate the key */
    key->status = __PTHREAD_KEY_FREE;

    key_op_unlock();
    return 0;
}

void *
pthread_getspecific(pthread_key_t key)
{
    //TODO: possibly need a key-specific lock
    /* Ensure the key is valid */
    if (key->status == __PTHREAD_KEY_FREE) {
        return NULL;
    }

    /* Obtain the key instance */
    pthread_t pthread = thread_state_get_pthread();
    struct pthread_key_instance *key_instance = pthread->keys[key->slot];

    /* Ensure the key instance is valid */
    if (key_instance == NULL) {
        return NULL;
    }
    if (key_instance->generation != key->generation) {
        return NULL;
    }

    return key_instance->data;
}

int
pthread_setspecific(pthread_key_t key, const void *data)
{
    /* Ensure the key is valid */
    if (key->status == __PTHREAD_KEY_FREE) {
        return EINVAL;
    }

    /* Obtain the key instance */
    pthread_t pthread = thread_state_get_pthread();
    struct pthread_key_instance *key_instance = pthread->keys[key->slot];

    /* If the slot contains an existing, invalid key, call its destructor */
    if (key_instance != NULL && key_instance->generation != key->generation) {
        if (key_instance->destructor != NULL) {
            key_instance->destructor(key_instance->data);
        }
    }

    /* If the key instance doesn't exist, create it */
    if (key_instance == NULL) {
        key_instance = malloc(sizeof(struct pthread_key_instance));
        key_instance->destructor = key->destructor;
        key_instance->generation = key->generation;
        pthread->keys[key->slot] = key_instance;
    }

    /* Set the data */
    key_instance->data = (void*)data;
    return 0;
}


/*************************************
 *          Cancellation             *
 *************************************/

int
pthread_cancel(pthread_t pthread)
{
    /* Ensure the thread is valid */
    if (pthread_is_valid(pthread) == 0) {
        return ESRCH;
    }

    pthread_lock(pthread);

    /* Ensure the thread hasn't already terminated and doesn't
       have its cancellation state disabled */
    if (pthread->termination_state == PTHREAD_STATUS_TERMINATED ||
        pthread->cancel_state == PTHREAD_CANCEL_DISABLE) {
        pthread_unlock(pthread);
        return 0;

        /* If the cancel type is deferred, buffer the request */
    } else if (pthread->cancel_type == PTHREAD_CANCEL_DEFERRED) {
        pthread->cancel_request = PTHREAD_CANCEL_REQUEST;
        pthread_unlock(pthread);
        return 0;

    } else {

        /* If the cancel type is asynchronous, cancel the thread immediately.
           If we're cancelling ourselves, just do a function call; otherwise,
           use exreg to throw the thread into pthread_exit_with_cancel. */
        if (pthread_equal(pthread_self(), pthread) != 0) {
            pthread_unlock(pthread);
            pthread_exit_with_cancel();
            return 0;

        } else {
            pthread_unlock(pthread);
            L4_Word_t dummy;
            L4_ThreadId_t dummy_id;
            (void)L4_ExchangeRegisters(pthread->l4tid, L4_ExReg_ip, dummy,
                                       (L4_Word_t)&pthread_exit_with_cancel, 0, 0, L4_nilthread,
                                       &dummy, &dummy, &dummy, &dummy, &dummy, &dummy_id);
            return 0;
        }
    }
}

int
pthread_setcancelstate(int state, int *oldstate)
{
    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
        return EINVAL;
    }

    /* Obtain the current pthread */
    pthread_t pthread = thread_state_get_pthread();
    pthread_lock(pthread);

    /* Set the old state */
    if (oldstate != NULL) {
        *oldstate = pthread->cancel_state;
    }

    /* Set the new state */
    pthread->cancel_state = state;

    pthread_unlock(pthread);
    return 0;
}

int
pthread_setcanceltype(int type, int *oldtype)
{
    if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS) {
        return EINVAL;
    }

    /* Obtain the current pthread */
    pthread_t pthread = thread_state_get_pthread();
    pthread_lock(pthread);

    /* Set the old type */
    if (oldtype != NULL) {
        *oldtype = pthread->cancel_type;
    }

    /* Set the new state */
    pthread->cancel_type = (uint8_t)type;

    pthread_unlock(pthread);
    return 0;
}

void
pthread_testcancel(void)
{
    /* Obtain the current pthread */
    pthread_t pthread = thread_state_get_pthread();

    /* Cancel the thread if there is buffered request */
    if (pthread->cancel_request == PTHREAD_CANCEL_REQUEST) {
        pthread_exit_with_cancel();
        return;
    }
}


/*************************************
 *            Mutexes                *
 *************************************/

int
pthread_mutex_init(pthread_mutex_t * restrict mutex,
                   const pthread_mutexattr_t * restrict attr)
{
    //TODO: we don't presently handle attributes

    /* Cast to internal representation */
    struct pthread_mutex_i * mutex_i = (struct pthread_mutex_i *)mutex;

    /* Ensure the mutex isn't already initialized */
    if (mutex_i->magic == PTHREAD_MUTEX_MAGIC) {
        return EBUSY;
    }

    /* Initialize the magic number and the internal (backing) mutex */
    mutex_i->magic = PTHREAD_MUTEX_MAGIC;
    okl4_mutex_init(&mutex_i->mutex);
    return 0;
}

int
pthread_mutex_destroy(pthread_mutex_t * mutex)
{
    /* Ensure the mutex is valid.
       There is not much we can do if the pointer itself
       is invalid */
    struct pthread_mutex_i * mutex_i = (struct pthread_mutex_i *)mutex;
    if (mutex_i == NULL || mutex_i->magic != PTHREAD_MUTEX_MAGIC) {
        return EINVAL;
    }

    /* Ensure the the mutex is not being used by trying to lock it. */
    if (!okl4_mutex_trylock(&mutex_i->mutex)) {
        return EBUSY;
    }

    /* Invalidate the mutex */
    mutex_i->magic = 0;
    return 0;
}

int
pthread_mutex_lock(pthread_mutex_t * mutex)
{
    /* Ensure the mutex is valid. */
    struct pthread_mutex_i * mutex_i = (struct pthread_mutex_i *)mutex;
    if (mutex_i == NULL || mutex_i->magic != PTHREAD_MUTEX_MAGIC) {
        return EINVAL;
    }
    okl4_mutex_lock(&mutex_i->mutex);
    return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    /* Ensure the mutex is valid. */
    struct pthread_mutex_i * mutex_i = (struct pthread_mutex_i *)mutex;
    if (mutex_i == NULL || mutex_i->magic != PTHREAD_MUTEX_MAGIC) {
        return EINVAL;
    }

    if (okl4_mutex_trylock(&mutex_i->mutex) != 0) {
        return EBUSY;
    }

    return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t * mutex)
{
    /* Ensure the mutex is valid. */
    struct pthread_mutex_i * mutex_i = (struct pthread_mutex_i *)mutex;
    if (mutex_i == NULL || mutex_i->magic != PTHREAD_MUTEX_MAGIC) {
        return EINVAL;
    }
    okl4_mutex_unlock(&mutex_i->mutex);
    return 0;
}


/*************************************
 *       Condition Variables         *
 *************************************/

int
pthread_cond_init(pthread_cond_t * restrict cond, const pthread_condattr_t * restrict attr)
{
    /* Cast to our internal representation */
    struct pthread_cond_i * cond_i = (struct pthread_cond_i *)cond;

    /* Return error if cond is already initialized */
    if (cond_i->magic == PTHREAD_COND_MAGIC) {
        return EBUSY;
    }

    //TODO: we don't presently handle attributes
    cond_i->magic = PTHREAD_COND_MAGIC;
    okl4_mutex_init(&cond_i->mutex);
    cond_i->first_waiter = NULL;
    cond_i->last_waiter = NULL;
    return 0;
}

int
pthread_cond_destroy(pthread_cond_t * cond)
{
    /* Cast to our internal representation */
    struct pthread_cond_i * cond_i = (struct pthread_cond_i *)cond;

    /* Ensure the pointer is a valid cond variable.
     * Not much we can do if they pass in an invalid pointer :-( */
    if (cond_i == NULL || cond_i->magic != PTHREAD_COND_MAGIC) {
        return EINVAL;
    }

    /* Return error if the cond var is being used */
    okl4_mutex_lock(&cond_i->mutex);

    if (cond_i->first_waiter != NULL) {
        okl4_mutex_unlock(&cond_i->mutex);
        return EBUSY;
    }

    /* Invalidate the cond var */
    cond_i->magic = 0;
    okl4_mutex_unlock(&cond_i->mutex);
    return 0;
}

/* This will be called with mutex locked by the caller,
 * else undefined behaviour results */
int
pthread_cond_wait(pthread_cond_t * restrict cond, pthread_mutex_t * restrict mutex)
{
    L4_ThreadId_t sender;
    L4_MsgTag_t tag = L4_Niltag;

    /* Cast to our internal representation */
    struct pthread_cond_i * cond_i = (struct pthread_cond_i *)cond;

    /* Ensure the pointer is a valid cond variable.
     * Not much we can do if they pass in an invalid pointer :-( */
    if (cond_i == NULL || cond_i->magic != PTHREAD_COND_MAGIC) {
        return EINVAL;
    }

    /* Join the list */
    pthread_t pthread = thread_state_get_pthread();
    // Don't think this is needed??
    pthread_lock(pthread);
    struct pthread_cond_waiter * waiter = malloc(sizeof(struct pthread_cond_waiter));
    if (waiter == NULL) { 
        pthread_unlock(pthread);
        return ENOMEM;
    }
    waiter->pthread = pthread;
    waiter->next = NULL;

    okl4_mutex_lock(&cond_i->mutex);
    if (cond_i->first_waiter == NULL) {
        cond_i->first_waiter = waiter;
        cond_i->last_waiter = waiter;
    } else {
        cond_i->last_waiter->next = waiter;
        cond_i->last_waiter = waiter;
    }
    okl4_mutex_unlock(&cond_i->mutex);

    /* Unlock the given mutex */
    pthread_mutex_unlock(mutex);

    pthread_unlock(pthread);

    /* Block */
    tag = L4_Wait(&sender);
    if (L4_Label(tag) != PTHREAD_SIGNAL_CODE) {
        L4_KDB_Enter("pthread_cond_wait(): spurious IPC");
    }

    /* Re-obtain the mutex */
    pthread_mutex_lock(mutex);

    return 0;
}

int
pthread_cond_signal(pthread_cond_t * cond)
{
    L4_MsgTag_t tag = L4_Niltag;

    /* Cast to our internal representation */
    struct pthread_cond_i * cond_i = (struct pthread_cond_i *)cond;

    /* Ensure the pointer is a valid cond variable.
     * Not much we can do if they pass in an invalid pointer :-( */
    if (cond_i == NULL || cond_i->magic != PTHREAD_COND_MAGIC) {
        return EINVAL;
    }

    /* Pop the first from the queue and IPC him to stop blocking */
    okl4_mutex_lock(&cond_i->mutex);
    struct pthread_cond_waiter * waiter = cond_i->first_waiter;
    if (waiter != NULL) {
        L4_Set_Label(&tag, PTHREAD_SIGNAL_CODE);
        L4_Set_MsgTag(tag);
        L4_Send(waiter->pthread->l4tid);

        /* Remove from list and free */
        cond_i->first_waiter = waiter->next;
        if (cond_i->last_waiter == waiter) {
            cond_i->last_waiter = NULL;
        }
        free(waiter);
    }
    okl4_mutex_unlock(&cond_i->mutex);

    return 0;
}

int
pthread_cond_broadcast(pthread_cond_t * cond)
{
    L4_MsgTag_t tag = L4_Niltag;

    /* Cast to our internal representation */
    struct pthread_cond_i * cond_i = (struct pthread_cond_i *)cond;

    /* Ensure the pointer is a valid cond variable.
     * Not much we can do if they pass in an invalid pointer :-( */
    if (cond_i == NULL || cond_i->magic != PTHREAD_COND_MAGIC) {
        return EINVAL;
    }

    okl4_mutex_lock(&cond_i->mutex);
    struct pthread_cond_waiter * waiter = cond_i->first_waiter;
    while (waiter != NULL) {
        L4_Set_Label(&tag, PTHREAD_SIGNAL_CODE);
        L4_Set_MsgTag(tag);
        L4_Send(waiter->pthread->l4tid);

        /* Remove from list and free */
        cond_i->first_waiter = waiter->next;
        if (cond_i->last_waiter == waiter) {
            cond_i->last_waiter = NULL;
        }
        free(waiter);

        /* Next waiter */
        waiter = cond_i->first_waiter;
    }
    okl4_mutex_unlock(&cond_i->mutex);

    /* Note: if there are multiple waiters, the first one is just going
     * to get the mutex lock again and then all the others released are going to wait
     * to obtain it. A high-performance implementation would move all waiters
     * except the first onto the wait list of the mutex, since they will get there
     * anyway */

    return 0;
}
