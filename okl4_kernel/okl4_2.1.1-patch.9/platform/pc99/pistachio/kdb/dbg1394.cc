/*
 * Copyright (c) 2001, 2006, National ICT Australia
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
 * Description:   ieee1394 OHCI debug
 */
/*-
 * Copyright (c) 2003 Hidetoshi Shimokawa
 * Copyright (c) 1998-2002 Katsushi Kobayashi and Hidetoshi Shimokawa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the acknowledgement as bellow:
 *
 *    This product includes software developed by K. Kobayashi and H. Shimokawa
 *
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/dev/firewire/fwohci.c,v 1.81 2005/03/29 01:44:59 sam Exp $
 *
 */
/*lint -e648 */
#include <macros.h>
#include <types.h>
#include <config.h>
#include <arch/cpu.h>
#include <arch/ioport.h>
#include <tcb.h>
#include <kdb/kdb.h>
#include <kdb/init.h>
#include <kdb/input.h>
#include <kdb/cmd.h>
#include <kdb/console.h>
#include <init.h>
#include <debug.h>

#include "fwohcireg.h"
#if !defined(_lint)
#define printf(x...)
#endif
typedef void * ohci_t;

word_t  fwaddr_phys, fwaddr_virt = 0;

/* whether or not anybody is listening on firewire */
int stand_alone = 0;

#define SEC_EFI_IO              SEC_KDEBUG
#define SEC_EFI_DATA            SEC_KDEBUG_DATA

static int bus_scan( int *, int *, int * ) SECTION (SEC_EFI_IO);
static int pcireg_cfgread(int, int, int, int, int) SECTION (SEC_EFI_IO);
static void pcireg_cfgwrite(int, int, int, int, int, int ) SECTION (SEC_EFI_IO);
static int  start_driver( int, int, int ) SECTION (SEC_EFI_IO);
static void dump_nid_status( ohci_t ) SECTION (SEC_EFI_IO);
static void dump_sid_status( ohci_t ) SECTION (SEC_EFI_IO);
static void handle_bus_reset( ohci_t ) SECTION (SEC_EFI_IO);
static void check_bus_reset(void) SECTION (SEC_EFI_IO);
static void dump_int_status( ohci_t ) SECTION (SEC_EFI_IO);
static int  fwohci_init( ohci_t ) SECTION (SEC_EFI_IO);
static void fwohci_ibr( ohci_t ) SECTION (SEC_EFI_IO);
static void fwohci_reset( ohci_t ) SECTION (SEC_EFI_IO);
static int  fwohci_probe_phy( ohci_t ) SECTION (SEC_EFI_IO);
static void fill_crom( ohci_t ) SECTION (SEC_EFI_IO);
static u16_t fw_crc16(u32_t *, u32_t) SECTION (SEC_EFI_IO);
static u32_t fwphy_wrdata( ohci_t, u32_t, u32_t) SECTION (SEC_EFI_IO);
static u32_t fwphy_rddata( ohci_t,  unsigned int) SECTION (SEC_EFI_IO);
#if 0   /*UNUSED*/
static u32_t fwohci_set_bus_manager( ohci_t, unsigned int) SECTION (SEC_EFI_IO);
#endif
static void DELAY(int) SECTION (SEC_EFI_IO);
static void bus_space_write_4( void *, unsigned, u32_t ) SECTION (SEC_EFI_IO);
static u32_t bus_space_read_4( void *, unsigned offset ) SECTION (SEC_EFI_IO);

/* used externally from io.c */
void init_dbg1394(void) SECTION (SEC_EFI_IO);
void putc_dbg1394(char)  SECTION (SEC_EFI_IO);
char getc_dbg1394(bool)  SECTION (SEC_EFI_IO);


/* in init.cc -- add a 4mb mapping to the init PTs */
void add_init_root_mapping( void *vaddr );


/************************* Mem Serial Code *************************/

volatile struct
_serial_buf/*XXX gcc*/ {
        u32_t send_data;
        u32_t send_ack;
        u32_t recv_data;
        u32_t recv_ack;
} serial_buf  SECTION(SEC_EFI_DATA);


#if 0
int test_dbg1394(void)  SECTION (SEC_EFI_IO);
int
test_dbg1394(void)
{
  return serial_buf.send_data;

  while(1)
    {
      serial_buf.send_data = 'R';
      serial_buf.send_ack = 1;

      while( serial_buf.send_ack );
    }

}
#endif

void
putc_dbg1394( char c )
{
        while( serial_buf.send_ack )
        {
                check_bus_reset();

                /* drop chars if no other node */
                if( stand_alone )
                        break;
        }

        serial_buf.send_data = c;
        serial_buf.send_ack = 1;
}

char
getc_dbg1394( bool block )
{
        char c;

        while( serial_buf.recv_ack != 1 )
        {
                check_bus_reset();
                if( !block )
                        return -1;
        }

        c = serial_buf.recv_data;
        serial_buf.recv_ack = 0;

        return c;
}

#if defined(CONFIG_KDB_BREAKIN) && defined(CONFIG_KDB_CONS_DBG1394)
void SECTION (SEC_EFI_IO)
kdebug_check_breakin (void)
{
        if( kdb_consoles[kdb_current_console].getc( 0 ) == 27 )  /* escape */
            enter_kdebug("breakin");
}
#endif /* CONFIG_KDB_BREAKIN */

