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
 * Author: Josh Matthews <jmatthews@ok-labs.com>
 * Created: Friday December 14 2007
 * Description: stdio over serial for POSIX environments
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <l4/thread.h>
#include <l4/kdebug.h>
#include <l4/misc.h>
#include <l4/ipc.h>

#include <iguana/env.h>
#include <iguana/thread.h>
#include <iguana/physmem.h>
#include <iguana/memsection.h>

#ifdef THREAD_SAFE
#include <mutex/mutex.h>
#endif

#include <interfaces/vserial_client.h>

extern struct __file __stdin;
extern struct __file __stdout;
extern struct __file __stderr;

int serial_init(void);
int serial_fopen_init(FILE *stream);

void posix_flush_stdio(void);

/*
 * Initialisation
 */
#define SERIAL_INIT_UNINITIALISED  (-1)
#define SERIAL_INIT_ERROR (1)
#define SERIAL_INIT_OK (0)
static int8_t serial_init_result = SERIAL_INIT_UNINITIALISED;

/***************************/
/*    SERIAL VARIABLES     */
/***************************/

#define BUFFER_SIZE 0x2000
#define PACKET_SIZE 16

#define TERMINATED 1
#define COMPLETED 2

/* For communication with the serial server */
struct serial_control_block {
    volatile uintptr_t tx;
    volatile uintptr_t rx;
};

/* First part must match structure of stream_packet in serial server */
struct stream_packet {
    uintptr_t next;
    uintptr_t data_ptr;
    size_t size;
    size_t xferred;
    volatile int status;
    char data[PACKET_SIZE];
};

struct ig_serial {
    server_t server;
    device_t handle;

    memsection_ref_t ms;
    uintptr_t ms_base;
    struct serial_control_block *control;

    /* Free list of packets */
    struct stream_packet *free_list;

    /* Head of list of transmitted packets */
    struct stream_packet *tx_complete;

    /* Last in list of transmitted packets */
    struct stream_packet *tx_last;

    /* Head of list of received packets */
    struct stream_packet *rx_complete;

    /* Last in list of received packets */
    struct stream_packet *rx_last;

    int available;
};

static inline uintptr_t
vaddr_to_memsect(uintptr_t base, void *vaddr) {
    return ((uintptr_t) vaddr - base) << 4;
}

static inline void *
memsect_to_vaddr(uintptr_t base, uintptr_t memsect) {
    return (void *) (base + (memsect >> 4));
}

/* Our serial device */
static struct ig_serial *serial;


/***************************/
/*    NULL IO FUNCTIONS    */
/***************************/

#ifdef NDEBUG
static size_t
null_write(const void *data, long int position, size_t count, void *handle /*unused*/)
{
    return count;
}

static size_t
null_read(void *data, long int position, size_t count, void *handle /*unused*/)
{
    return 0;
}

#else
/***************************/
/*    KDB IO FUNCTIONS     */
/***************************/

static size_t
l4kdb_write(const void *data, long int position, size_t count,
            void *handle /* unused */ )
{
    size_t i;
    const char *real_data = data;

    for (i = 0; i < count; i++)
        (void)L4_KDB_PrintChar(real_data[i]);
    return count;
}

static size_t
l4kdb_read(void *data, long int position, size_t count,
           void *handle /* unused */ )
{
    size_t i;
    char *real_data = data;

    for (i = 0; i < count; i++) {
        do {
            if (OKL4_KDB_RES_UNIMPLEMENTED == L4_KDB_ReadChar(&real_data[i]))
                return 0;
            if (real_data[i] != 0) {
                break;
            }
        } while (1);
    }
    return count;
}
#endif


/************************************************************/
/*            SERIAL DEVICE SPECIFIC FUNCTIONS              */
/* These functions can be re-implemented if the underlying  */
/* serial device changes in some drastic way                */
/************************************************************/

/* Transmit packets */
static inline void
serial_device_transmit(void)
{
    L4_Notify(serial->server, 0x1);
}

/* Notify the server (for whatever reason) */
static inline void
serial_device_notify(void)
{
    L4_Notify(serial->server, 0x1);
}

