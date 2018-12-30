#!/usr/bin/env python
"""
This file contains the implementation of the IPC complex tests generation.
"""
##############################################################################
# Copyright (c) 2004 Open Kernel Labs, Inc. (Copyright Holder).
# All rights reserved.
# 
# 1. Redistribution and use of OKL4 (Software) in source and binary
# forms, with or without modification, are permitted provided that the
# following conditions are met:
# 
#     (a) Redistributions of source code must retain this clause 1
#         (including paragraphs (a), (b) and (c)), clause 2 and clause 3
#         (Licence Terms) and the above copyright notice.
# 
#     (b) Redistributions in binary form must reproduce the above
#         copyright notice and the Licence Terms in the documentation and/or
#         other materials provided with the distribution.
# 
#     (c) Redistributions in any form must be accompanied by information on
#         how to obtain complete source code for:
#        (i) the Software; and
#        (ii) all accompanying software that uses (or is intended to
#        use) the Software whether directly or indirectly.  Such source
#        code must:
#        (iii) either be included in the distribution or be available
#        for no more than the cost of distribution plus a nominal fee;
#        and
#        (iv) be licensed by each relevant holder of copyright under
#        either the Licence Terms (with an appropriate copyright notice)
#        or the terms of a licence which is approved by the Open Source
#        Initative.  For an executable file, "complete source code"
#        means the source code for all modules it contains and includes
#        associated build and other files reasonably required to produce
#        the executable.
# 
# 2. THIS SOFTWARE IS PROVIDED ``AS IS'' AND, TO THE EXTENT PERMITTED BY
# LAW, ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED.  WHERE ANY WARRANTY IS
# IMPLIED AND IS PREVENTED BY LAW FROM BEING DISCLAIMED THEN TO THE
# EXTENT PERMISSIBLE BY LAW: (A) THE WARRANTY IS READ DOWN IN FAVOUR OF
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT) AND (B) ANY LIMITATIONS PERMITTED BY LAW (INCLUDING AS TO
# THE EXTENT OF THE WARRANTY AND THE REMEDIES AVAILABLE IN THE EVENT OF
# BREACH) ARE DEEMED PART OF THIS LICENCE IN A FORM MOST FAVOURABLE TO
# THE COPYRIGHT HOLDER (AND, IN THE CASE OF A PARTICIPANT, THAT
# PARTICIPANT). IN THE LICENCE TERMS, "PARTICIPANT" INCLUDES EVERY
# PERSON WHO HAS CONTRIBUTED TO THE SOFTWARE OR WHO HAS BEEN INVOLVED IN
# THE DISTRIBUTION OR DISSEMINATION OF THE SOFTWARE.
# 
# 3. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR ANY OTHER PARTICIPANT BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import sys

script_path = sys.argv[0]
save_path = os.getcwd()
index = script_path.find("ipc_test_generator.py")
os.chdir(script_path[0:index])
cfile = open('../src/ipccomplex.c', 'w')

str_test_impl = ""
str_add_test = ""

str_includes = """#include \"ipccomplex_utility.h\"
#include \"utility.h\"
#include \"l4test.h\"
#include <l4/ipc.h>
#include <stdio.h>

static L4_ThreadId_t ipc_main_thread;

L4_ThreadId_t
get_ipc_main_tid(void)
{
    return ipc_main_thread;
}

"""

def remove_nonsense(str):
        if (str.rfind("non blocking") > -1) and (str.rfind(", ready") > -1):
                return False
        if (str.rfind("not allowed") > -1) and (str.rfind(", ready") > -1 or str.startswith("blocking")):
                return False
        return True

main_comb_code = ['MAIN_SBLOCK | MAIN_RBLOCK | ', 'MAIN_RBLOCK | ', 'MAIN_SBLOCK | ', '']
main_comb = ['blocking send, blocking receive', 'non blocking send, blocking receive', 
             'blocking send, non blocking receive', 'non blocking send, non blocking receive']
blocking_list = ['blocking ', 'non blocking ']
ready_list = ['ready to ', 'not ready to ']
perm_list_recv = ['IPC send from main thread allowed', 'IPC send from main thread not allowed']
perm_list_send = ['IPC send to main thread allowed', 'IPC send to main thread not allowed']
recv_comb = [b+"receive, " + r+"receive, " + p for b in blocking_list for r in ready_list for p in perm_list_recv]
recv_tmp = filter(remove_nonsense, recv_comb)
send_comb = [b+"send, " + r+"send, " + p for b in blocking_list for r in ready_list for p in perm_list_send]
send_tmp = filter(remove_nonsense, send_comb)

recv_comb = [recv_tmp[0], recv_tmp[1], recv_tmp[1], recv_tmp[0], recv_tmp[3], recv_tmp[3], recv_tmp[0], recv_tmp[2],
             recv_tmp[2], recv_tmp[0], recv_tmp[2], recv_tmp[1]]
send_comb = [send_tmp[0], send_tmp[0], send_tmp[1], send_tmp[1], send_tmp[0], send_tmp[3], send_tmp[3], send_tmp[0],
             send_tmp[2], send_tmp[2], send_tmp[1], send_tmp[2]]

for test_num, comb_num in zip(range(48), 4*range(1, 13)):
        str_test_impl += """
/*
\\begin{test}{IPCCPX00%02d}
  \TestDescription{IPC send to receiving thread and IPC receive from
    sending thread in the same IPC call}
  \TestImplementationProcess{
    Call IPC with the following arguments:
    \\begin{enumerate}
    \item \mytotid{receiving thread}
    \item \myfromtid{sending thread}
    \end{enumerate}}
  \TestPreConditions{\\begin{enumerate}
    \item main thread: %s.
    \item receiving thread: %s.
    \item sending thread: %s.
    \end{enumerate}}
  \TestPassStatus{pass}
  \TestImplementationStatus{Implemented}
\end{test}
*/
/* Test MRs valid on send and receive */
START_TEST(IPCCPX00%02d)
{
    L4_ThreadId_t xas_main_thread, receiver, sender;
    L4_ThreadId_t deblocker;
    L4_SpaceId_t xas_main_space, receiver_space, sender_space;
    L4_Word_t case_code = %sCASE%02d;
    L4_MsgTag_t tag;

    create_customised_thread(case_code);
    xas_main_thread = get_xas_main_tid();
    receiver = get_receiver_tid();
    sender = get_sender_tid();
    deblocker = get_deblocker_tid();
    xas_main_space = lookupSpace(xas_main_thread);
    receiver_space = lookupSpace(receiver);
    sender_space = lookupSpace(sender);

    tag = L4_Receive(xas_main_thread);
    fail_unless(L4_IpcSucceeded(tag), "Thread did not complete");
 
    deleteThread(receiver);
    deleteThread(sender);
    deleteThread(xas_main_thread);
    deleteThread(deblocker);
    deleteSpace(xas_main_space);
    deleteSpace(receiver_space);
    deleteSpace(sender_space);
    //printf(\"Test passed successfully !\\n\");
}
END_TEST
""" % (test_num+1, main_comb[test_num/12], recv_comb[comb_num-1], send_comb[comb_num-1], test_num+1, main_comb_code[test_num/12], comb_num)
        str_add_test += """tcase_add_test(tc, IPCCPX00%02d);\n    """ % (test_num + 1)

str_test_setup = """
static void test_setup(void)
{
    initThreads(1);
    ipc_main_thread = L4_Myself();
}

static void test_teardown(void)
{

}
"""

str_make_tcase = """
TCase *
make_ipc_cpx_tcase(void)
{
    TCase *tc;
   
    initThreads(0);

    tc = tcase_create(\"Ipc complex\");
    tcase_add_checked_fixture(tc, test_setup, test_teardown);
    
    """ + str_add_test + """
    return tc;
}
"""

cfile.write(str_includes + str_test_impl + str_test_setup + str_make_tcase)
cfile.close()
os.chdir(save_path)

