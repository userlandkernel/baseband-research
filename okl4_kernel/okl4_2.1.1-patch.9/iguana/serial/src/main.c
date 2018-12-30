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
/*
 * Author: Ben Leslie
 */

#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include <l4/ipc.h>
#include <l4/thread.h>
#include <l4/misc.h>
#include <l4/kdebug.h>

#include <iguana/hardware.h>
#include <iguana/memsection.h>
#include <iguana/thread.h>

#include <iguana/object.h>
#include <iguana/pd.h>
#include <iguana/env.h>

#include <driver/driver.h>
#include <driver/driver_ops.h>
#include <driver/character_ops.h>

#include <circular_buffer/cb.h>
#include <range_fl/range_fl.h>

#include <interfaces/serial_serverdecls.h>

// See SConstruct for iguana - enabled by DEBUG_TRACE option
#ifdef  IG_DEBUG_PRINT
#define DEBUG_PRINT (void) printf
#else
#define DEBUG_PRINT(...)
#endif

#define IRQ_LABEL (((~(0UL)) << 20) >> 16)

struct driver_info {
    int waiting_for_device;
    int waiting_for_read;
    rfl_t read_buffers;
    objref_t sink;
};

extern struct driver SERIAL_DRIVER;

struct cb_get_handle *cbg;
struct driver *device;

static struct driver_info *global_drv_info;

static void read_callback(callback_data_t data, int, struct data *,
                          uintptr_t key);
static void write_callback(callback_data_t data, int, struct data *,
                           uintptr_t key);

extern void *__callback_buffer;
struct cb_get_handle *iguana_cb_handle;

static L4_ThreadId_t main_tid;

int
main(int argc, char **argv)
{
    struct driver_ops *ops;

    main_tid = thread_l4tid(env_thread(iguana_getenv("MAIN")));
    device_create_impl(main_tid, 0UL, NULL);

    ops = SERIAL_DRIVER.ops.d_ops;

    iguana_cb_handle = cb_attach(__callback_buffer);

    serial_server_loop();
    assert(!"Should reach here\n");

    return 0;
}

objref_t
device_create_impl(CORBA_Object _caller, objref_t driver,
                   idl4_server_environment * _env)
{
    struct driver_info *drv_info;
    bus_space_t space;
    uintptr_t space_ptr;
    uintptr_t memory;

    struct driver_ops *ops;

    ops = SERIAL_DRIVER.ops.d_ops;

    global_drv_info = drv_info = malloc(sizeof(struct driver_info));
    if (!drv_info)
        return (objref_t)NULL;

    /* Let us actually allocate the backed memory.. */
    memory = memsection_create_user(0x1000, &space_ptr);
    space = (char *)space_ptr;

    //DEBUG_PRINT("Backing memory section: %p\n", (void *)space_ptr);
    /*
     * FIXME: These should be passed into the server properly
     */
#ifdef ARCH_ARM
#if defined(PLATFORM_S3C2410)
    {
        int r;
#if SERIAL_UART == 0
        uintptr_t base = 0x50000000;
        hardware_register_interrupt(main_tid, 32+0);
        hardware_register_interrupt(main_tid, 32+1);
#elif SERIAL_UART == 1
        uintptr_t base = 0x50004000;
        hardware_register_interrupt(main_tid, 32+3);
        hardware_register_interrupt(main_tid, 32+4);
#elif SERIAL_UART == 2
        uintptr_t base = 0x50008000;
        hardware_register_interrupt(main_tid, 32+6);
        hardware_register_interrupt(main_tid, 32+7);
#endif

        r = hardware_back_memsection(memory, base, L4_IOMemory);
        assert(r == 0);
        DEBUG_PRINT("Backed s3c2410 serial\n");
    }
#endif
#if defined(PLATFORM_PLEB)
    {
        int r;

        r = hardware_back_memsection(memory, 0x80050000, L4_IOMemory);
        assert(r == 0);
        DEBUG_PRINT("Backed SA1100 serial\n");
        hardware_register_interrupt(main_tid, 17);
    }
#elif defined(PLATFORM_PXA)
    {
        int r;

#if defined(SERIAL_FFUART)
        /* Use FFUART */
        DEBUG_PRINT("Using FFUART.\n");
        r = hardware_back_memsection(memory, 0x40100000, L4_IOMemory);
        hardware_register_interrupt(main_tid, 22);
#else
#error unsupported UART
#endif
        assert(r == 0);
        DEBUG_PRINT("Backed PXA serial\n");
    }
#elif defined(PLATFORM_IXP4XX)
    {
        int r;

#if defined(SERIAL_HSUART)
        /* Use High Speed UART */
        r = hardware_back_memsection(memory, 0xc8000000, L4_IOMemory);
        hardware_register_interrupt(main_tid, 15);
#elif defined(SERIAL_CONUART)
        /* Use Console UART */
        r = hardware_back_memsection(memory, 0xc8001000, L4_IOMemory);
        hardware_register_interrupt(main_tid, 13);
#else
#error unsupported UART
#endif
        assert(r == 0);
        DEBUG_PRINT("Backed IXP4xx serial\n");
    }
#endif
#if defined(MACHINE_KZM_ARM11)
        hardware_back_memsection(memory, 0x43f90000, L4_IOMemory);
        hardware_register_interrupt(main_tid, 45);
#endif
#endif
    //DEBUG_PRINT("Backed memory section\n");
    device = ops->setup(1, &space, NULL, NULL);
    //DEBUG_PRINT("Driver enable\n");
    driver_enable(device);
    //DEBUG_PRINT("Done driver enable\n");
    drv_info->waiting_for_device = 0;
    drv_info->waiting_for_read = 0;

    drv_info->read_buffers = rfl_new();
    {
        char *buffer;

        buffer = malloc(5000);
        assert(buffer != NULL);
        rfl_insert_range(drv_info->read_buffers, (uintptr_t)buffer,
                         (uintptr_t)buffer + 4999);
    }
    {
        memsection_ref_t memsection;
        thread_ref_t unused;
        thread_ref_t myself;

        memsection = memsection_lookup((objref_t)drv_info, &unused);
        myself = thread_id(main_tid);
        memsection_register_server(memsection, myself);
    }
    return (objref_t)drv_info;
}