/* Shutdown the serial device */
static void
serial_device_shutdown(void)
{
    /* Shutdown the virtual device */
    virtual_serial_shutdown(serial->server, serial->handle, NULL);

    /* Free our allocated resources */
    memsection_delete(serial->ms);
    free(serial);
}

/* Receive packets */
static inline void
serial_device_receive(void)
{
    L4_ThreadId_t sender;
    L4_Wait(&sender);
}

static int
serial_device_init(void)
{
    L4_ThreadId_t my_tid;
    my_tid = thread_l4tid(thread_myself());

    /* Ensure we have been given a vserial instance */
    const envitem_t* vserial_tid = iguana_getenv("VSERIAL_TID");
    const envitem_t* vserial_handle = iguana_getenv("VSERIAL_HANDLE");
    if (vserial_tid == NULL || vserial_handle == NULL) {
        return SERIAL_INIT_ERROR;
    }
    
    /* Create a serial instance */
    serial = malloc(sizeof(struct ig_serial));
    if (!serial) {
        return SERIAL_INIT_ERROR;
    }
    serial->server = env_thread_id(vserial_tid);
    serial->handle = env_const(vserial_handle);

    /* Initialize a shared ms of size BUFFER_SIZE (must be multiple of page size)
       Give the ms to the server. Note that the password is zero by default */
    serial->ms = memsection_create(BUFFER_SIZE, &(serial->ms_base));
    virtual_serial_add_memsection(serial->server, serial->handle, serial->ms, 0, 0, NULL);

    /* Initialize the control block at the start of the memsection */
    serial->control = (struct serial_control_block*)serial->ms_base;
    serial->control->tx = ~0;
    serial->control->rx = ~0;
    serial->free_list = (struct stream_packet*)(serial->ms_base + sizeof(struct serial_control_block));

    /* Register the control block with the server */
    virtual_serial_register_control_block(serial->server,
                                          serial->handle,
                                          vaddr_to_memsect(serial->ms_base, serial->control), NULL);

    /* Initialise the free list of packets */
    int i;
    int num_packets = (BUFFER_SIZE - sizeof(struct serial_control_block)) / sizeof(struct stream_packet);
    for (i = 0; i < num_packets; i++){
        struct stream_packet * this_packet = &serial->free_list[i];
        this_packet->next = (uintptr_t) &serial->free_list[i+1];
        this_packet->data_ptr = vaddr_to_memsect(serial->ms_base, &this_packet->data);
        serial->available += PACKET_SIZE;
    }
    serial->free_list[i-1].next = 0;
    serial->tx_last = NULL;
    serial->tx_complete = NULL;
    serial->rx_last = NULL;
    serial->rx_complete = NULL;

    /* Now feed some packets to the server so it may RX into them */
    for (i = 0; i < 10; i++){
        struct stream_packet *packet = packet = serial->free_list;
        assert(packet);

        if (serial->rx_complete == NULL) {
            serial->rx_complete = packet;
        }

        serial->free_list = (void*) packet->next;
        packet->next = ~0; 
        packet->size = PACKET_SIZE;
        packet->xferred = 0;
        packet->status = 0;
        serial->available -= PACKET_SIZE;

        if (serial->rx_last != NULL) {
            serial->rx_last->next = vaddr_to_memsect(serial->ms_base, packet);
        } else {
            serial->control->rx = vaddr_to_memsect(serial->ms_base, packet);
        }
        serial->rx_last = packet;
    }

    L4_Notify(serial->server, 0x1);

    /* Accept 0x1 messages from the serial server */
    L4_Set_NotifyMask(0x1);
    L4_Accept(L4_NotifyMsgAcceptor);

    virtual_serial_init(serial->server, serial->handle, my_tid.raw, 0x1, NULL);

    return SERIAL_INIT_OK;
}

/****************************/
/*     POLICY FUNCTIONS     */
/****************************/

/* Policies allow arbitrary behaviour to be executed upon receipt or
   transmission of each character.
   You are provided with the data and index in the packet,
   the buffer and index of the char* that is being read/written,
   the count of chars in the data packet (that you can modify,
   if you remove data - it is initialized to the total)
   and the count of chars in the buffer (same)
   Return -1 if the calling function is to break (ie: transmit the
   packet immediately with no further additions), 0 otherwise
*/

