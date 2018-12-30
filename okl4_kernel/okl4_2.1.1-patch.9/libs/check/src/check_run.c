/*
 * Check: a unit test framework for C
 * Copyright (C) 2001, 2002 Arien Malec
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#ifdef WITH_FORK_TESTS
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <l4/ipc.h>
#include <l4/message.h>
#include <l4/thread.h>
#include <l4/schedule.h>
#include <l4/config.h>
#include <l4e/map.h>

#include <check/check.h>
#include "check_error.h"
#include "check_list.h"
#include "check_impl.h"
#include "check_msg.h"
#include "check_log.h"

extern void * __okl4_bootinfo;

enum rinfo {
  CK_R_SIG,
  CK_R_PASS,
  CK_R_EXIT,
  CK_R_FAIL_TEST,
  CK_R_FAIL_FIXTURE
};

enum tf_type {
  CK_FORK_TEST,
  CK_NOFORK_TEST,
  CK_NOFORK_FIXTURE
};

static void srunner_run_init (SRunner *sr, enum print_output print_mode);
static void srunner_run_end (SRunner *sr, enum print_output print_mode);
static void srunner_iterate_suites (SRunner *sr,
                                    enum print_output print_mode);
static void srunner_run_tcase (SRunner *sr, TCase *tc);
static int srunner_run_unchecked_setup (SRunner *sr, TCase *tc);
static void srunner_run_unchecked_teardown (SRunner *sr, TCase *tc);
static TestResult * tcase_run_checked_setup (SRunner *sr, TCase *tc);
static void tcase_run_checked_teardown (TCase *tc);
static void srunner_iterate_tcase_tfuns (SRunner *sr, TCase *tc);
static void srunner_add_failure (SRunner *sr, TestResult *tf);
static TestResult *tcase_run_tfun_nofork (SRunner *sr, TCase *tc, TF *tf);

#ifdef WITH_FORK_TESTS
static TestResult *tcase_run_tfun_fork (SRunner *sr, TCase *tc, TF *tf);
static TestResult *receive_result_info_fork (const char *tcname,
                                             const char *tname, int status);
static void set_fork_info (TestResult *tr, int status);
static int waserror (int status);
static char *signal_msg (int sig);
static char *exit_msg (int exitstatus);
#endif
static TestResult *tcase_run_tfun_withpager (SRunner *sr, TCase *tc, TF *tfun);
static void set_withpager_info (TestResult *tr, int status);

static TestResult *receive_result_info_nofork (const char *tcname,
                                               const char *tname);
static void set_nofork_info (TestResult *tr);
static char *null_msg (void);
static char *exception_msg (void);
static char *unknown_msg (void);
static char *pass_msg (void);

#define MSG_LEN 100

static void srunner_run_init (SRunner *sr, enum print_output print_mode)
{
  set_fork_status(srunner_fork_status(sr));
  setup_messaging();
  srunner_init_logging (sr, print_mode);
  log_srunner_start (sr);
}

static void srunner_run_end (SRunner *sr, enum print_output print_mode)
{
  log_srunner_end (sr);
  srunner_end_logging (sr);
  teardown_messaging();
  set_fork_status(CK_FORK);
}

static void srunner_iterate_suites (SRunner *sr,
                                    enum print_output print_mode)

{
  List *slst;
  List *tcl;
  TCase *tc;

  slst = sr->slst;

  for (list_front(slst); !list_at_end(slst); list_advance(slst)) {
    Suite *s = list_val(slst);

    log_suite_start (sr, s);

    tcl = s->tclst;

    for (list_front(tcl);!list_at_end (tcl); list_advance (tcl)) {
      tc = list_val (tcl);
      srunner_run_tcase (sr, tc);
    }

    log_suite_end (sr, s);
  }
}

void srunner_run_all (SRunner *sr, enum print_output print_mode)
{
  if (sr == NULL)
    return;
  if (print_mode >= CK_LAST)
    eprintf("Bad print_mode argument to srunner_run_all: %d",
            __FILE__, __LINE__, print_mode);

  srunner_run_init (sr, print_mode);
  srunner_iterate_suites (sr, print_mode);
  srunner_run_end (sr, print_mode);
}

static void srunner_add_failure (SRunner *sr, TestResult *tr)
{
  list_add_end (sr->resultlst, tr);
  /* If the context is either of these, the test has run. */
  if ((tr->ctx == CK_CTX_TEST) || (tr->ctx == CK_CTX_TEARDOWN))
    sr->stats->n_checked++;
  if (tr->rtype == CK_FAILURE)
    sr->stats->n_failed++;
  else if (tr->rtype == CK_ERROR)
    sr->stats->n_errors++;

}