/************************* PCI Scanning *************************/

extern space_t * kernel_space;
#define CONF1_ADDR_PORT    0x0cf8
#define CONF1_DATA_PORT    0x0cfc

static int
pcireg_cfgread(int bus, int slot, int func, int reg, int bytes)
{
        int data = -1;
        int port;


        out_u32(CONF1_ADDR_PORT, (1 << 31)
                | (bus << 16) | (slot << 11)
                | (func << 8) | (reg & ~0x03));
        port = CONF1_DATA_PORT + (reg & 0x03);

        if (port != 0) {
                switch (bytes) {
                case 1:
                        data = in_u8(port);
                        break;
                case 2:
                        data = in_u16(port);
                        break;
                case 4:
                        data = in_u32(port);
                        break;
                }
                out_u32(CONF1_ADDR_PORT, 0);
        }

        return (data);
}

static void
pcireg_cfgwrite(int bus, int slot, int func, int reg, int bytes, int data )
{
        int port;

        out_u32(CONF1_ADDR_PORT, (1 << 31)
                | (bus << 16) | (slot << 11)
                | (func << 8) | (reg & ~0x03));
        port = CONF1_DATA_PORT + (reg & 0x03);

        if (port != 0) {
                switch (bytes) {
                case 1:
                        out_u8(port, data);
                        break;
                case 2:
                        out_u16(port, data);
                        break;
                case 4:
                        out_u32(port, data);
                        break;
                }
                out_u32(CONF1_ADDR_PORT, 0);
        }
}


#define BUS_MAX 32
static int
bus_scan( int *rbus, int *rdevice, int *rfunction )
{
        int bus, device, function, func_max;
        unsigned int id;

        for (bus = 0; bus < BUS_MAX; bus++)
        {
                printf( "scanning bus #%d\n", bus );
                for (device=0; device < 32; device++)
                {
                        for (function=0, func_max = 1; function < func_max; function++)
                        {

                        id = pcireg_cfgread( bus, device, function, 0, 4);

                        if( id != 0xffffffff )
                        {
                                int vendor, devid;

                                vendor = pcireg_cfgread( bus, device, function, 0, 2);
                                devid = pcireg_cfgread( bus, device, function, 2, 2);

                                printf( "found device at bus %d, device %d, function %d, vendor:devid %x:%x\n",
                                        bus, device, function, vendor, devid );

                                if( function == 0 )
                                {
                                        int multi;
                                        /* read whether or not it is multi-function */
                                        multi = pcireg_cfgread( bus, device, function, 14, 1);
                                        multi >>= 7;

                                        if( multi )
                                                func_max = 8;
                                }

                                /* is it the one we want? */
                                if(((vendor == 0x1180) && (devid == 0x552))
                                   || ((vendor == 0x11c1) && (devid == 0x5811))
                                   || ((vendor == 0x104c) && (devid == 0x8025))
                                   )
                                {
                                        printf( "Found OHCI controller!\n" );
                                        *rbus = bus;
                                        *rdevice = device;
                                        *rfunction = function;
                                        return 1;
                                }

                        }
                        }
                }
        }

        return 0;
}

/************************* Driver *************************/
#define OHCI_VERSION            0x00
#define OHCI_ATRETRY            0x08
#define OHCI_CROMHDR            0x18
#define OHCI_BUS_OPT            0x20
#define OHCI_BUSIRMC            (1 << 31)
#define OHCI_BUSCMC             (1 << 30)
#define OHCI_BUSISC             (1 << 29)
#define OHCI_BUSBMC             (1 << 28)
#define OHCI_BUSPMC             (1 << 27)
#define OHCI_BUSFNC             OHCI_BUSIRMC | OHCI_BUSCMC | OHCI_BUSISC |\
                                OHCI_BUSBMC | OHCI_BUSPMC
#define OHCI_EUID_HI            0x24
#define OHCI_EUID_LO            0x28

#define OHCI_CROMPTR            0x34

#define OHCI_HCCCTL             0x50
#define OHCI_HCCCTLCLR          0x54
#define OHCI_HCC_IMGVALID       (1 << 31)

#define OHCI_SID_BUF            0x64
#define OHCI_SID_CNT            0x68
#define OHCI_SID_ERR            (1 << 31)

#define OHCI_IT_MASK            0x98
#define OHCI_IT_MASKCLR         0x9c

#define OHCI_IR_MASK            0xa8
#define OHCI_IR_MASKCLR         0xac

#define OHCI_LNKCTL             0xe0
#define OHCI_LNKCTLCLR          0xe4
#define OHCI_PHYACCESS          0xec

#define OHCI_NID_STAT           0xe8

#define OHCI_AREQHI             0x100
#define OHCI_AREQLO             0x108
#define OHCI_PREQHI             0x110
#define OHCI_PREQLO             0x118


/* dunno why we need this guy... */
#define MAXREC(x)       (2 << (x))

#if 0
#define htonl(x) \
         ((((x) & 0xff000000) >> 24) | \
          (((x) & 0x00ff0000) >>  8) | \
          (((x) & 0x0000ff00) <<  8) | \
          (((x) & 0x000000ff) <<  24) )