static int
write_policy(char c, char* data, int* data_idx, char* buffer, int* buffer_idx,
             int* data_count, int* buffer_count)
{
    /* Add to the packet */
    data[(*data_idx)++] = c;

    /* If a new line */
    if (c == '\n') {

        /* If the \n is the last char in the packet, we save it for the
           next packet (since we can't fit the \r) */
        if (*data_idx == PACKET_SIZE) {
            (*data_count)--;
            (*buffer_count)--;
            /* Break */
            return -1;
        }

        /* Otherwise, inject a \r as long as the next char isn't already a \r */
        if (!(*buffer_idx != *buffer_count && buffer[*buffer_idx] == '\r')) {
            data[(*data_idx)++] = '\r';
            (*data_count)++;
        }
    }
    return 0;
}


static int
read_policy(char c, char* data, int* data_idx, char* buffer, int* buffer_idx,
            int* data_count, int* buffer_count)
{
    /* Insert the received char
       If it is a \r, replace with a \n */
    if (c == '\r') c = '\n';
    buffer[(*buffer_idx)++] = c;

    /* Echo the character to the screen
       This assumes we are on stdin */
    __stdout.buffering_mode = _IONBF;
    fputc(c, &__stdout);
    __stdout.buffering_mode = _IOLBF;

    return 0;
}

/****************************/
/*   SERIAL IO FUNCTIONS   */
/****************************/

static int
serial_free_packets(struct stream_packet ** list_head, struct stream_packet ** list_end)
{
    int count = 0;
    struct stream_packet *packet;
    uintptr_t next;
    packet = *list_head; //serial->tx_complete;
    
    while(packet) {

        /* Keep finding packets until we hit an uncompleted one */
        next = packet->next;
        if ((packet->status & COMPLETED) == 0) {
            break;
        }
        count++;

        /* Append to free list */
        packet->next = (uintptr_t) serial->free_list;
        serial->free_list = packet;
        serial->available += PACKET_SIZE;
        if (*list_end == packet) {
            *list_end = NULL;
        }
        if (next == ~0) {
            packet = NULL;
        } else {
            packet = memsect_to_vaddr(serial->ms_base, next);
        }
    }
    *list_head = packet;
    return count;
}

/* Write Interface */
/* Lock will have already been called on the stream */
static size_t
serial_write(const void *data, long int position, size_t count, void *handle)
{
    /* Position argument is position to write to in underlying file.
       This is a device, so we disregard */
    struct stream_packet *packet;
    int transmitted = 0;
    int remaining = count;
    int kick = 0;
    char *buffer = (char*)data;

    while (transmitted < count) {
        
        /* Obtain a packet from the free list to use
           and remove it from the free list */
        packet = serial->free_list;

        /* If there are no free packets, kick the server to send some
           and attempt to free on our side, then keep trying */
        if (packet == NULL) {
            serial_device_transmit();
            serial_free_packets(&(serial->tx_complete), &(serial->tx_last));
            continue;
        }
        
        /* Remove the packet from the free list */
        serial->free_list = (void*) packet->next;

        /* Determine the amount of bytes to transfer in this packet */
        int data_count = (remaining > PACKET_SIZE) ? PACKET_SIZE : remaining;

        /* Copy the data into the packet and adjust counters */
        int data_idx_cur = 0, buffer_idx_cur = 0;
        int buffer_count  = data_count;
        while (buffer_idx_cur < buffer_count) {

            /* Obtain the character */
            char c = buffer[buffer_idx_cur++];

            /* Call the write policy
               It will add the packet, or do whatever else it wants
               Policies return -1 if we should break */
            if (write_policy(c, (char*)&(packet->data), &data_idx_cur,
                            buffer, &buffer_idx_cur,
                            &data_count, &buffer_count) == -1) {
                break;
            }
        }

        buffer += buffer_count;
        transmitted += buffer_count;
        remaining -= buffer_count;
        serial->available -= PACKET_SIZE;

        /* Set the packet attributes */
        packet->next = ~0;
        packet->size = data_count;
        packet->status = 0;

        /* We want to add the packets to the tx_complete list
           If the list is null, make this packet the head */
        if (serial->tx_complete == NULL) serial->tx_complete = packet;

        /* If there is a last packet in the completed tx list,
           set this packet to be the next packet
           If the last one has been terminated (ie: sent), this is a new ring
           If there isn't a last packet, this is definitely a new ring */
        int new_q = 0;
        if (serial->tx_last != NULL) {
            serial->tx_last->next = vaddr_to_memsect(serial->ms_base, packet);
            if (serial->tx_last->status & TERMINATED) {
                new_q = 1;
            }
        } else {
            new_q = 1;
        }

        /* If this is new ring, set the control blocks first tx packet to this packet */
        if (new_q) {
            kick = 1;
            serial->control->tx = vaddr_to_memsect(serial->ms_base, packet);
        }
        serial->tx_last = packet;
    }
    if (kick) serial_device_transmit();

    return transmitted;
}