static TestResult * tr_not_run(const char * tcname, const char * tname)
{
    TestResult * tr;

    tr = tr_create();
    set_nofork_info(tr);
    tr->ctx = CK_CTX_SETUP;
    // These strings are allocated with malloc because msg and file
    // are freed at the end of the tests, and string constants will
    // break malloc -TGB
    tr->msg = malloc(strlen("Test Not Run")+1);
    strcpy(tr->msg, "Test Not Run");
    tr->file = malloc(strlen("None")+1);
    strcpy(tr->file, "None");
    tr->line = 0;
    tr->tcname = tcname;
    tr->tname = tname;

    return tr;
}

static void srunner_iterate_tcase_tfuns (SRunner *sr, TCase *tc)
{
  List *tfl;
  TF *tfun;
  TestResult *tr = NULL;
  char **exception;

  tfl = tc->tflst;

  for (list_front(tfl); !list_at_end (tfl); list_advance (tfl)) {
    tfun = list_val (tfl);
    exception = sr->exceptions;
    tr = NULL;
    while(exception && *exception)
    {
      if(strcmp(*exception, tfun->name) == 0)
      {
        printf("Not running excepted test %s\n", tfun->name);
        tr = tr_not_run(tc->name, tfun->name);
      }
      exception++;
    }
    if(!tr)
    {
      switch (srunner_fork_status(sr)) {
      case CK_FORK:
#ifdef WITH_FORK_TESTS
        tr = tcase_run_tfun_fork (sr, tc, tfun);
#else
        assert(!"Can't run a fork test!\n");
#endif
        break;
      case CK_WITHPAGER:
        tr = tcase_run_tfun_withpager(sr, tc, tfun);
        break;
      case CK_NOFORK:
        tr = tcase_run_tfun_nofork (sr, tc, tfun);
        break;
      default:
        eprintf("Bad fork status in SRunner", __FILE__, __LINE__);
      }
    }
    srunner_add_failure (sr, tr);
    log_test_end(sr, tr);
  }
}

static int srunner_run_unchecked_setup (SRunner *sr, TCase *tc)
{
  TestResult *tr;
  List *l;
  Fixture *f;
  int rval = 1;

  set_fork_status(CK_NOFORK);

  l = tc->unch_sflst;

  for (list_front(l); !list_at_end(l); list_advance(l)) {
    send_ctx_info(get_send_key(),CK_CTX_SETUP);
    f = list_val(l);
    f->fun();

    tr = receive_result_info_nofork (tc->name, "unchecked_setup");

    if (tr->rtype != CK_PASS) {
      srunner_add_failure(sr, tr);
      rval = 0;
      break;
    }
    free(tr->file);
    free(tr->msg);
    free(tr);
  }

  set_fork_status(srunner_fork_status(sr));
  return rval;
}

static TestResult * tcase_run_checked_setup (SRunner *sr, TCase *tc)
{
  TestResult *tr = NULL;
  List *l;
  Fixture *f;
  enum fork_status fstat = srunner_fork_status(sr);

  l = tc->ch_sflst;
  if (fstat == CK_FORK || fstat == CK_WITHPAGER) {
    send_ctx_info(get_send_key(),CK_CTX_SETUP);
  }

  for (list_front(l); !list_at_end(l); list_advance(l)) {
    if (fstat == CK_NOFORK) {
      send_ctx_info(get_send_key(),CK_CTX_SETUP);
    }
    f = list_val(l);
    f->fun();

    /* Stop the setup and return the failure if nofork mode. */
    if (fstat == CK_NOFORK) {
      tr = receive_result_info_nofork (tc->name, "checked_setup");
      if (tr->rtype != CK_PASS) {
        break;
      }

      free(tr->file);
      free(tr->msg);
      free(tr);
      tr = NULL;
    }
  }

  return tr;
}