#else
#define htonl(x) x
#endif
/* from firewire_phy.h */
#define FW_PHY_SPD_REG                  0x02
#define FW_PHY_SPD                      (3<<6)
#define FW_PHY_NP                       (15<<0)
#define FW_PHY_ESPD_REG                 0x03
#define FW_PHY_ESPD                     (7<<5)

#define FW_PHY_RHB                      (1<<7)
#define FW_PHY_IBR_REG                  0x01
#define FW_PHY_IBR                      (1<<6)

/* meh, that's what BSD does */
#define CROMSIZE 0x400
#define CROMENTS (CROMSIZE/sizeof(u32_t))

static u32_t crom[CROMENTS]  __attribute__ ((aligned (2048))) SECTION(SEC_EFI_DATA);
static u32_t crom_phys SECTION(SEC_EFI_DATA);

/* this is a bit more of a hack... */
#define SIDBUFSIZE 0x400
#define SIDBUFENTS (SIDBUFSIZE/(sizeof(u32_t)))

static u32_t sidbuf[SIDBUFENTS]   __attribute__ ((aligned (2048)))  SECTION(SEC_EFI_DATA);
static u32_t sidbuf_phys SECTION(SEC_EFI_DATA);

static int firewire_debug = 1;

/*
 * This is a bit confusing, since if we define printf() then it means
 * that printf() is off (the preprocessor removes this.
 *
 * Printf() uses this.
 */
#ifndef printf
static char *linkspeed[] =
{
        "S100", "S200", "S400", "S800",
        "S1600", "S3200", "undef", "undef"
};
#endif

static u32_t
bus_space_read_4( void *base, unsigned offset )
{
        char *base2;
        volatile unsigned int *reg;

        base2 = (char*)base;
        base2 += offset;

        reg = (volatile unsigned int *)base2;
        return *reg;
}

static void
bus_space_write_4( void *base, unsigned offset, u32_t val )
{
        char *base2;
        volatile unsigned int *reg;

        base2 = (char*)base;
        base2 += offset;

        reg = (volatile unsigned int *)base2;
        *reg = val;;
}

#define OWRITE(sc, r, x) bus_space_write_4( (sc), (r), (x) )
#define OREAD(sc, r) bus_space_read_4( (sc), (r) )

static void
DELAY(int x)
{
        int i;

        for( i = 0; i < x * 100000; i++ );
}


#if 0   /*XXX static but not used*/
/*
 * Communication with PHY device
 */
static u32_t
fwohci_set_bus_manager( ohci_t sc, unsigned int node)
{
        int i;
        u32_t bm;

#define OHCI_CSR_DATA   0x0c
#define OHCI_CSR_COMP   0x10
#define OHCI_CSR_CONT   0x14
#define OHCI_BUS_MANAGER_ID     0

        OWRITE(sc, OHCI_CSR_DATA, node);
        OWRITE(sc, OHCI_CSR_COMP, 0x3f);
        OWRITE(sc, OHCI_CSR_CONT, OHCI_BUS_MANAGER_ID);
        for (i = 0; !(OREAD(sc, OHCI_CSR_CONT) & (1<<31)) && (i < 1000); i++)
                DELAY(10);
        bm = OREAD(sc, OHCI_CSR_DATA);
        if((bm & 0x3f) == 0x3f)
                bm = node;
        if (firewire_debug)
                printf("fw_set_bus_manager: %d->%d (loop=%d)\n", bm, node, i);

        return(bm);
}
#endif

static u32_t
fwphy_rddata( ohci_t sc,  unsigned int addr)
{
        u32_t fun, stat;
        unsigned int i, retry = 0;

        addr &= 0xf;
#define MAX_RETRY 100
again:
        OWRITE(sc, FWOHCI_INTSTATCLR, OHCI_INT_REG_FAIL);
        fun = PHYDEV_RDCMD | (addr << PHYDEV_REGADDR);
        OWRITE(sc, OHCI_PHYACCESS, fun);
        for ( i = 0 ; i < MAX_RETRY ; i ++ ){
                fun = OREAD(sc, OHCI_PHYACCESS);
                if ((fun & PHYDEV_RDCMD) == 0 && (fun & PHYDEV_RDDONE) != 0)
                        break;
                DELAY(100);
        }
        if(i >= MAX_RETRY) {
            if (firewire_debug) {
                    printf("phy read failed(1).\n");
            }
            if (++retry < MAX_RETRY) {
                DELAY(100);
                goto again;
            }
        }
        /* Make sure that SCLK is started */
        stat = OREAD(sc, FWOHCI_INTSTAT);
        if ((stat & OHCI_INT_REG_FAIL) != 0 ||
                        ((fun >> PHYDEV_REGADDR) & 0xf) != addr) {
            if (firewire_debug) {
                        printf("phy read failed(2).\n");
            }
            if (++retry < MAX_RETRY) {
                DELAY(100);
                goto again;
            }
        }
        if (firewire_debug || retry >= MAX_RETRY) {
                printf("fwphy_rddata: 0x%x loop=%d, retry=%d\n", addr, i, retry);
        }
#undef MAX_RETRY
        return((fun >> PHYDEV_RDDATA )& 0xff);
}