/* Read Interface */
static size_t
serial_read(void *data, long int position, size_t count, void *handle)
{
    char* buffer = data;
    int received = 0;
    int buffer_idx_cur = 0;

    while (received < count) {

        /* Receive from the serial device
           This waits until the serial device alerts us that there are packets
           to be consumed */
        serial_device_receive();

        /* Process each packet */
        struct stream_packet *next, *packet = serial->rx_complete;
        int new_q = 0;
        while (packet) {
            new_q = 0;
            next  = (void*)packet->next;

            /* If this packet isn't done, break */
            if (!(packet->status & COMPLETED)) {
                break;
            }

            /* Determine the number of characters we are able to receive */
            int data_count = count - received > packet->xferred ? packet->xferred : count - received;
            int buffer_count = data_count;

            /* Copy the received data into the buffer */
            for (int data_idx_cur = 0; data_idx_cur < data_count; data_idx_cur++) {

                /* Obtain the character */
                char c = packet->data[data_idx_cur];

                /* Call the read policy to do its work */
                if (read_policy(c, (char*)&(packet->data), &data_idx_cur,
                                buffer, &buffer_idx_cur,
                                &data_count, &buffer_count) == -1) {
                    break;
                }
            }

            /* Add the number of characters actually received
               This may have been adjusted in the read policy */
            received += buffer_count;

            /* Free the packet they were delivered in */
            packet->next = ~0;
            packet->size = PACKET_SIZE;
            packet->xferred = 0;
            packet->status = 0;

            if (serial->rx_last != NULL) {
                serial->rx_last->next = vaddr_to_memsect(serial->ms_base, packet);
                if (serial->rx_last->status & TERMINATED) {
                    new_q = 1;
                }
            } else {
                new_q = 1;
            }
            if (new_q) {
                serial->control->rx = vaddr_to_memsect(serial->ms_base, packet);
            }
            serial->rx_last = packet;

            /* Obtain the next packet */
            if ((uintptr_t)next == ~0) {
                packet = NULL;
            } else {
                packet = memsect_to_vaddr(serial->ms_base, (uintptr_t)next);
            }
        }

        /* Notify the server of the new empty packet if necessary */
        if (new_q) serial_device_notify();
        serial->rx_complete = packet;
    }

    return received;
}

static void
serial_flush(int count)
{
    int done;
    L4_ThreadId_t dummy;
    struct stream_packet *packet;

    /* If anything was written, wait until the serial server has completely finished */
    if (count > 0) {
        do {

            /* Transmit packets, if any */
            serial_device_transmit();
            serial_free_packets(&(serial->tx_complete), &(serial->tx_last));

            /* Wait for the serial server to kick us. */
            L4_Wait(&dummy);

            /* Ensure until all tx packets have completed. */
            done = 1;
            packet = serial->tx_complete;
            while (packet) {

                /* If this packet isn't done, we're not either. */
                if (!(packet->status & COMPLETED) || !(packet->status & TERMINATED)) {
                    done = 0;
                    break;
                }

                /* Obtain the next packet. */
                packet = (packet->next != ~0) ? 
                    memsect_to_vaddr(serial->ms_base, packet->next) : NULL;
            }
    
        } while (!done);
    }
}

/***************************/
/*       SHUT DOWN         */
/***************************/

static int
serial_close(void* handle /* Unused */)
{
    /* Shut down the serial device */

    /* Re-wire stdio to use L4_KDB functions */
    __stdout.write_fn = l4kdb_write;
    __stdout.close_fn = NULL;
    __stderr.write_fn = l4kdb_write;
    __stdin.read_fn = l4kdb_read;
    __stdout.flush_fn = NULL;
    __stderr.flush_fn = NULL;
    
    serial_device_shutdown();
    return 0;
}