static void tcase_run_checked_teardown (TCase *tc)
{
  List *l;
  Fixture *f;

  l = tc->ch_tflst;

  send_ctx_info(get_send_key(),CK_CTX_TEARDOWN);

  for (list_front(l); !list_at_end(l); list_advance(l)) {
    f = list_val(l);
    f->fun();
  }
}

static void srunner_run_unchecked_teardown (SRunner *sr, TCase *tc)
{
  List *l;
  Fixture *f;

  set_fork_status(CK_NOFORK);
  l = tc->unch_tflst;

  for (list_front(l); !list_at_end(l); list_advance(l)) {

    f = list_val(l);
    send_ctx_info(get_send_key(),CK_CTX_TEARDOWN);
    f->fun ();
  }
  set_fork_status(srunner_fork_status(sr));
}

static void srunner_run_tcase (SRunner *sr, TCase *tc)
{

  if (srunner_run_unchecked_setup (sr,tc)) {

    srunner_iterate_tcase_tfuns(sr,tc);

    srunner_run_unchecked_teardown (sr, tc);
  }
}

#ifdef WITH_FORK_TESTS
static TestResult *receive_result_info_fork (const char *tcname,
                                             const char *tname, int status)
{
  TestResult *tr;

  tr = receive_test_result (get_recv_key(), waserror(status));
  if (tr == NULL)
    eprintf("Failed to receive test result", __FILE__, __LINE__);
  tr->tcname = tcname;
  tr->tname = tname;
  set_fork_info(tr, status);

  return tr;
}
#endif

static TestResult *receive_result_info_withpager (const char *tcname,
                                             const char *tname, int status)
{
  TestResult *tr;

  // status 0 is pass, 1 is fail, everything else is error
  tr = receive_test_result (get_recv_key(), (status > 1 || status < 0));
  if (tr == NULL)
    eprintf("Failed to receive test result", __FILE__, __LINE__);
  tr->tcname = tcname;
  tr->tname = tname;
  set_withpager_info(tr, status);

  return tr;
}

static TestResult *receive_result_info_nofork (const char *tcname,
                                               const char *tname)
{
  TestResult *tr;

  tr = receive_test_result(get_recv_key(), 0);
  if (tr == NULL)
    eprintf("Failed to receive test result", __FILE__, __LINE__);
  tr->tcname = tcname;
  tr->tname = tname;
  set_nofork_info(tr);

  return tr;
}

#ifdef WITH_FORK_TESTS
static void set_fork_info (TestResult *tr, int status)
{
  int was_sig = WIFSIGNALED(status);
  int was_exit = WIFEXITED(status);
  int exit_status = WEXITSTATUS(status);

  if (was_sig) {
    tr->rtype = CK_ERROR;
    tr->msg = signal_msg(WTERMSIG(status));
  } else if (was_exit && exit_status == 0) {
    tr->rtype = CK_PASS;
    tr->msg = pass_msg();
  } else if (was_exit && exit_status != 0) {
    if (tr->msg == NULL) { /* early exit */
      tr->rtype = CK_ERROR;
      tr->msg = exit_msg(exit_status);
    } else {
      tr->rtype = CK_FAILURE;
    }
  }
}
#endif

static void set_withpager_info (TestResult *tr, int status)
{
  // Examine status for correct error message
  // status is return value from pager function
  if(status == 0)
  {
      tr->rtype = CK_PASS;
      tr->msg = pass_msg();
  }
  else if (status == 1)
  {
      tr->rtype = CK_FAILURE;
  }
  else if (status == 2)
  {
      tr->rtype = CK_ERROR;
      tr->msg = null_msg();
  }
  else if (status == 3)
  {
      tr->rtype = CK_ERROR;
      tr->msg = exception_msg();
  }
  else
  {
      tr->rtype = CK_ERROR;
      tr->msg = unknown_msg();
  }
}

