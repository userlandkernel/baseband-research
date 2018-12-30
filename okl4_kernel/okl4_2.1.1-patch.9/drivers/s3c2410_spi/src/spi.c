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

/*
 * Authors: Nelson Tam, Geoff Lee
 */

/*
 * The device specific driver sets up the default values in relevant
 * registers on the device.  It should be the only part of the
 * driver framework that is aware of register names, their possible
 * values and meaning.  On the other hand it should avoid using
 * magic numbers because the .dx files should define them.
 *
 * It is also responsible for inspecting the tx/rx streams, and when
 * appropriate, push tx data onto the device and push rx data to
 * the client.
 *
 * In the case of this driver, data transmission is triggered by
 * interrupts, so the tx/rx work is done solely in the interrupt
 * handler.  If polling mode is supported (for example), the poll
 * handler will need to do this work also.
 *
 * See also:
 *
 *  - iguana/example/drivers : Memory resource allocation
 *  - iguana/vspi : tx/rx queue management
 *
 * XXX: The more I think about this, this should really be called
 *      the s3c2410_jbt driver.
 *
 * TODO: Move the GPIO bits into another driver / server
 *
 * TODO: Implement micro-sleep
 *
 * - nt
 */

#include "spi.h"
#include <l4/kdebug.h>
#include <util/trace.h>

#include <l4/misc.h>            /* TODO: rm after GPIO server - nt */
#include <iguana/memsection.h>  /* TODO: rm after GPIO server - nt */
#include <iguana/hardware.h>    /* TODO: rm after GPIO server - nt */

#if 0
#define dprintf(arg...) printf(arg)
#else
#define dprintf(arg...) do { } while (0/*CONSTCOND*/);
#endif

/*
 * XXX HACK XXX
 *
 * Move to GPIO server - nt
 *
 */
#define S3C2410_GPIO_PHYS      (0x56000000)

#define S3C2410_GPIO_BANKG     (32 * 6)
#define S3C2410_GPIO_BANKB     (32 * 1)

#define S3C2410_GPIO(bank, offset)    ((bank) + (offset))
#define S3C2410_GPG3          S3C2410_GPIO(S3C2410_GPIO_BANKG, 3)
#define S3C2410_GPG4          S3C2410_GPIO(S3C2410_GPIO_BANKG, 4)
#define S3C2410_GPG5          S3C2410_GPIO(S3C2410_GPIO_BANKG, 5)
#define S3C2410_GPG6          S3C2410_GPIO(S3C2410_GPIO_BANKG, 6)
#define S3C2410_GPG7          S3C2410_GPIO(S3C2410_GPIO_BANKG, 7)

#define S3C2410_GPIO_OFFSET(pin)    ((pin) & 31)
#define S3C2410_GPIO_BANK(pin)      ((pin) / 32)

#define S3C2410_GPIO_BASE(pin)      (S3C2410_GPIO_BANK(pin) * 4)
#define S3C2410_GPCON(pin)          (S3C2410_GPIO_BASE(pin))
#define S3C2410_GPDAT(pin)          (S3C2410_GPIO_BASE(pin) + 1)

#define S3C2410_GPGDAT(base)  ((base) + (0x4))

#define S3C2410_GPG3_nSS1       (0x03 << 6)
#define S3C2410_GPG3_OUTPUT     (0x01 << 6)

#define S3C2410_GPG5_SPIMISO1   (0x03 << 10)
#define S3C2410_GPG5_INPUT      (0x00 << 10)

#define S3C2410_GPG6_SPIMOSI1   (0x03 << 12)
#define S3C2410_GPG6_OUTPUT     (0x01 << 12)

#define S3C2410_GPG7_SPICLK1    (0x03 << 14)
#define S3C2410_GPG7_OUTPUT     (0x01 << 14)

#define S3C2410_GPB3 S3C2410_GPIO(S3C2410_GPIO_BANKB, 3)
#define S3C2410_GPB0 S3C2410_GPIO(S3C2410_GPIO_BANKB, 0)


