/*
 * Part of Magpie: start two threads for communication via IPC.
 *
 * Adapted from the Pingpong benchmark included in L4KA Pistachio and
 * licensed accordingly. Original copyright notice follows.
*/

/*********************************************************************
 *                
 * Copyright (C) 2002, 2004-2006,  Karlsruhe University
 *                
 * File path:     bench/pingpong/pingpong.cc
 * Description:   Pingpong test application
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: pingpong.cc,v 1.34.2.14 2006/11/18 14:44:47 stoess Exp $
 *                
 ********************************************************************/
#include <config.h>
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <l4io.h>

#define START_ADDR(func)	((L4_Word_t) func)

#define IPC_ARCH_OPTIMIZATION

#define SMALLSPACE_SIZE		16

L4_ThreadId_t master_tid, pager_tid, server_tid, client_tid;

L4_Word_t client_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t server_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t pager_stack[2048] __attribute__ ((aligned (16)));


L4_Fpage_t kip_area, utcb_area;
L4_Word_t utcb_size;

#define UTCB(x) ((void*)(L4_Address(utcb_area) + (x) * utcb_size))
#define NOUTCB	((void*)-1)


static L4_Word_t page_bits;

extern "C" void memset (char * p, char c, int size)
{
    for (;size--;)
	*(p++)=c;
}


extern "C" {
/* Implemented by the test suite */
void test_client_main (L4_ThreadId_t);
void test_server_main (void);
}

/* Implemented below*/
void test_client (void);
void test_server (void);


/*
 * Architecture dependent function stubs.
 * We only do ia32 at the moment.
 */

#include "ia32.h"



/*
 * Default arch dependent definitions and function stubs.
 */

#if !defined(UTCB_ADDRESS)
#define UTCB_ADDRESS	(0x80000000UL)
#endif

#if !defined(HAVE_HANDLE_ARCH_PAGEFAULT)
L4_INLINE L4_Fpage_t handle_arch_pagefault (L4_MsgTag_t tag, L4_Word_t faddr, L4_Word_t fip, L4_Word_t log2size)
{
    return L4_FpageLog2 (faddr, log2size);
}
#endif

#if !defined(HAVE_READ_CYCLES)
L4_INLINE L4_Word_t read_cycles (void)
{
    return 0;
}
#endif

#if !defined(HAVE_READ_INSTRS)
L4_INLINE L4_Word_t read_instrs (void)
{
    return 0;
}
#endif

void
test_client(void)
{
	/* Run the main program (defined externally) */
	test_client_main(server_tid);

	/* Endless sleep */
	for (;;)
		L4_Sleep (L4_Never);

	/* NOTREACHED */
}

void
test_server(void)
{
	/* Run the main program. */
	test_server_main();

	/* Endless sleep */
	for (;;)
		L4_Sleep (L4_Never);

	/* NOTREACHED */
}


static void send_startup_ipc (L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp)
{
    /*printf ("sending startup message to %lx, (ip=%lx, sp=%lx)\n",
	    (long) tid.raw, (long) ip, (long) sp);*/
    L4_Msg_t msg;
    L4_Clear (&msg);
    L4_Append (&msg, ip);
    L4_Append (&msg, sp);
    L4_Load (&msg);
    L4_Send (tid);
}


void
pager (void) {
	L4_ThreadId_t tid;
	L4_MsgTag_t tag;
	L4_Msg_t msg;
	
	for (;;) {
		tag = L4_Wait (&tid);

		for (;;) {
			L4_Store (tag, &msg);

			//printf ("Pager got msg from %p (%p, %p, %p)\n",
			//	(void *) tid.raw, (void *) tag.raw,
			//	(void *) L4_Get (&msg, 0), (void *) L4_Get (&msg, 1));

			if (L4_GlobalId (tid) == master_tid) {
				/* Start client and server threads. */
				send_startup_ipc (server_tid, L4_Get(&msg, 0),
						  (L4_Word_t) server_stack +
						  sizeof (server_stack) - 32);
				/* Start client */
				send_startup_ipc (client_tid, L4_Get(&msg, 1),
						  (L4_Word_t) client_stack+
						  sizeof (client_stack) - 32);
				break;

			}

			if (L4_UntypedWords (tag) != 2 || L4_TypedWords (tag) != 0 ||
			!L4_IpcSucceeded (tag)) {
				printf ("nest: malformed pagefault IPC from %p (tag=%p)\n",
					(void *) tid.raw, (void *) tag.raw);
				L4_KDB_Enter ("malformed pf");
				break;
			}

			L4_Word_t faddr = L4_Get (&msg, 0);
			L4_Word_t fip   = L4_Get (&msg, 1);

			L4_Fpage_t fpage = handle_arch_pagefault (tag, faddr, fip, page_bits);
			
			L4_Clear (&msg);
			L4_Append (&msg, L4_MapItem (fpage + L4_FullyAccessible, faddr));
			L4_Load (&msg);
			tag = L4_ReplyWait (tid, &tid);
		}
	}
}