static void set_nofork_info (TestResult *tr)
{
  if (tr->msg == NULL) {
    tr->rtype = CK_PASS;
    tr->msg = pass_msg();
  } else {
    tr->rtype = CK_FAILURE;
  }
}

static TestResult *tcase_run_tfun_nofork (SRunner *sr, TCase *tc, TF *tfun)
{
  TestResult *tr;

  tr = tcase_run_checked_setup(sr, tc);
  if (tr == NULL) {
    tfun->fn();
    tcase_run_checked_teardown(tc);
    return receive_result_info_nofork(tc->name, tfun->name);
  }

  return tr;
}

#ifdef WITH_FORK_TESTS
static TestResult *tcase_run_tfun_fork (SRunner *sr, TCase *tc, TF *tfun)
{
  pid_t pid;
  int status = 0;

  pid = fork();
  if (pid == -1)
     eprintf ("Unable to fork:",__FILE__,__LINE__);
  if (pid == 0) {
    tcase_run_checked_setup (sr, tc);
    tfun->fn();
    tcase_run_checked_teardown (tc);
    exit(EXIT_SUCCESS);
  }
  (void) wait(&status);
  return receive_result_info_fork (tc->name, tfun->name, status);
}

static char *signal_msg (int signal)
{
  char *msg = emalloc (MSG_LEN); /* free'd by caller */
  snprintf (msg, MSG_LEN, "Received signal %d", signal);
  return msg;
}

static char *exit_msg (int exitval)
{
  char *msg = emalloc(MSG_LEN); /* free'd by caller */
  snprintf (msg, MSG_LEN,
            "Early exit with return value %d", exitval);
  return msg;
}
#endif

/* Stack size in words */
#define STACK_SIZE 0x1000
/* stack for test thread */
static L4_Word_t stack[STACK_SIZE];
L4_ThreadId_t test_tid;

/* create a thread that starts execution at the given function */
static L4_ThreadId_t createThread(void (*ip)(void))
{
  L4_ThreadId_t tid;
  int r;
  L4_Word_t utcb;

  /* Allocate a new UTCB and thread number for the thread. */
  tid = L4_rootserver;
  tid.global.X.thread_no += 3;
#ifndef NO_UTCB_RELOCATE
  utcb = ((L4_PtrSize_t)L4_GetUtcbBase()) + L4_GetUtcbSize();
#else
  utcb = ~0UL;
#endif

  test_tid = tid;

  /* Create the new thread */
  r = L4_ThreadControl(tid, L4_rootspace, L4_rootserver, L4_rootserver, L4_rootserver,
          0, (void *)(L4_PtrSize_t)utcb);
  if(r != 1)
    printf("\nThread Create failed, ErrorCode = %d\n", (int)L4_ErrorCode());
  assert(r == 1);

  /* Start up the thread. */
  L4_Start_SpIp(tid, (L4_Word_t)(stack+STACK_SIZE), (L4_PtrSize_t)ip);
  return tid;
}

/* delete the given thread */
static void deleteThread(L4_ThreadId_t thread)
{
  int r;
  r = L4_ThreadControl(thread, L4_nilspace, L4_nilthread, L4_nilthread, L4_nilthread, 0, (void *)0);
  if(r != 1)
    printf("\nThread Delete failed, ErrorCode = %d\n", (int)L4_ErrorCode());
  assert(r == 1);
}

/* globals for test thread to use */
static SRunner * g_sr;
static TCase * g_tc;
static TF * g_tfun;

/* function executed by test thread */
static void trun_withpager(void)
{
  ARCH_THREAD_INIT

  L4_MsgTag_t tag;

  tcase_run_checked_setup (g_sr, g_tc);
  g_tfun->fn();
  tcase_run_checked_teardown (g_tc);

  // label 0 for success
  tag = L4_Niltag;
  tag = L4_MsgTagAddLabel(tag, 0);
  L4_Set_MsgTag(tag);
  tag = L4_Call(L4_Pager());
  printf("\n%s (%s, %d)\n", L4_IpcFailed(tag)?"Error":"No Error", (L4_ErrorCode()&1)?"Receive phase":"Send phase", (int)(L4_ErrorCode()&0x1e)>>1);
  assert(!"Shouldn't get here - Did you leave a send redirector set?");
}