CORBA_Object global_client;

/*
 * Interface stream
 */
uintptr_t
stream_write_impl(CORBA_Object _caller, objref_t stream_obj,
                  uintptr_t length, uintptr_t data,
                  idl4_server_environment * _env)
{
    uintptr_t __retval = 0;
    struct data *c_data;

    /* implementation of stream::write */
    c_data = malloc(sizeof(struct data));
    assert(c_data != NULL);

    c_data->data = (void *)data;
    c_data->length = length;
    c_data->xferred = 0;
    global_client = _caller;

    /*
     * FIXME: Add to queue
     */
    char_write(device, 1, c_data, write_callback, NULL, (uintptr_t)c_data);

    idl4_set_no_response(_env);
    return __retval;
}

static object_t *sink_instance = NULL;

/*
 * Interface stream
 */
int
stream_set_sink_impl(L4_ThreadId_t _caller, objref_t stream_obj,
                     objref_t sink, idl4_server_environment * env)
{
    int __retval = 0;
    struct data *data;

    sink_instance = object_get_async_interface(sink);
    object_print(sink_instance);
    global_drv_info->sink = sink;

    data = malloc(sizeof(struct data));
    assert(data != NULL);
    data->data =
        (void *)rfl_alloc_first(global_drv_info->read_buffers, &data->length);
    data->length--;             /* Allow for '\0' */
    data->xferred = 0;

    char_read(device, 1, data, read_callback, NULL, (uintptr_t)data);

    return __retval;
}

int irq_count = 0;

static void
read_callback(callback_data_t cb_data, int seqc, struct data *seqv,
              uintptr_t key)
{
    uintptr_t *size;
    uintptr_t *addr;
    L4_MsgTag_t tag = L4_Niltag;

    seqv->data[seqv->xferred] = '\0';
    // DEBUG_PRINT("%s %" PRIxPTR, seqv->data, global_drv_info->sink);
    if (sink_instance == NULL) {
        goto drop_read;
    }

    size = cb_alloc(sink_instance->call_cb, sizeof(uintptr_t));
    if (size == NULL)
        goto drop_read;
    *size = 3 * sizeof(uintptr_t);
    addr = cb_alloc(sink_instance->call_cb, *size);
    if (addr == NULL) {
        cb_undo_alloc(sink_instance->call_cb);
        goto drop_read;
    }
    addr[0] = 0x69;
    addr[1] = (uintptr_t)seqv->data;
    addr[2] = seqv->xferred;
    cb_sync_alloc(sink_instance->call_cb);

    L4_Set_MsgTag(tag);
    L4_Send_Nonblocking(sink_instance->server);

    /* Return unused to the pool */
    rfl_insert_range(global_drv_info->read_buffers,
                     (uintptr_t)seqv->data + seqv->xferred,
                     (uintptr_t)seqv->data + seqv->length);
    /* Indicate we can have more reads */
    global_drv_info->waiting_for_read = 1;
    free(seqv);
    return;

  drop_read:
    /* No sink... we just drop on the floor */
    /* Return unused to the pool */
    rfl_insert_range(global_drv_info->read_buffers, (uintptr_t)seqv->data,
                     (uintptr_t)seqv->data + seqv->length);
    /* Indicate we can have more reads */
    global_drv_info->waiting_for_read = 1;
    free(seqv);
    return;
}

