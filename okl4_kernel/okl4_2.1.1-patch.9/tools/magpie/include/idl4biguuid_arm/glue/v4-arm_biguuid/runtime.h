#ifndef IDL4_ARM_GLUE_RUNTIME_H
#define IDL4_ARM_GLUE_RUNTIME_H

/* Most generic functions aren't here, because they weren't used. Let me know
 * if you miss them - nfd feb 2005 */
#include IDL4_INC_ARCH(helpers.h)
#include IDL4_INC_API(interface.h)
#include IDL4_INC_GLUE(msgbuf.h)

#include <l4/ipc.h>
#include <l4/message.h>

// L4_Word_t * mr = &(__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET + i];

#define IDL4_INLINE inline

static inline void
idl4_process_request(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, 
		     idl4_msgbuf_t *msgbuf, long *cnt, void *func)
{
	*cnt = ((int (*)(L4_ThreadId_t, idl4_msgbuf_t*))func)(*partner, msgbuf);
	if (*cnt<0) {
		*partner = L4_nilthread;
		*cnt = 0;
	}
}

/*static inline void
idl4_reply_and_wait(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag, 
		    idl4_msgbuf_t *msgbuf, long *cnt)
{
	L4_MsgTag_t _result;
	
	L4_MsgLoad((L4_Msg_t*)&msgbuf->obuf);
	_result = L4_ReplyWait(*partner, partner);
	L4_MsgStore(_result, &(msgbuf->ibuf));
	*msgtag = _result;
}
*/

static inline void
idl4_reply_and_wait(L4_ThreadId_t *partner, L4_MsgTag_t *msgtag,
		idl4_msgbuf_t *msgbuf, long *cnt)
{
	register L4_ThreadId_t ipc_dest asm("r0") = *partner;
	register L4_ThreadId_t ipc_FromSpecifier asm("r1") = L4_anythread;
	register L4_Word_t ipc_Timeouts asm("r2") = L4_Timeouts(L4_ZeroTime, L4_Never);
	register L4_MsgTag_t client_msgtag asm("r3") = msgbuf->obuf.tag;
	register L4_Word_t r_mr1 asm("r4") = msgbuf->obuf.msg[1];
	register L4_Word_t r_mr2 asm("r5") = msgbuf->obuf.msg[2];
	register L4_Word_t r_mr3 asm("r6") = msgbuf->obuf.msg[3];
	register L4_Word_t r_mr4 asm("r7") = msgbuf->obuf.msg[4];
	register __L4_Ipc_t r_syscall asm("r12") = __L4_Ipc;
	int i;
	L4_Word_t * tcb_mr = &(__L4_ARM_Utcb())[__L4_TCR_MR_OFFSET];
	/* Store extra params in the message registers */
	for (i=5; i < msgbuf->obuf.tag.X.u + msgbuf->obuf.tag.X.t + 1; i++) {
		tcb_mr[i] = msgbuf->obuf.msg[i];
	}
	__asm__ __volatile (
		__L4_SYSCALL 
		: "=r" (ipc_dest), "=r" (client_msgtag), "=r" (r_mr1), "=r" (r_mr2), "=r" (r_mr3), 
				"=r" (r_mr4), "=r" (ipc_FromSpecifier), "=r" (ipc_Timeouts), "=r" (r_syscall)
		: "r" (client_msgtag), "r" (r_mr1), "r" (r_mr2), "r" (r_mr3), "r" (r_mr4),
				"r" (r_syscall), "r" (ipc_dest), "r" (ipc_FromSpecifier), "r" (ipc_Timeouts)
		: "lr", "r8", "r9", "r10" /* r8, r9, r10 constitute __L4_ALWAYS_CLOBBER */
	);
	*msgtag = client_msgtag;
	*partner = ipc_dest;
	tcb_mr[0] = client_msgtag.raw;
	tcb_mr[1] = r_mr1;
	tcb_mr[2] = r_mr2;
	tcb_mr[3] = r_mr3;
	tcb_mr[4] = r_mr4;
	/* NB we store stuff in ibuf here, but we could really just get away with using the
	 * MRs... */
	L4_MsgStore(client_msgtag, &(msgbuf->ibuf));
}

#endif //IDL4_ARM_GLUE_RUNTIME_H