static uint32_t *gpio_base = NULL;

static void
spi_gpio_cfg(uint32_t *base, uint32_t pin, uint32_t func)
{
    uint32_t mask, cfg;
    uint32_t *addr = (base + S3C2410_GPCON(pin));


    mask = 3 << S3C2410_GPIO_OFFSET(pin) * 2;
    cfg = (*addr & ~mask) | func;

    dprintf("%s: writing 0x%lx to 0x%lx\n", __func__, cfg, (uint32_t)addr);

    *addr = cfg;
}

static void
spi_gpio_set(uint32_t *base, uint32_t pin, uint32_t val)
{
    uint32_t mask, dat;
    uint32_t *addr = (base + S3C2410_GPDAT(pin));

    mask = 1 << S3C2410_GPIO_OFFSET(pin);
    dat = (*addr & ~mask) | (val << S3C2410_GPIO_OFFSET(pin));

    dprintf("%s: writing 0x%lx to 0x%lx\n", __func__, dat, (uint32_t)addr);

    *addr = dat;
}

static void
spi_config_gpio(struct s3c2410_spi *device)
{
    uintptr_t ms, p_ms;
    (void)device;

    /* XXX what to do if fail? -gl */
    if (gpio_base == NULL)
    {
        ms = memsection_create_user(0x1000, &p_ms);
        hardware_back_memsection(ms, S3C2410_GPIO_PHYS, L4_IOMemory);
        gpio_base = (uint32_t *)p_ms;
    }

//    Turn on the backlight
    spi_gpio_cfg(gpio_base, S3C2410_GPB0, 0x01);
    spi_gpio_set(gpio_base, S3C2410_GPB0, 1);

//    But not the vibrator!
//    spi_gpio_cfg(gpio_base, S3C2410_GPB3, S3C2410_GPG3_OUTPUT);
//    spi_gpio_set(gpio_base, S3C2410_GPB3, 1);

    /* Put the MOSI/MISO pins in a known-state */
    spi_gpio_cfg(gpio_base, S3C2410_GPG6, S3C2410_GPG6_OUTPUT);

    spi_gpio_cfg(gpio_base, S3C2410_GPG7, S3C2410_GPG7_OUTPUT);
    spi_gpio_set(gpio_base, S3C2410_GPG6, 1);
    spi_gpio_set(gpio_base, S3C2410_GPG7, 1);

    /* disable CS */
    spi_gpio_cfg(gpio_base, S3C2410_GPG3, S3C2410_GPG3_OUTPUT);
    spi_gpio_set(gpio_base, S3C2410_GPG3, 1);

    /*
     * Set the GPIO pins to functional mode, i.e. we will use
     * SPI as a UART instead of bit-banging - nt
     */
    spi_gpio_cfg(gpio_base, S3C2410_GPG5, S3C2410_GPG5_SPIMISO1);
    spi_gpio_cfg(gpio_base, S3C2410_GPG6, S3C2410_GPG6_SPIMOSI1);
    spi_gpio_cfg(gpio_base, S3C2410_GPG7, S3C2410_GPG7_SPICLK1);

    /*
     * conf pull-up register
     * get LCM out of reset
     *
     * nt - don't worry, this is done by u-boot
     */

    return;
}

static int
spi_set_master_mode(struct s3c2410_spi *device)
{
    spcon_set_mstr(1);
    return 0;
}

static int
spi_enable_sck(struct s3c2410_spi *device)
{
    spcon_set_ensck(1);
    return 0;
}

static int
spi_set_clock_mode(struct s3c2410_spi *device)
{
    spcon_set_cpol(1);
    spcon_set_cpha(1);
    return 0;
}