static void
write_callback(callback_data_t data, int seqc, struct data *seqv, uintptr_t key)
{
    uintptr_t retval = seqv->xferred;

    assert(key != 0);
    free((void *)key);
    stream_write_reply(global_client, retval);
}

/*
 * This function is run before every message receieved in the server loop -
 * the name is defined in serial.idl4
 */
void
deferred_work(void)
{
    uintptr_t *size;

    /* Process our own work queues */
    // deferred_work();
    if (global_drv_info && global_drv_info->waiting_for_read) {
        struct data *data;

        global_drv_info->waiting_for_read = 0;
        data = malloc(sizeof(struct data));
        assert(data != NULL);
        data->data =
            (void *)rfl_alloc_first(global_drv_info->
                                    read_buffers, &data->length);
        if ((uintptr_t)data->data != -1) {
            data->length--;
            data->xferred = 0;
            char_read(device, 1, data, read_callback, NULL, (uintptr_t)data);
        } else {
            //DEBUG_PRINT("Couldn't allocate anymore!!!\n");
            free(data);
        }
    }

    /*
     * Now check async queues -- we check this everytime, which is a bit silly,
     * would be nice if we could detect a failed ipc to us
     */
    size = cb_get(iguana_cb_handle, sizeof(uintptr_t));
    if (size) {
        // uintptr_t *data =
        cb_get(iguana_cb_handle, *size);
        // DEBUG_PRINT("Got stuff: %" PRIdPTR "\n", *size);
        // DEBUG_PRINT("Got stuff: %" PRIxPTR " %" PRIxPTR "\n", data[0], data[1]);
    }
}

/*
 * Function called from the server loop when an IRQ is received. See the
 * definition in serial.idl4
 */
void
serial_irq_handler(L4_ThreadId_t partner)
{
    driver_interrupt(device, partner.global.X.thread_no);
}

#if 0
void
device_server(void)
{
    L4_ThreadId_t partner;
    L4_MsgTag_t msgtag;
    idl4_msgbuf_t msgbuf;
    long cnt;
    uintptr_t *size;

    partner = L4_nilthread;
    msgtag.raw = 0;
    cnt = 0;

    while (1) {
        /* Wait for message */
        idl4_msgbuf_sync(&msgbuf);
        idl4_reply_and_wait(&partner, &msgtag, &msgbuf, &cnt);
        if (idl4_is_error(&msgtag)) {
            DEBUG_PRINT("Error %" PRIxPTR "\n", L4_ErrorCode());
            partner = L4_nilthread;
            msgtag.raw = 0;
            cnt = 0;
            continue;
        }

        if (msgtag.X.label == IRQ_LABEL) {
            driver_interrupt(device, partner.global.X.thread_no);

            continue;
        }

        switch (magpie_get_interface_bigid(&msgbuf)) {
        case 37:
            idl4_process_request(&partner, &msgtag, &msgbuf, &cnt,
                                 device_vtable[idl4_get_function_id
                                               (&msgtag) & DEVICE_FID_MASK]);
            break;
        case 38:
            idl4_process_request(&partner, &msgtag, &msgbuf, &cnt,
                                 stream_vtable[idl4_get_function_id
                                               (&msgtag) & STREAM_FID_MASK]);
            break;
        default:
            DEBUG_PRINT("serial: device_server: Bad label! (0x%lx) from %lx\n",
                   magpie_get_interface_bigid(&msgbuf), partner.raw);
            partner = L4_nilthread;
        }
    }
}
#endif
