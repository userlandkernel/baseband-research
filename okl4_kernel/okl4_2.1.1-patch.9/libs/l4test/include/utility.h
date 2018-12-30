/*
 * Copyright (c) 2005, National ICT Australia
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
#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <l4/types.h>
#include <bootinfo/bootinfo.h>
#include <l4/thread.h>
#include <l4test/arch/constants.h>

/* These constants are defined on a per architecture basis */
extern const L4_Word_t default_attr;
extern L4_ThreadId_t default_thread_handler;

#define STACK_SIZE      0x400 // in words

/*
 * Return the arch-dependant location of the UTCB.
 *
 * While there is an architecture independant way of getting the UTCB
 * (L4_GetUtcbBase()), the field offsets are all in terms of the
 *  archtecture dependant location, which is not necessarily the same
 *  thing (in partictular, ia32).
 */
L4_Word_t* get_arch_utcb_base(void);

/*
 * Return the arch-dependant location of the MR registers.
 *
 * While there is an architecture independant way of getting the UTCB
 * (L4_GetUtcbBase()), there doesn't seem to be an architecture
 * independant way of fetching fields in the UTCB, in particular the
 * MRs.  Further the field offsets constants are all defined in terms
 * of an archtecture dependant location, which is not necessarily the
 * same as what's returned by L4_GetUtcbBase() (in partictular, ia32).
 */
L4_Word_t* get_mr_base(void);

/* initialise the thread creation functions,
 * if own_address_spaces is 1 then each created thread will live in it's own address space
 */
void initThreads(int own_address_spaces);
/* create a new address space */
L4_SpaceId_t createSpaceWithClist(L4_ClistId_t clist);
L4_SpaceId_t createSpace(void);
/* delete the given space */
void deleteSpace(L4_SpaceId_t space);
/* lookup space of thread  */
L4_SpaceId_t lookupSpace(L4_ThreadId_t thread);
/* lookup reply handle of thread 
 * Should not be used in any except IPC tests to check the reply handle is correct
 */
L4_ThreadId_t lookupReplyHandle(L4_ThreadId_t thread);
/* update space of thread */
void updateSpace(L4_ThreadId_t thread, L4_SpaceId_t space);
/* create a thread that starts execution at the given function */
L4_ThreadId_t createThread(void (*ip)(void));
/* create a thread that starts execution at the given function n the
 * given threads address space (nilspace for new address space)*/
L4_ThreadId_t createThreadInSpace(L4_SpaceId_t space, void (*ip)(void));
/* update a thread's threadid */
void update_thread_id(L4_ThreadId_t, L4_ThreadId_t);
/* allocate Thread for some tests that create own threads/AS and need the l4test handler */
L4_ThreadId_t allocateThread(L4_SpaceId_t space);
void deallocateThread(L4_ThreadId_t tid);
/* delete the given thread */
void deleteThread(L4_ThreadId_t thread);
/* Return a local thread's stack buffer. */
L4_Word_t* threadStack (L4_ThreadId_t t);

/** Create a system thread that will not be automatically deleted at
 * the conclusion of each test. */
L4_ThreadId_t createSystemThread(void (*ip)(void));

/** Determine if the given thread ID is a valid system thread. */
int isSystemThread(L4_ThreadId_t tid);

/* check if a thread is sending */
int checkSending(L4_ThreadId_t thread);
/* check if a thread is receiving */
int checkReceiving(L4_ThreadId_t thread);

/* wait for a thread to start sending */
void waitSending(L4_ThreadId_t thread);
/* wait for a thread to start receiving */
void waitReceiving(L4_ThreadId_t thread);
/* wait for a thread to send IPC until timeout */
int waitThreadTimeout(L4_ThreadId_t thread);

/* Tag and acceptor permutation code
 * These functions support the permutation of
 * specified bits in the message tag and the acceptor
 *
 * the permutations are generated from the low order bits of an integer
 */

/* since we use an integer for easy permutations
 * these are the bits in the integer that map to the
 * various flags
 */
#define NUM_PERMUTS 0x40
#define BIT_S       0x20
#define BIT_R       0x10
#define BIT_N       0x08
#define BIT_P       0x04
#define BIT_U       0x02
#define BIT_A       0x01


/* Set the message Tag and acceptor based on the given integer */
L4_MsgTag_t setMsgTagAndAcceptor(int i);

