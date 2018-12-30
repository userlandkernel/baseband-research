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
#include <driver/driver.h>
#include <driver/character_ops.h>
#include <stddef.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <l4/types.h>
#include <l4/ipc.h>
#include <l4/kdebug.h>
#include <iguana/thread.h>
#include <timer/timer.h>
#include <iguana/env.h>

static void *setup(int spacec, bus_space_t *spacev, dma_handle_t dma,
                   bus_space_t pciconf);
static void enable(void *device);
static void cleanup(void *device);
static void interrupt(void *device, int irq);
static int write(void *device, int seqc, struct data *seqv,
                 callback_t callback, callback_data_t callback_data,
                 uintptr_t key);
static int read(void *device, int seqc, struct data *seqv,
                callback_t callback, callback_data_t callback_data,
                uintptr_t key);
static void poll_thread(void *);

struct kdb_dev {
    struct driver_instance generic;
    int enabled;
    int xmit_fifo;

    /* Current transmit */
    int txen;
    int writec, writec_init;
    struct data *writev, *writev_init;
    callback_t write_callback;
    callback_data_t write_cb_data;
    uintptr_t write_key;
    /* Current recieve */
    int rxen;
    int readc, readc_init;
    struct data *readv, *readv_init;
    callback_t read_callback;
    callback_data_t read_cb_data;
    uintptr_t read_key;
};

static int do_read(struct kdb_dev *self);

static struct character_ops ops = {
    /* Driver ops */
    {setup,
     enable,
     cleanup,
     interrupt},
    /* Character ops */
    write,
    read
};

struct driver drv_l4_kdb = {
    "L4 kdb",
    character_device,
    {(struct driver_ops *)&ops}
};

static int
write(void *device, int seqc, struct data *seqv,
      callback_t callback, callback_data_t callback_data, uintptr_t key)
{
    int i, j;

    j = seqc;
    while (j != 0) {
        for (i = 0; i < seqv->length; i++) {
            L4_KDB_PrintChar(seqv->data[i]);
        }
        seqv->xferred = seqv->length;
        j--;
    }
    callback(callback_data, seqc, seqv, key);

    return 0;
}

static int
read(void *device, int seqc, struct data *seqv,
     callback_t callback, callback_data_t callback_data, uintptr_t key)
{
    struct kdb_dev *self = device;

    assert(self->readc == 0);
    self->readc = self->readc_init = seqc;
    self->readv = self->readv_init = seqv;
    self->read_callback = callback;
    self->read_cb_data = callback_data;
    self->read_key = key;
    return do_read(self);
}

static signed char got_char = -1;
L4_ThreadId_t poll_thrd, main_tid;

/* This should be somewhere better */
#define IRQ_LABEL (((~(0UL)) >> 20 << 20) >> 16)

static void
poll_thread(void *x)
{
    L4_Msg_t msg;
    timer_t timer;
    int r;

    timer_init();

    /* Create a timer */
    timer = timer_create();
    assert(timer != NULL);

    r = timer_request(timer, MILLISECS(10), TIMER_PERIODIC);

    while (1) {
        unsigned char x;

        L4_KDB_ReadChar(&x);

        if ((x > 0) && (x < 128)) {
            got_char = x;
            L4_MsgClear(&msg);
            L4_Set_Label(&msg.tag, IRQ_LABEL);
            L4_MsgLoad(&msg);
            L4_Call(main_tid);
        }

        timer_wait();
    }
}

static void *
setup(int spacec, bus_space_t *spacev, dma_handle_t dma, bus_space_t pciconf)
{
    struct kdb_dev *drv;

    drv = malloc(sizeof(struct kdb_dev));
    if (drv == 0)
        return 0;
    memset(drv, 0, sizeof *drv);

    drv->generic.classp = &drv_l4_kdb;
    main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));

    poll_thrd = thread_l4tid(thread_create_simple(poll_thread, NULL, 239));
    L4_KDB_SetThreadName(poll_thrd, "kbd_poll");

    return drv;
}

static int
do_read(struct kdb_dev *self)
{
    int got_some = 0;

    if (got_char != -1) {
        got_some = 1;
        self->readv->data[self->readv->xferred] = got_char;
        got_char = -1;
        // printf("foo\n");
        self->readv->xferred++;

        /* End of this data item */
        if (self->readv->xferred >= self->readv->length) {
            self->readv++;
            self->readc--;
        }
    }
    if (got_some) {
        // printf("Doing callback... %d %d %d (last %i)\n", uart_rne_get(),
        // self->readc, self->readv_init->xferred,
        // self->readv->data[self->readv->xferred-1]);
        self->read_callback(self->read_cb_data, self->readc_init,
                            self->readv_init, self->read_key);
        self->readc = 0;
    }
    if (self->readc == 0) {
        return 0;               /* Data read */
    }
    return 1;                   /* Data still pending */
}

static void
enable(void *device)
{
}
static void
cleanup(void *device)
{
}

static void
interrupt(void *device, int irq)
{
    struct kdb_dev * self = (struct kdb_dev *)device;

    if(self->readv) {
        do_read(self);
    }
}