static u32_t
fwphy_wrdata( ohci_t sc, u32_t addr, u32_t data)
{
        u32_t fun;

        addr &= 0xf;
        data &= 0xff;

        fun = (PHYDEV_WRCMD | (addr << PHYDEV_REGADDR) | (data << PHYDEV_WRDATA));
        OWRITE(sc, OHCI_PHYACCESS, fun);
        DELAY(100);

        return(fwphy_rddata( sc, addr));
}

/*
 * CRC16 check-sum for IEEE1394 register blocks.
 */
static u16_t
fw_crc16(u32_t *ptr, u32_t len)
{
        u32_t i, sum, crc = 0;
        int shift;
        len = (len + 3) & ~3;
        for(i = 0 ; i < len ; i+= 4){
                for( shift = 28 ; shift >= 0 ; shift -= 4){
                        sum = ((crc >> 12) ^ (ptr[i/4] >> shift)) & 0xf;
                        crc = (crc << 4) ^ ( sum << 12 ) ^ ( sum << 5) ^ sum;
                }
                crc &= 0xffff;
        }
        return((u16_t) crc);
}

static void
fill_crom( ohci_t sc )
{
        unsigned int i;

        /* zero the data */
        for( i = 0; i < CROMENTS; i++ )
                crom[i] = 0;

        /* make the phys addr */
        //crom_phys = (u32_t)crom;
        crom_phys = (u32_t) virt_to_phys( crom );

        crom[1] = 0x31333934;
        crom[2] = 0xf000a002;
        crom[3] = OREAD(sc, OHCI_EUID_HI);
        crom[4] = OREAD(sc, OHCI_EUID_LO);
        crom[5] = 0;
        crom[0] = (4 << 24) | (5 << 16);

        crom[0] |= fw_crc16( &crom[1], 5*4 );

        printf( "fill_crom: EUID_HI: %lx\n", crom[3] );
        printf( "fill_crom: EUDI_LO: %lx\n", crom[4] );
        printf( "fill_crom: crom_phys is %lx\n", crom_phys );

        /* fill in the magic about ourselves... */
        crom[6] = 0;  /* empty directory length (and hopefully invalid CRC) */
        crom[7] = 0x4742444b;  /* 'KDBG' kernel debug protocol */
        crom[8] = 0;  /* assume we're in the bottom 32 bits */
        //crom[9] = (u32_t)&serial_buf;
        crom[9] = (u32_t) virt_to_phys( (void*) &serial_buf );

        printf( "fill_crom: serial_buf is %lx\n", crom[9] );

        /* zero out the sidbuf */
        for( i = 0; i < SIDBUFENTS; i++ )
                sidbuf[i] = 0;

        /* make the phys addr */
        sidbuf_phys = (u32_t) virt_to_phys( sidbuf );
        //sidbuf_phys = (u32_t) sidbuf;

        printf( "fill_crom: sidbuf_phys is %lx\n", sidbuf_phys );

        // enter_kdebug("filled crom");
}

