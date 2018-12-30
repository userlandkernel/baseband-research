#if defined(CONFIG_SESSION)
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

#ifndef _LIBIGUANA_SESSION_H_
#define _LIBIGUANA_SESSION_H_

#include <l4/types.h>
#include <iguana/types.h>

struct session {
    session_ref_t ref;
    clist_ref_t clist;
    int own_clist;
};

/**
 * @brief Create a new session, with a specified clist
 *
 * @param object The object that you wish to access
 * @param clist The clist which the server will be provided with.
 * @param server_tid The L4 thread Id of the approriate server is returned.
 *
 * \return A pointer to the userlevel session object
 */
struct session *session_create_with_clist(objref_t object,
                                          memsection_ref_t clist,
                                          L4_ThreadId_t *server_tid);
int _session_create(objref_t object, memsection_ref_t clist,
                    L4_ThreadId_t *server_tid, struct session *session);

/**
 * @brief Create a new session, and create a dedicated clist for it
 *
 * @param object The object that you wish to access
 * @param server_tid The L4 thread Id of the approriate server is returned
 *
 * \return A pointer to the userlevel session object
 */
struct session *session_create(objref_t object, L4_ThreadId_t *server_tid);

/** @brief Create a new session, and share your base clist with it.
 *
 * This is obviously not meant to be generally used, and provides a short cut
 * until all software correctly uses caps.
 *
 * @param[in] object The object that you wish to access
 * @param[out] server_tid The L4 thread Id of the approriate server is returned
 *
 * \return A pointer to the userlevel session object
 */
struct session *session_create_full_share(objref_t object, L4_ThreadId_t
                                          *server_tid);

/**
 * @brief Set up async buffers for session communcations.
 *
 * @param[in] session The session to use
 * @param[in] call_buf A circular buffer for calls through the session
 * @param[in] return_buf A circular buffer for return values from the session
*/
void session_add_async(struct session *session, objref_t call_buf,
                       objref_t return_buf);

/**
 * Provide a given session access to a specific interface on an object
 *
 * @param session The session to provide the access to
 * @param object The object to provide access to 
 * @param interface The allowed interface
 *
 * \return True if access was provided. False otherwise. You may not be able to
 * provide access if you do not have access, or have access, but can not transfer it.
 */
int session_provide_access(struct session *session, objref_t object,
                            int interface);

/**
 * Provide a given session access to a specific interface on an object
 *
 * @param session The session to provide the access to
 * @param object The object to provide access to 
 * @param interface The allowed interface
 *
 * \return True if access was revoked. False otherwise. You may not be able to
 * revoke access if you have not previously provided it.
 */
int session_revoke_access(struct session *session, objref_t object,
                           int interface);

/**
 * Delete a session making any communication based on the session impossible.
 *
 * @param session The session to delete
 *
 */
void session_delete(struct session *session);

#endif /* _LIBIGUANA_SESSION_H_ */
#endif