#define DEFAULT_PRESCALE    0xff        /* from linux driver */
#define spi_set_def_prescale(d)   spi_set_prescale(d, DEFAULT_PRESCALE)
static int
spi_set_prescale(struct s3c2410_spi *device, uint8_t prescale)
{
    sppre_set_scale(prescale);
    return 0;
}

static int
spi_enable_interrupt_mode(struct s3c2410_spi *device)
{
    spcon_set_smod(0x1);    /* 01 = interrupt mode */
    return 0;
}

static int
spi_disable_interrupt_mode(struct s3c2410_spi *device)
{
    spcon_set_smod(0x0);    /* 00 = polling mode */
    return 0;
}

static int spi_set_params(struct s3c2410_spi *device)
{
    spi_set_def_prescale(device);
    spi_set_master_mode(device);
    spi_set_clock_mode(device);

    spi_enable_sck(device);

    sppin_set_keep(0x1);    /* stay in master mode after tx */
    sppin_set_resv1(0x1);   /* according to spec this should be 1 */

    dprintf("%s: done\n", __func__);

    return 0;
}



static inline void
spi_nCS (uint8_t bit)
{
    spi_gpio_set(gpio_base, S3C2410_GPG3, bit);
}



enum jbt_write_mode
{
    JBT_WRITE_NODATA    = 1,
    JBT_WRITE_8         = 2,
    JBT_WRITE_16        = 3,
};

#define JBT_COMMAND 0x000
#define JBT_DATA    0x100

#define spi_bus_write_nodata(cmd)   \
    spi_bus_write(device, cmd, 0, JBT_WRITE_NODATA)

#define spi_bus_write8(cmd, data)   \
    spi_bus_write(device, cmd, data, JBT_WRITE_8)

#define spi_bus_write16(cmd, data)  \
    spi_bus_write(device, cmd, data, JBT_WRITE_16)

#define SPI_WAIT()                                  \
    do {                                            \
        while (!spsta_get_redy()) dprintf(".");     \
        dprintf("\n");                              \
    } while (0)

static int
spi_bus_write
(
    struct s3c2410_spi  *device,
    uint8_t             cmd,
    uint16_t            data,
    enum jbt_write_mode mode
)
{
    int         i;
    uint16_t    buf[3];

    dprintf("%s: writing %2.2x : %4.4x (mode %d)\n",
            __func__, cmd, data, mode);

    switch (mode)
    {
    default:
        printf("%s: warning! unknown spi write mode %d\n", __func__, mode);
        return 1;

    case JBT_WRITE_16:
        buf[2] = JBT_DATA | (data & 0xff);
        data >>= 8;
        /* FALL THROUGH */
    case JBT_WRITE_8:
        buf[1] = JBT_DATA | (data & 0xff);
        /* FALL THROUGH */
    case JBT_WRITE_NODATA:
        buf[0] = JBT_COMMAND | cmd;
        break;
    }

    /* write data to reg */
    spi_nCS(0);                 /* chip-select the slave */

    for (i = 0; i < mode; i++)
    {
        dprintf("%s: writing byte %2.2x on wire\n", __func__, buf[i] & 0xff);

        SPI_WAIT();
        sptdat_set_txdata(buf[i] & 0xff);

        dprintf("%s: writing byte %2.2x on wire\n", __func__, buf[i] >> 8);

        SPI_WAIT();
        sptdat_set_txdata(buf[i] >> 8);
    }

    spi_nCS(1);                 /* un-select the slave */

    return 0;
}





static int
device_setup_impl(struct device_interface *di, struct s3c2410_spi *device,
                  struct resource *resources)
{
    for (; resources && (resources->type != BUS_RESOURCE && resources->type != MEMORY_RESOURCE); 
        resources = resources->next);

    if (!resources)
        return 0;

    device->main = *resources;

    device->tx.device = device;
    device->rx.device = device;
    device->tx.ops = stream_ops;
    device->rx.ops = stream_ops;

    spi_set_params(device);
    spi_config_gpio(device);

    /* TODO: do everything that is done in jbt_init_regs */

