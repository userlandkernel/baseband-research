/*
 * Copyright (c) 2003-2006, National ICT Australia (NICTA)
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
 * Description: Generic L4 init code
 */


/* Code for handling Mach-O binary format */

#include <macros.h>
#include <types.h>
#include <config.h>
#include <debug.h>
#include <generic/lib.h>
#include <arch/config.h>
#include <generic/memregion.h>
#include <arch/memory.h>

/* XXX: we compile with -fno-leading-underscore */
#define _mh_execute_header __mh_execute_header

/* macho header data structure symbol */
extern struct mach_header _mh_execute_header;

/* header type */
struct mach_header {
        unsigned long   magic;
        int             cputype;
        int             cpusubtype;
        unsigned long   filetype;
        unsigned long   ncmds;
        unsigned long   sizeofcmds;
        unsigned long   flags;
};

#define LC_SEGMENT              0x1

struct macho_segcmd {
        unsigned long   cmd;
        unsigned long   cmdsize;
        char            segname[16];
        unsigned long   vmaddr;
        unsigned long   vmsize;
        unsigned long   fileoff;
        unsigned long   filesize;
        int/*vm_prot_t*/        maxprot;
        int/*vm_prot_t*/        initprot;
        unsigned long   nsects;
        unsigned long   flags;
};

struct macho_section {
        char            sectname[16];
        char            segname[16];
        unsigned long   addr;
        unsigned long   size;
        unsigned long   offset;
        unsigned long   align;
        unsigned long   reloff;
        unsigned long   nreloc;
        unsigned long   flags;
        unsigned long   reserved1;
        unsigned long   reserved2;
};

/* XXX: x86 doesn't have a strcmp, and there's no standard way
 * to do it.
 */
static int strcmp( const char *s1, const char *s2 )
{
        while( 1 ) {
                if( !*s1 && !*s2 )
                        return 0;
                if( (!*s1 && *s2) || (*s1 < *s2) )
                        return -1;
                if( (*s1 && !*s2) || (*s1 > *s2) )
                        return 1;
                s1++;
                s2++;
        }
}


struct macho_segcmd *
findsegnamed(const char *s)
{
        int                     i = 0, ncmds;
        struct mach_header      *mh;
        struct macho_segcmd     *segcmd = NULL;

        mh = &_mh_execute_header;
        segcmd = (struct macho_segcmd *)(mh + 1);

        printf( "***** finding segment named '%s'\n", s );

        ncmds = mh->ncmds;

        for (i = 0; i < ncmds; i++)
          {
                if (strcmp(segcmd->segname, s) == 0)
                        break;
                /*
                 * next one
                 */
                segcmd = (struct macho_segcmd *)((char *)segcmd +
                                                 segcmd->cmdsize);
        }

        return (segcmd);
}

struct macho_section *
findsectnamed(const char *seg, const char *s)
{
        int                     i = 0, ncmds;
        struct mach_header      *mh;
        unsigned long           j;
        struct  macho_segcmd    *segcmd = NULL;
        struct  macho_section   *sect = NULL;

        mh = &_mh_execute_header;
        segcmd = (struct macho_segcmd *)(mh + 1);

        ncmds = mh->ncmds;

        for (i = 0; i < ncmds; i++) {
                if (segcmd->cmd != LC_SEGMENT)
                        continue;

                if (seg && strcmp(seg, segcmd->segname))
                        goto next;

                sect = (struct macho_section *)((char *)segcmd +
                     sizeof(*segcmd));
                for (j = 0; j < segcmd->nsects; j++) {
                        if (strcmp(sect->sectname, s) == 0)
                                goto found;

                        sect++;
                }

                /*
                 * next one
                 */
next:
                segcmd = (struct macho_segcmd *)((char *)segcmd +
                    segcmd->cmdsize);

        }

found:
        return (sect);
}


/* macho BSS clearing function */
void
clear_bss(void)
{
        struct macho_section    *sect;
        char *bss_start;
        char *bss_end;

        /*
         * Get bss symbols
         */
        sect = findsectnamed(NULL, "__bss");
        bss_start = (char *)sect->addr;
        bss_end = (char *)sect->addr + sect->size;
        memset(bss_start, 0, bss_end - bss_start);

        /*
         * zero the common here, might as well.
         */
        sect = findsectnamed(NULL, "__common");
        if( sect != NULL )
                memset((void *)sect->addr, 0, sect->size);
}


/*** apparently this stuff is being phased out... ***/

/*
 * findbinarysiz
 *
 * Add up anything that is not __INIT and return the size.
 */
unsigned long
findbinarysize(void)
{
        int                     i = 0, ncmds;
        unsigned long           siz = 0;
        struct mach_header      *mh;
        struct  macho_segcmd    *segcmd = NULL;

        mh = &_mh_execute_header;
        segcmd = (struct macho_segcmd *)(mh + 1);

        ncmds = mh->ncmds;

        for( i = 0; i < ncmds; i++ )
        {
                if (segcmd->cmd != LC_SEGMENT)
                        goto next;      /* not loading */
                if (strcmp(segcmd->segname, "__PAGEZERO") == 0)
                        goto next;      /* Skip page zero */
                if (strcmp(segcmd->segname, "__INIT") == 0)
                        goto next;      /* Skip initialization */
                siz += segcmd->vmsize;

next:
                segcmd = (struct macho_segcmd *)((char *)segcmd +
                    segcmd->cmdsize);
        }

        return (siz);
}


/* Kernel layout vars */
char *_start_bootmem_phys = NULL;
char *_end_bootmem_phys = NULL;

/* kernel code and data */
char *_start_text_phys = NULL;
char *_end_text_phys = NULL;
char *_start_text = NULL;
char *_end_text = NULL;
char *_start_cpu_local = NULL;
char *_end_cpu_local = NULL;

/* init section */
char *_start_init = NULL;
char *_end_init = NULL;


/* macho pulls out locations of things at runtime */
void
init_macho_vars(void)
{
        unsigned long   binsiz;
        struct macho_segcmd     *segcmd;

        /* setup phys bootmem vars */
        _start_bootmem_phys = _start_bootmem - KERNEL_RW_OFFSET;
        _end_bootmem_phys = _end_bootmem - KERNEL_RW_OFFSET;

        printf("bootmem      %p - %p\n", _start_bootmem, _end_bootmem );
        printf("bootmem_phys %p - %p\n", _start_bootmem_phys,
               _end_bootmem_phys );

        binsiz = findbinarysize();
        printf("binsiz = %d\n", binsiz);
        _start_text = (char *)findsegnamed("__TEXT")->vmaddr;
        _end_text = _start_text + binsiz;
        _start_text_phys = _start_text - KERNEL_RW_OFFSET;
        _end_text_phys = _end_text - KERNEL_RW_OFFSET;
        printf("mach-o: kernel %p - %p\n", _start_text, _end_text);

        segcmd = findsegnamed("__INIT");
        _start_init = (char *)segcmd->vmaddr;
        _end_init = (char *)(segcmd->vmaddr + segcmd->vmsize);
        printf("mach-o: %s %p - %p\n",
            segcmd->segname,
            _start_init, _end_init);

        printf("mem0 should be %lx - %lx\n", _start_text_phys,
               _end_bootmem_phys );

}