/***************************/
/*      INITIALIZATION     */
/***************************/

int
serial_fopen_init(FILE *handle)
{
    /* Ensure serial has been initialised. */
    assert(serial_init_result != SERIAL_INIT_UNINITIALISED);

    if (handle != NULL) {
        /* Associate stdio functions with serial device */
        handle->write_fn = serial_write;
        handle->read_fn = serial_read;

        /* Wire the close fp of stdout to serial_close, to shutdown the serial
           device on sys_exit */
        handle->close_fn = serial_close;

#ifdef NDEBUG
        __stdout.write_fn = null_write;
        __stderr.write_fn = null_write;
#else
        __stdout.write_fn = l4kdb_write;
        __stderr.write_fn = l4kdb_write;
#endif
    }

    return 0;
}

/* Creates the serial device, initializes mutex for stdout/stdin/stderr
 * Returns 0 on success, 1 on error
 * Possible errors include not finding serial device, out of mem etc. */
int
serial_init(void)
{
    /* If already initialized, return immediately */
    if (serial_init_result != SERIAL_INIT_UNINITIALISED) {
        return serial_init_result;
    }

    serial_init_result = 1;
    return 1;
    /* Attempt to initialize the serial device
       Return failure if an error occurs */
    if (serial_device_init() != SERIAL_INIT_OK) {
        return (serial_init_result = SERIAL_INIT_ERROR);
    }

    /* Make stdout line buffered, with a char buffer of size BUFSIZ
       stderr is not buffered */
    setvbuf(&__stdout, NULL, _IOLBF, BUFSIZ);

    /* Associate stdio functions with serial device */
    __stdout.write_fn = serial_write;
    __stdin.read_fn = serial_read;
    __stderr.write_fn = serial_write;
    __stdout.flush_fn = serial_flush;
    __stderr.flush_fn = serial_flush;

    /* Wire the close fp of stdout to serial_close, to shutdown the serial
       device on sys_exit */
    __stdout.close_fn = serial_close;

    /* Return success */
    return (serial_init_result = SERIAL_INIT_OK);
}

/***************************/
/*    IO INIT FUNCTIONS    */
/***************************/

static size_t
init_write(const void *data, long int position, size_t count,
           void *handle /*unused*/)
{
    /* Ensure serial has been initialised. */
    assert(serial_init_result != SERIAL_INIT_UNINITIALISED);

    /* If serial initialisation failed, fall back to using KDB
     * for serial I/O. */
    if (serial_init_result != SERIAL_INIT_OK) {
#ifdef NDEBUG
        __stdout.write_fn = null_write;
        __stderr.write_fn = null_write;
#else
        __stdout.write_fn = l4kdb_write;
        __stderr.write_fn = l4kdb_write;
#endif
    }

    /* Call the write function and return its result */
    return __stderr.write_fn(data, position, count, handle);
}

static size_t
init_read(void *data, long int position, size_t count,
          void *handle /*unused*/)
{
    /* Ensure serial has been initialised. */
    assert(serial_init_result != SERIAL_INIT_UNINITIALISED);

    /* If serial initialisation failed, fall back to using KDB
     * for serial I/O. */
    if (serial_init_result != 0) {
#ifdef NDEBUG
        __stdin.read_fn = null_read;
#else
        __stdin.read_fn = l4kdb_read;
#endif
    }

    /* Call the read function and return its result */
    return __stdin.read_fn(data, position, count, handle);
}


/***************************/
/*   STD FD DECLARATIONS   */
/***************************/

struct __file __stdin = {
    (void *)0,
    init_read,
    NULL,
    NULL,
    NULL,
    _IONBF,
    NULL,
    0,
    0,
    0,
};

struct __file __stdout = {
    (void *)1,
    NULL,
    init_write,
    NULL,
    NULL,
    _IONBF,
    NULL,
    0,
    0,
    0,
};

struct __file __stderr = {
    (void *)2,
    NULL,
    init_write,
    NULL,
    NULL,
    _IONBF,
    NULL,
    0,
    0,
    0,
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;