/* Simple pager loop
 * Takes success, failure and exception messages, converting them into status values
 * Doesn't handle failed IPCs
 */
static int pager_loop (void)
{
  L4_Msg_t msg;
  L4_MsgTag_t tag;
  L4_ThreadId_t from = L4_nilthread;
  L4_Word_t result;
  int i;

  while(1)
  {
    if(from.raw != L4_nilthread.raw)
      tag = L4_ReplyWait(from, &from);
    else
      tag = L4_Wait(&from);

    L4_MsgStore(tag, &msg);
    if(!L4_IpcSucceeded(tag)) {
      printf("IPC send failed - maybe thread was deleted?\n");
      continue;
    }

    result = L4_MsgLabel(&msg);

    switch(result)
    {
    case 0:     // Success
      return 0;
    case 1: // Fail
      return 1;
    default: // Unknown
      {
          long type = (result << 16);
          type >>= 20;
          switch(type)
          {
          case -2:
              printf("libcheck: pagefault received from %lx\n", from.raw);
              for (i = 0 ; i < 3 ; i++)
                  printf("word %d: %lx\n", i, L4_MsgWord(&msg, i));
              return 3;
          case -4:
          case -5:
              printf("libcheck: exception received from %lx\n", from.raw);
              printf("tag: %lx\n", L4_MsgMsgTag(&msg).raw);
              for(i = 0; i < L4_UntypedWords(L4_MsgMsgTag(&msg)); i++)
                  printf("word %d: %lx\n", i, L4_MsgWord(&msg, i));  
              return 3;
          default:
              break;
          }
          printf("libcheck: Unknown message from %lx\n", from.raw);
      }
      return -1;
    }
  }
}

static TestResult *tcase_run_tfun_withpager (SRunner *sr, TCase *tc, TF *tfun)
{
  L4_ThreadId_t tid;
  L4_Word_t dummy;
  int status = 0;
  g_sr = sr; g_tc = tc; g_tfun = tfun;

  // create a lower priority thread to run the test
  tid = createThread((void (*)(void))trun_withpager);
  L4_Set_Priority(tid, 200);
  L4_Schedule(tid, -1, 0x00000001, -1, -1, 0, &dummy);
  L4_Schedule(L4_myselfconst, -1, 0x00000001, -1, -1, 0, &dummy);

  // Do pager loop which returns exit status
  status = pager_loop();

  // Clean up
  deleteThread(tid);

  // return correctly
  return receive_result_info_withpager (tc->name, tfun->name, status);
}
static char *null_msg (void)
{
  char *msg = emalloc(sizeof("Null Pointer Dereferenced"));
  strcpy (msg, "Null Pointer Dereferenced");
  return msg;
}
static char *exception_msg (void)
{
  char *msg = emalloc(sizeof("Exception Occurred"));
  strcpy (msg, "Exception Occurred");
  return msg;
}
static char *unknown_msg (void)
{
  char *msg = emalloc(sizeof("Unknown Error"));
  strcpy (msg, "Unknown Error");
  return msg;
}
static char *pass_msg (void)
{
  char *msg = emalloc(sizeof("Passed"));
  strcpy (msg, "Passed");
  return msg;
}

enum fork_status srunner_fork_status (SRunner *sr)
{
  if (sr->fstat == CK_FORK_UNSPECIFIED) {
#ifdef WITH_FORK_TESTS
    char *env = getenv ("CK_FORK");
    if (env == NULL)
      return CK_FORK;
    if (strcmp (env,"no") == 0)
      return CK_NOFORK;
    else
      return CK_FORK;
#else
    return CK_NOFORK;
#endif
  } else
    return sr->fstat;
}

void srunner_set_fork_status (SRunner *sr, enum fork_status fstat)
{
  sr->fstat = fstat;
}

#ifdef WITH_FORK_TESTS
static int waserror (int status)
{
  int was_sig = WIFSIGNALED (status);
  int was_exit = WIFEXITED (status);
  int exit_status = WEXITSTATUS (status);

  return (was_sig || (was_exit && exit_status != 0));
}
#endif