static int
fwohci_probe_phy( ohci_t sc )
{
        u32_t reg, reg2;
        int e1394a = 1;
        int nport = 0, speed;
/*
 * probe PHY parameters
 * 0. to prove PHY version, whether compliance of 1394a.
 * 1. to probe maximum speed supported by the PHY and
 *    number of port supported by core-logic.
 *    It is not actually available port on your PC .
 */
        OWRITE(sc, OHCI_HCCCTL, OHCI_HCC_LPS);
        reg = fwphy_rddata(sc, FW_PHY_SPD_REG);

        if((reg >> 5) != 7 )
        {
                nport = reg & FW_PHY_NP;
                speed = reg & FW_PHY_SPD >> 6;
                printf( "Phy 1394 only %s, %d ports.\n",
                        linkspeed[speed], nport );
        }
        else
        {
                reg2 = fwphy_rddata(sc, FW_PHY_ESPD_REG);
                nport = reg & FW_PHY_NP;
                speed = (reg2 & FW_PHY_ESPD) >> 5;
                printf( "Phy 1394a available %s, %d ports.\n",
                        linkspeed[speed], nport );

                /* check programPhyEnable */
                reg2 = fwphy_rddata(sc, 5);
#if 0
                if (e1394a && (OREAD(sc, OHCI_HCCCTL) & OHCI_HCC_PRPHY)) {
#else   /* XXX force to enable 1394a */
                if (e1394a) {
#endif
                    if (firewire_debug) {
                        printf( "Enable 1394a Enhancements\n" );
                    }
                    /* enable EAA EMC */
                    reg2 |= 0x03;
                    /* set aPhyEnhanceEnable */
                    OWRITE(sc, OHCI_HCCCTL, OHCI_HCC_PHYEN);
                    OWRITE(sc, OHCI_HCCCTLCLR, OHCI_HCC_PRPHY);
                } else {
                    /* for safe */
                    reg2 &= ~0x83;
                }
                reg2 = fwphy_wrdata(sc, 5, reg2);
        }

        /* FIXME: WTF does this do?  */
        reg = fwphy_rddata(sc, FW_PHY_SPD_REG);
        if((reg >> 5) == 7 )
        {
                reg = fwphy_rddata(sc, 4);
                reg |= 1 << 6;
                (void) fwphy_wrdata(sc, 4, reg);
                reg = fwphy_rddata(sc, 4);
        }

        /* do some more magic, as per the linux source */
#if 0
        printf( "Doing magic port enables\n" );
        for( i = 0; i < nport; i++ )
        {
                u32_t status;

                printf( "enable %d... \n", i );

                fwphy_wrdata( sc, 7, i );
                status = fwphy_rddata( sc, 8 );

                if( status & 0x20 )
                        fwphy_wrdata( sc, 8, status );
        }
#endif

        return 0;
}


static void
fwohci_reset( ohci_t sc )
{
        int i, max_rec, speed;
        u32_t reg, reg2;

        /* Disable interrupts */
        OWRITE(sc, FWOHCI_INTMASKCLR, ~0);

#if 0
        /* Now stopping all DMA channels */
        OWRITE(sc,  OHCI_ARQCTLCLR, OHCI_CNTL_DMA_RUN);
        OWRITE(sc,  OHCI_ARSCTLCLR, OHCI_CNTL_DMA_RUN);
        OWRITE(sc,  OHCI_ATQCTLCLR, OHCI_CNTL_DMA_RUN);
        OWRITE(sc,  OHCI_ATSCTLCLR, OHCI_CNTL_DMA_RUN);

        OWRITE(sc,  OHCI_IR_MASKCLR, ~0);
        for( i = 0 ; i < sc->fc.nisodma ; i ++ ){
                OWRITE(sc,  OHCI_IRCTLCLR(i), OHCI_CNTL_DMA_RUN);
                OWRITE(sc,  OHCI_ITCTLCLR(i), OHCI_CNTL_DMA_RUN);
        }
#endif

        /* FLUSH FIFO and reset Transmitter/Reciever */
        OWRITE(sc, OHCI_HCCCTL, OHCI_HCC_RESET);
        if (firewire_debug) {
            printf("resetting OHCI...");
        }
        i = 0;
        while(OREAD(sc, OHCI_HCCCTL) & OHCI_HCC_RESET) {
                if (i++ > 100) break;
                DELAY(1000);
        }
        if (firewire_debug) {
            printf("done (loop=%d)\n", i);
        }
        /* Probe phy */
        (void) fwohci_probe_phy( sc );

        /* Probe link */
        reg = OREAD(sc,  OHCI_BUS_OPT);
        reg2 = reg | OHCI_BUSFNC;
        max_rec = (reg & 0x0000f000) >> 12;
        speed = (reg & 0x00000007);
        printf( "Link %s, max_rec %d bytes.\n",
                linkspeed[speed], MAXREC(max_rec) );

#if 1
        printf( "skipping fixing max_rec (%d)\n", max_rec );
#else
        /* XXX fix max_rec */
        sc->fc.maxrec = sc->fc.speed + 8;
        if (max_rec != sc->fc.maxrec) {
                reg2 = (reg2 & 0xffff0fff) | (sc->fc.maxrec << 12);
                device_printf(dev, "max_rec %d -> %d\n",
                                MAXREC(max_rec), MAXREC(sc->fc.maxrec));
        }
        if (firewire_debug) {
            device_printf(dev, "BUS_OPT 0x%x -> 0x%x\n", reg, reg2);
        }
#endif
        OWRITE(sc,  OHCI_BUS_OPT, reg2);

        /* Initialize registers */
        /* XXX: htonl on crom[0]? BSD doesn't... */
        OWRITE(sc, OHCI_CROMHDR, crom[0]);
        OWRITE(sc, OHCI_CROMPTR, crom_phys);
        OWRITE(sc, OHCI_HCCCTLCLR, OHCI_HCC_BIGEND); /* CLR = byteswap */
        OWRITE(sc, OHCI_HCCCTL, OHCI_HCC_IMGVALID);
        OWRITE(sc, OHCI_HCCCTL, OHCI_HCC_POSTWR);
        OWRITE(sc, OHCI_SID_BUF, sidbuf_phys);
        OWRITE(sc, OHCI_LNKCTL, OHCI_CNTL_SID);

        /* Enable link */
        OWRITE(sc, OHCI_HCCCTL, OHCI_HCC_LINKEN);

#if 0
        /* Force to start async RX DMA */
        sc->arrq.xferq.flag &= ~FWXFERQ_RUNNING;
        sc->arrs.xferq.flag &= ~FWXFERQ_RUNNING;
        fwohci_rx_enable(sc, &sc->arrq);
        fwohci_rx_enable(sc, &sc->arrs);

        /* Initialize async TX */
        OWRITE(sc, OHCI_ATQCTLCLR, OHCI_CNTL_DMA_RUN | OHCI_CNTL_DMA_DEAD);
        OWRITE(sc, OHCI_ATSCTLCLR, OHCI_CNTL_DMA_RUN | OHCI_CNTL_DMA_DEAD);
#endif

        /* AT Retries */
        OWRITE(sc, FWOHCI_RETRY,
                /* CycleLimit   PhyRespRetries ATRespRetries ATReqRetries */
                (0xffff << 16 ) | (0x0f << 8) | (0x0f << 4) | 0x0f) ;

#if 0
        sc->atrq.top = STAILQ_FIRST(&sc->atrq.db_trq);
        sc->atrs.top = STAILQ_FIRST(&sc->atrs.db_trq);
        sc->atrq.bottom = sc->atrq.top;
        sc->atrs.bottom = sc->atrs.top;

        for( i = 0, db_tr = sc->atrq.top; i < sc->atrq.ndb ;
                                i ++, db_tr = STAILQ_NEXT(db_tr, link)){
                db_tr->xfer = NULL;
        }
        for( i = 0, db_tr = sc->atrs.top; i < sc->atrs.ndb ;
                                i ++, db_tr = STAILQ_NEXT(db_tr, link)){
                db_tr->xfer = NULL;
        }


        /* Enable interrupts */
        OWRITE(sc, FWOHCI_INTMASK,
                        OHCI_INT_ERR  | OHCI_INT_PHY_SID
                        | OHCI_INT_DMA_ATRQ | OHCI_INT_DMA_ATRS
                        | OHCI_INT_DMA_PRRQ | OHCI_INT_DMA_PRRS
                        | OHCI_INT_PHY_BUS_R | OHCI_INT_PW_ERR);
        fwohci_set_intr(&sc->fc, 1);
#else
        /* FWIW, enable interrupts */
        OWRITE(sc, FWOHCI_INTMASK,
                        OHCI_INT_ERR  | OHCI_INT_PHY_SID
                        | OHCI_INT_DMA_ATRQ | OHCI_INT_DMA_ATRS
                        | OHCI_INT_DMA_PRRQ | OHCI_INT_DMA_PRRS
                        | OHCI_INT_PHY_BUS_R | OHCI_INT_PW_ERR);

        OWRITE(sc, FWOHCI_INTMASK, OHCI_INT_EN);
#endif

}


/* initiate a bus reset - make the wire do something */
static void
fwohci_ibr( ohci_t sc )
{
        u32_t fun;

        printf( "Initiate bus reset\n" );

        /*
         * Make sure our cached values from the config rom are
         * initialised.
         */
        /* XXX: checkme: byte order! of crom[0] and [2] */
        OWRITE(sc, OHCI_CROMHDR, htonl(crom[0]));
        OWRITE(sc, OHCI_BUS_OPT, htonl(crom[2]));

        /*
         * Set root hold-off bit so that non cyclemaster capable node
         * shouldn't became the root node.
         */
#if 1
        fun = fwphy_rddata(sc, FW_PHY_IBR_REG);
        // fun |= FW_PHY_IBR | FW_PHY_RHB;
        fun |= FW_PHY_IBR; /* don't be root */
        fun = fwphy_wrdata(sc, FW_PHY_IBR_REG, fun);
#else   /* Short bus reset */
        fun = fwphy_rddata(sc, FW_PHY_ISBR_REG);
        fun |= FW_PHY_ISBR | FW_PHY_RHB;
        fun = fwphy_wrdata(sc, FW_PHY_ISBR_REG, fun);
#endif
}

static int
fwohci_init( ohci_t sc )
{
        int i, mver;
        u32_t reg;


/* OHCI version */

        reg = OREAD(sc, OHCI_VERSION);

        mver = (reg >> 16) & 0xff;
        printf("OHCI version %x.%x (ROM=%d)\n",
               mver, reg & 0xff, (reg>>24) & 1);
        if (mver < 1 || mver > 9)
        {
                printf( "invalid OHCI version\n");
                return -1;
        }

/* Available Isochronous DMA channel probe */
        OWRITE(sc, OHCI_IT_MASK, 0xffffffff);
        OWRITE(sc, OHCI_IR_MASK, 0xffffffff);
        reg = OREAD(sc, OHCI_IT_MASK) & OREAD(sc, OHCI_IR_MASK);
        OWRITE(sc, OHCI_IT_MASKCLR, 0xffffffff);
        OWRITE(sc, OHCI_IR_MASKCLR, 0xffffffff);
        for (i = 0; i < 0x20; i++)
                if ((reg & (1 << i)) == 0)
                        break;
        printf("No. of Isochronous channels is %d.\n", i);
        if (i == 0)
                return -1;

        /* create a crom */
        fill_crom( sc );

        // enter_kdebug("filled crom" );

        /* (re)set the hardware */
        fwohci_reset( sc );

        return 0;
}


static void
dump_int_status( ohci_t sc )
{
        u32_t stat;
        stat = OREAD( sc, FWOHCI_INTSTAT );

        printf( "stat: %lx\n", stat );

        printf( "INTERRUPT < %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s> 0x%08x, 0x%08x\n",
                stat & OHCI_INT_EN ? "DMA_EN ":"",
                stat & OHCI_INT_PHY_REG ? "PHY_REG ":"",
                stat & OHCI_INT_CYC_LONG ? "CYC_LONG ":"",
                stat & OHCI_INT_ERR ? "INT_ERR ":"",
                stat & OHCI_INT_CYC_ERR ? "CYC_ERR ":"",
                stat & OHCI_INT_CYC_LOST ? "CYC_LOST ":"",
                stat & OHCI_INT_CYC_64SECOND ? "CYC_64SECOND ":"",
                stat & OHCI_INT_CYC_START ? "CYC_START ":"",
                stat & OHCI_INT_PHY_INT ? "PHY_INT ":"",
                stat & OHCI_INT_PHY_BUS_R ? "BUS_RESET ":"",
                stat & OHCI_INT_PHY_SID ? "SID ":"",
                stat & OHCI_INT_LR_ERR ? "DMA_LR_ERR ":"",
                stat & OHCI_INT_PW_ERR ? "DMA_PW_ERR ":"",
                stat & OHCI_INT_DMA_IR ? "DMA_IR ":"",
                stat & OHCI_INT_DMA_IT  ? "DMA_IT " :"",
                stat & OHCI_INT_DMA_PRRS  ? "DMA_PRRS " :"",
                stat & OHCI_INT_DMA_PRRQ  ? "DMA_PRRQ " :"",
                stat & OHCI_INT_DMA_ARRS  ? "DMA_ARRS " :"",
                stat & OHCI_INT_DMA_ARRQ  ? "DMA_ARRQ " :"",
                stat & OHCI_INT_DMA_ATRS  ? "DMA_ATRS " :"",
                stat & OHCI_INT_DMA_ATRQ  ? "DMA_ATRQ " :"",
                stat, OREAD(sc, FWOHCI_INTMASK)
                );

}

static void
dump_nid_status( ohci_t sc )
{
        u32_t t;
        t = OREAD(sc, OHCI_NID_STAT );
        printf( "read nid & status 0x%lx\n", t );
}

static void
dump_sid_status( ohci_t sc )
{
        u32_t t;
        t = OREAD(sc, OHCI_SID_CNT );
        printf( "sid status 0x%lx (%s)\n", t, t & OHCI_SID_ERR ? "err" : "ok");
}


static void
check_bus_reset(void)
{
        /* work out the sc */
        ohci_t sc;

        /* XXX: is there a better way to do this? */
        if( fwaddr_virt != 0 )
                sc = (ohci_t) fwaddr_virt;
        else
                sc = (ohci_t) fwaddr_phys;


        /* check for bus reset */
        u32_t stat;
        stat = OREAD( sc, FWOHCI_INTSTAT );

        if( stat & OHCI_INT_PHY_BUS_R )
                handle_bus_reset( sc );
}

static void
handle_bus_reset( ohci_t sc )
{

        OWRITE(sc, FWOHCI_INTMASKCLR,  OHCI_INT_PHY_BUS_R);

        printf( "BUS reset\n" );
        OWRITE(sc, FWOHCI_INTMASKCLR,  OHCI_INT_CYC_LOST);
        OWRITE(sc, OHCI_LNKCTLCLR, OHCI_CNTL_CYCSRC);

        /* clear some other bits */
        OWRITE(sc, FWOHCI_INTSTATCLR, OHCI_INT_PHY_BUS_R);
        OWRITE(sc, FWOHCI_INTSTATCLR, OHCI_INT_PHY_SID);

        /* now post-SID stuff */
        {
                u32_t node_id; /* *buf,  */
                int plen;

                OWRITE(sc, OHCI_ATRETRY, 1<<(13+16) | 0xfff);
                node_id = OREAD(sc, FWOHCI_NODEID);
                plen = OREAD(sc, OHCI_SID_CNT);

                printf( "node_id=0x%08x, gen=%d, ",
                        node_id, (plen >> 16) & 0xff );

                if (!(node_id & OHCI_NODE_VALID))
                {
                        printf("Bus reset failure\n");
                        return;
                }

                if (node_id & OHCI_NODE_ROOT)
                {
                        printf("CYCLEMASTER mode\n");
                        OWRITE(sc, OHCI_LNKCTL,
                                OHCI_CNTL_CYCMTR | OHCI_CNTL_CYCTIMER);

                        /* If we're root, we didn't ask for it, so we should
                         * be the only guy on the bus.
                         * If we're the only one, drop output.
                         */
#if 0
                        stand_alone = 1;
#endif
                }
                else
                {
                        printf("non CYCLEMASTER mode\n");
                        OWRITE(sc, OHCI_LNKCTLCLR, OHCI_CNTL_CYCMTR);
                        OWRITE(sc, OHCI_LNKCTL, OHCI_CNTL_CYCTIMER);

                        /* someone's (maybe) listening */
                        stand_alone = 0;
                }
                // fc->nodeid = node_id & 0x3f;

                if (plen & OHCI_SID_ERR)
                {
                        printf( "SID Error\n" );
                        return;
                }


        }

        /* enable stuff */
        {

                u32_t t;
                t = ~0U;

                OWRITE( sc, OHCI_AREQHI, t );
                OWRITE( sc, OHCI_AREQLO, t );
                OWRITE( sc, OHCI_PREQHI, t );
                OWRITE( sc, OHCI_PREQLO, t );

                printf( "enabled AREQ and PREQ (0x%lx)\n", t );
        }


}

static int
start_driver( int bus, int device, int function )
{
        /* Read BAR0 to find the memory guy */
        unsigned int bar0_val;
        unsigned int bar0_size;
        addr_t paddr, vaddr;
        ohci_t sc;

        bar0_val = pcireg_cfgread( bus, device, function, 16, 4);
        pcireg_cfgwrite( bus, device, function, 16, 4, -1);
        bar0_size = pcireg_cfgread( bus, device, function, 16, 4);
        pcireg_cfgwrite( bus, device, function, 16, 4, bar0_val);

        printf( "bar0_val: 0x%x\n", bar0_val );
        printf( "bar0_size: 0x%x\n", bar0_size );

        /* map it */
        paddr = (void*) (bar0_val & ~(0xFFFUL));
        /*
         * XXX WTF??!
         *
         * Chuck had code like this?!
         *
         * addr_t vaddr;
         * // runs some code without initializing vaddr
         *
         * add_init_root_mapping( vaddr );
         * vaddr = paddr;
         */
        vaddr = paddr;
        add_init_root_mapping( vaddr );
        fwaddr_phys = (word_t)vaddr;
        fwaddr_virt = (word_t)vaddr;
        printf( "vaddr is 0x%lx\n", vaddr );

        /* pointer to the OHCI structure */
        sc = (ohci_t) vaddr;

        /* now call the BSD init function */
        (void) fwohci_init( sc );

        /* and do a bus reset */
        fwohci_ibr( sc );

        /* probe the bus reset */
        {
                int i;
                u32_t stat;

                for( i = 0; i < 3; i++ )
                {
                        stat = OREAD( sc, FWOHCI_INTSTAT );

                        if( stat & OHCI_INT_PHY_BUS_R )
                                break;

                        DELAY(100);
                }

                if( (stat & OHCI_INT_PHY_BUS_R) == 0 )
                {
                        printf( "bus didn't reset??\n" );
                        return -1;
                }
        }

        handle_bus_reset( sc );

        {
#if 0
                volatile unsigned int *p = NULL;
                kernel_space->add_mapping((void*) 0, 0,
                                          pgent_t::size_4k, true, true, true);

                printf( "reading from %p\n", p );
                printf( "read 0x%lx\n", *p );
#endif
                // while(1)
                {

                        /* dump status */
                        dump_int_status( sc );
                        dump_nid_status( sc );
                        dump_sid_status( sc );

                        // enter_kdebug("pausing...");
                }
        }

        putc_dbg1394( 'L' );
        putc_dbg1394( '4' );
        putc_dbg1394( ' ' );
        putc_dbg1394( '1' );
        putc_dbg1394( '3' );
        putc_dbg1394( '9' );
        putc_dbg1394( '4' );
        putc_dbg1394( ' ' );
        putc_dbg1394( 'S' );
        putc_dbg1394( 't' );
        putc_dbg1394( 'a' );
        putc_dbg1394( 'r' );
        putc_dbg1394( 't' );
        putc_dbg1394( 'i' );
        putc_dbg1394( 'n' );
        putc_dbg1394( 'g' );
        putc_dbg1394( '\n' );

        return 0;
}

/************************* Glue *************************/
void
init_dbg1394(void)
{
    int r, bus, device, function;

    printf( "doing 1394 init...\n" );

    printf( "scanning busses for stuff...\n" );
    r = bus_scan( &bus, &device, &function );

    if( r )
        (void) start_driver( bus, device, function );
    else
        printf( "didn't find an OHCI controller :(\n" );
}

/* xxx */
static addr_t ich7_paddr = 0, ich7_vaddr = 0;

#undef printf
unsigned do_magic_read(int off, int size)  SECTION (SEC_EFI_IO);
unsigned
do_magic_read(int off, int size)
{
        u32_t *r4;
        u16_t *r2;
        u8_t  *r1;
        unsigned val;

        if( ich7_vaddr == 0 )
        {
                ich7_paddr = (addr_t) pcireg_cfgread( 0, 0x1f, 0, 0xf0, 4 );
                ich7_vaddr = (addr_t) (((unsigned)ich7_paddr) & (~0x3UL));
                printf("ich7_paddr = %p, ich7_vaddr = %p\n",
                    ich7_paddr, ich7_vaddr);
                add_init_root_mapping( ich7_vaddr );
        }

        switch(size)
        {
                case 4:
                        r4 = (u32_t*) ich7_vaddr;
                        val = r4[off>>2];
                        break;
                case 2:
                        r2 = (u16_t*) ich7_vaddr;
                        val = r2[off>>1];
                        break;

                case 1:
                        r1 = (u8_t*) ich7_vaddr;
                        val = r1[off];
                        break;
                default:
                        val = (unsigned) -1;
        }

        return val;
}

#if !defined(_lint)
#define printf(x...)
#endif

extern void do_magic_write(int off, int size, int val);
void
do_magic_write(int off, int size, int val)
{
        u32_t *r4;
        u16_t *r2;
        u8_t  *r1;

        if( ich7_vaddr == 0 )
        {
                /* I'm the worst person in the world - CEG */
                ich7_paddr = (addr_t) pcireg_cfgread( 0, 0x1f, 0, 0xf0, 4 );
                ich7_vaddr = (addr_t) (((unsigned)ich7_paddr) & (~0x3UL));
                add_init_root_mapping( ich7_vaddr );
        }

        switch(size)
        {
                case 4:
                        r4 = (u32_t*) ich7_vaddr;
                        r4[off>>2] = val;
                        break;
                case 2:
                        r2 = (u16_t*) ich7_vaddr;
                        r2[off>>1] = val;
                        break;

                case 1:
                        r1 = (u8_t*) ich7_vaddr;
                        r1[off] = val;;
                        break;
                default:
                        /* na und? */
                        break;
        }
}

unsigned pci_read_byte(int bus, int dev, int fn, int off)  SECTION (SEC_EFI_IO);

unsigned
pci_read_byte(int bus, int dev, int fn, int off)
{
        return pcireg_cfgread(bus, dev, fn, off, 1);
}
