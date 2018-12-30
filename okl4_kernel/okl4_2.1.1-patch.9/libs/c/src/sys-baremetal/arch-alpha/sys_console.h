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

#ifndef _SYS_CONSOLE_H_
#define _SYS_CONSOLE_H_

#include <stdlib.h>

#define BUFSIZE 256

#define INIT_HWRPB (void*) 0x10000000

/*
 * Console callback routine numbers
 */
#define CCB_GETC                0x01
#define CCB_PUTS                0x02
#define CCB_RESET_TERM          0x03
#define CCB_SET_TERM_INT        0x04
#define CCB_SET_TERM_CTL        0x05
#define CCB_PROCESS_KEYCODE     0x06

#define CCB_OPEN                0x10
#define CCB_CLOSE               0x11
#define CCB_IOCTL               0x12
#define CCB_READ                0x13
#define CCB_WRITE               0x14

#define CCB_SET_ENV             0x20
#define CCB_RESET_ENV           0x21
#define CCB_GET_ENV             0x22
#define CCB_SAVE_ENV            0x23

#define CCB_PSWITCH             0x30
#define CCB_BIOS_EMUL           0x32
/*
 * Environment variable numbers
 */
#define ENV_AUTO_ACTION         0x01
#define ENV_BOOT_DEV            0x02
#define ENV_BOOTDEF_DEV         0x03
#define ENV_BOOTED_DEV          0x04
#define ENV_BOOT_FILE           0x05
#define ENV_BOOTED_FILE         0x06
#define ENV_BOOT_OSFLAGS        0x07
#define ENV_BOOTED_OSFLAGS      0x08
#define ENV_BOOT_RESET          0x09
#define ENV_DUMP_DEV            0x0A
#define ENV_ENABLE_AUDIT        0x0B
#define ENV_LICENSE             0x0C
#define ENV_CHAR_SET            0x0D
#define ENV_LANGUAGE            0x0E
#define ENV_TTY_DEV             0x0F

#define NO_SRM_CONSOLE          -1L

#define CTB_TYPE_NONE     0
#define CTB_TYPE_DETACHED 1
#define CTB_TYPE_SERIAL   2
#define CTB_TYPE_GRAPHICS 3
#define CTB_TYPE_MULTI    4

struct ctb_struct {
    unsigned long type;
    unsigned long id;
    unsigned long reserved;
    unsigned long dsd_len;
    char dsd[0];
};

int64_t console_getenv(uint64_t i, char *env, uint64_t maxlen);
int64_t console_puts(const char *str, uint64_t len);

extern void pal_halt(void);
extern int64_t console_dispatch(uint64_t a0, uint64_t a1,
                                uint64_t a2, uint64_t a3, void *hwrpb);

#endif /* !_SYS_CONSOLE_H_ */