/* checks that i has the bits specified in the mask set as required by the value
 * Used to check that this is a permutation that we actually want to test
 */
int validTest(int i, int valid_mask, int valid_value);

/* Generate a user friendly output of what the integer means for the tag and the acceptor */
char * tagIntToString(int i);


/* Create and check a message with 10 MRS */
void fillMessage(L4_Msg_t * msg, L4_MsgTag_t tag);
void checkMessage(L4_Msg_t * msg, int i);

/* The magic numbers used in the mr checking */
#define MAGIC_LABEL2    0x1337
#define MAGIC_MR0               0x14133714
#define MAGIC_MR1               0x13371414
#define MAGIC_MR2               0xdeadbeef
#define MAGIC_MR3               0xbeefcafe
#define MAGIC_MR4               0x1337cafe
#define MAGIC_MR5               0xcafe1337
#define MAGIC_MR6               0xbeefdead
#define MAGIC_MR7               0xdeadcafe
#define MAGIC_MR8               0xcafedead
#define MAGIC_MR9               0x14141337


/* Particular bits in the returned message tag and
 * ErrorCode TCR - used in checking correctness of tests
 */
#define E_BIT 0x8000
#define X_BIT 0x4000
#define r_BIT 0x2000
#define p_BIT 0x1000

/* Get the E field from the message tag */
#define getTagE(tag) ((tag.raw &  E_BIT) >> 15)
/* Get the R field from the message tag */
#define getTagR(tag) ((tag.raw &  r_BIT) >> 13)
/* get the U field from the message tag */
#define getTagU(tag) (tag.raw & 0x3f)

#define ECODE_P_BIT 0x1
#define e_BITS (0x7 << 1)
/* Get the P field from the Error Code */
#define getECodeP(ErrorCode) (ErrorCode & ECODE_P_BIT)
/* Get the E field from the Error Code */
#define getECodeE(ErrorCode) ((ErrorCode & e_BITS)>>1)

/* Basic tags specifying send and receive blocking */
#define TAG_NOBLOCK 0x00000000
#define TAG_RBLOCK  0x00004000
#define TAG_SBLOCK  0x00008000
#define TAG_SRBLOCK 0x0000C000


/* Rounds addr up to the nearest naturally aligned address */
#define ROUND_UP(addr, size)                                    \
(  (((L4_Word_t)(addr)) + ((L4_Word_t)(size)) - 1)  &           \
   (~((L4_Word_t)(size)-1))     )


/* Multithreaded test support. */

/* Record a failure in the thread of a multithreaded test. */
void multithreaded_fail_unless(int result, int line, const char* msg, ...);

/* Record that the thread in a multithreaded test finished
 * successfully */
void multithreaded_pass(void);

/*
 * Run <nthreads> threads each starting execution at the given
 * threads.
 *
 * Returns a pointer to an array of thread ids or NULL if there was an
 * error.
 *
 */
L4_ThreadId_t* start_multithreaded_test(const char* file, void (*multithread_test)(void),
                                        int nthreads);

/*
 * Wait for the multithreaded tests to finish and collect the results.
 */
void collect_multithreaded_results(void);

/*
 * Run the given test simultaneously across multiple (NTHREAD) threads
 * to check for race conditions.  The threads must run in the same
 * address space.
 */
void run_multithreaded_test(const char* file, void (*multithread_test)(void));

/*
 * Empirically calculate the priority of the current thread. Assumes that no
 * other threads in the system are runnable.
 */
int get_current_threads_priority(void);

/*
 * Cause the CPU to generate a fault, which should in turn be converted
 * into an exception IPC.
 */
void trigger_cpu_fault(void);

/* Number of threads reserved for system-use. */
#define NUM_SYSTEM_THREADS    NTHREADS

/* Number of threads for test-use. */
#define NUM_TEST_THREADS      NTHREADS

/* Total number of threads in the system. This contrasts 'NTHREADS' which
 * is the number of threads in the system available for test use. */
#define NUM_THREADS           (NUM_TEST_THREADS + NUM_SYSTEM_THREADS)

/* Total number of threads available for multithreaded tests. */
#define NUM_MULTITHREADED_TEST_THREADS \
                              (NUM_TEST_THREADS - 2)

#endif