int main (void)
{
    L4_Word_t control;
    L4_Msg_t msg;

    L4_KernelInterfacePage_t * kip =
	(L4_KernelInterfacePage_t *) L4_KernelInterface ();

    /* Find smallest supported page size. There's better at least one
     * bit set. */
    for (page_bits = 0;  
         !((1 << page_bits) & L4_PageSizeMask(kip)); 
         page_bits++);

    // Size for one UTCB
    utcb_size = L4_UtcbSize (kip);

#if defined(KIP_ADDRESS)
    // Put kip in different location (e.g., to allow for small
    // spaces).
    kip_area = L4_FpageLog2 (KIP_ADDRESS, L4_KipAreaSizeLog2 (kip));
#else    
    // Put the kip at the same location in all AS to make sure we can
    // reuse the syscall jump table.
    kip_area = L4_FpageLog2 ((L4_Word_t) kip, L4_KipAreaSizeLog2 (kip));
#endif

    // We need a maximum of two threads per task
    utcb_area = L4_FpageLog2 ((L4_Word_t) UTCB_ADDRESS,
			      L4_UtcbAreaSizeLog2 (kip) + 1);
    printf ("kip_area = %lx, utcb_area = %lx, utcb_size = %lx\n", 
	    kip_area.raw, utcb_area.raw, utcb_size);

    // Touch the memory to make sure we never get pagefaults
    extern L4_Word_t _end, _start;
    for (L4_Word_t * x = (&_start); x < &_end; x++) {
		volatile L4_Word_t q;
		q = *(volatile L4_Word_t*) x;
    }
    
    // Create pager
    master_tid = L4_Myself ();
    pager_tid = L4_GlobalId (L4_ThreadNo (master_tid) + 1, 2);
    client_tid = L4_GlobalId (L4_ThreadNo (master_tid) + 2, 2);
    server_tid = L4_GlobalId (L4_ThreadNo (master_tid) + 3, 2);

    // VU: calculate UTCB address -- this has to be revised
    L4_Word_t pager_utcb = L4_MyLocalId().raw;
    pager_utcb = (pager_utcb & ~(utcb_size - 1)) + utcb_size;
    printf("local id = %lx, pager UTCB = %lx\n", L4_MyLocalId().raw,
	   pager_utcb);

    L4_ThreadControl (pager_tid, L4_Myself (), L4_Myself (), 
		      L4_Myself (), (void*)pager_utcb);
    L4_Start (pager_tid, (L4_Word_t) pager_stack + sizeof(pager_stack) - 32,
	      START_ADDR (pager));

	/* Put threads in separate address spaces */
	L4_ThreadControl (client_tid, client_tid, master_tid,
			  L4_nilthread, UTCB(0));
	L4_ThreadControl (server_tid, server_tid, master_tid, 
			  L4_nilthread, UTCB(1));
	L4_SpaceControl (client_tid, 0, kip_area, utcb_area, L4_nilthread,
			 &control);
	L4_SpaceControl (server_tid, 0, kip_area, utcb_area, L4_nilthread,
			 &control);
	L4_ThreadControl (client_tid, client_tid, master_tid, pager_tid, 
			  NOUTCB);
	L4_ThreadControl (server_tid, server_tid, master_tid, pager_tid, 
			  NOUTCB);

	// Send message to notify pager to startup both threads
	L4_Clear (&msg);
	L4_Append (&msg, START_ADDR (test_server));
	L4_Append (&msg, START_ADDR (test_client));
	L4_Load (&msg);
	L4_Send (pager_tid);

#if 0
	L4_ThreadControl (ping_tid, L4_nilthread, L4_nilthread, 
			  L4_nilthread, NOUTCB);
	L4_ThreadControl (pong_tid, L4_nilthread, L4_nilthread, 
			  L4_nilthread, NOUTCB);
    }

    for (;;)
	L4_KDB_Enter ("EOW");
#endif
	/* Done. */
	for (;;)
		L4_Sleep (L4_Never);
}