    dprintf("%s: done\n", __func__);

    return DEVICE_SUCCESS;
}

static int
device_enable_impl(struct device_interface *di, struct s3c2410_spi *device)
{
    spi_enable_interrupt_mode(device);

    device->state = STATE_ENABLED;

    dprintf("%s: done\n", __func__);

    return DEVICE_ENABLED;
}

static int
device_disable_impl(struct device_interface *di, struct s3c2410_spi *device)
{
    spi_disable_interrupt_mode(device);

    /* XXX: hack-sleep the LCD screen */
    spi_bus_write_nodata(0x28);
    spi_bus_write8(0xb0, 0x17);

    device->state = STATE_DISABLED;

    dprintf("%s: done\n", __func__);

    return DEVICE_DISABLED;
}

static int
device_poll_impl(struct device_interface *di, struct s3c2410_spi *device)
{
    dprintf("%s: called.\n", __func__);
    return 0;
}

/*
 * XXX clagged from imx21 spi driver: should be autogenerated.
 */
static int
device_num_interfaces_impl(struct device_interface *di, 
                           struct s3c2410_spi *device)
{
    dprintf("%s: called.\n", __func__);

    return 2;    /* XXX: in reality there are 2 spi interfaces, i.e. 4 stream interfaces - nt */
}

/*
 * XXX clagged from imx21 spi driver: should be autogenerated.
 */
static struct generic_interface *
device_get_interface_impl(struct device_interface *di,
                          struct s3c2410_spi *device, int i)
{
    switch (i) {
        case 0:
            return (struct generic_interface *)&device->tx;
        case 1:
            return (struct generic_interface *)&device->rx;
        default:
            return NULL;
    }
}

static int
do_txrx_work(struct s3c2410_spi *device)
{
    dprintf("%s: called\n", __func__);

    struct stream_interface *txsi = &device->tx;
    struct stream_interface *rxsi = &device->rx;

    struct stream_pkt *tx_pkt = stream_get_head(txsi);
    struct stream_pkt *rx_pkt = stream_get_head(rxsi);

    /*
     * tx phase - this triggers the SPI device to push data
     * into the rx_data register also
     *
     * XXX: checks and asserts? - nt
     * XXX: I don't even know if stream_pkt make sense for SPI
     */
    if (tx_pkt != NULL)
        sptdat_set_txdata(tx_pkt->data[tx_pkt->xferred++]);
    else
        sptdat_set_txdata(0xff); /* write dummy data */

    /* rx phase
     *
     * XXX: see tx phase - nt
     */
    if (rx_pkt != NULL)
        rx_pkt->data[rx_pkt->xferred++] = sprdat_get_rxdata();

    return 0;
}

/*
 * Note: device is always in interrupt mode thus data transfers
 * are done in this function
 */
/*
 * TODO: check for errors - MULF
 *
 */
static int
device_interrupt_impl(struct device_interface *di,
                      struct s3c2410_spi *device,
                      int irq)
{
    int rv = 1;

    dprintf("%s: in interrupt\n", __func__);

    if (spsta_get_dcol())
    {
        dprintf("%s: data collision error\n", __func__);
    }

    /* XXX: multi-master is not detected */

    rv = do_txrx_work(device);

    dprintf("%s: interrupt done, rv = %d.\n", __func__, rv);

    return 0;
}

/*
 * Flush buffers.  This includes any hardware FIFOs (irrelevant to this
 * SPI device) and perhaps stream packet queues.  So make sure packet
 * queues are flushed at the end of do_txrx_work().
 *
 * - nt
 */
static int
stream_sync_impl(struct stream_interface *si, struct s3c2410_spi *device)
{
    int rv = 0;

    dprintf("%s: stream %p dev %p\n", __func__, si, device);

    if (si == &device->tx || si == &device->rx)
        rv = do_txrx_work(device);

    dprintf("%s: done.\n", __func__);

    return rv;
}
