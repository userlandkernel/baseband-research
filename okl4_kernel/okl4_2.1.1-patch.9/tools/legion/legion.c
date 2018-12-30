/* Legion
 *
 * Geoffrey Lee < glee at cse unsw edu au >
 *
 * TODO 
 *
 * Accept fat binaries
 * Make 64-bit clean
 * Dump LC_SYMTAB for real mach_kernel for __PRELINK
 *
 * XXX there is still the issue of what happens to the kernel cache   XXX
 * XXX as that only dumps the real mach kernel not the whole whizbang XXX
 * XXX assumes kernel, kip, binfo ..  etc all sit at bottom 4GB       XXX
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "mach-o.h"

#define VERSION "0.1"
#define AUTHOR "Geoffrey Lee < glee at cse unsw edu au >"

#define MOD_PREALLOC    30

#define M_KERN      0
#define M_SEXEC     1
#define M_RAW       2   

struct arch {
    char        *name;      /* name of architecture */
    uint32_t    pagesiz;    /* pagesize */
};

struct mod {
    char        *name;  /* filename */
    int     type;   /* type of module */
    uint32_t    addr;   /* start load address */
    uint32_t    ip; /* program counter (if applicable) */
    uint32_t    siz;    /* size in memory, not rounded to page */
    uint32_t    filesiz;    /* size on disk */
};

#if 0
static struct arch archtab[] = {
    { "i386", 0x1000 },
    { "x86_64", 0x1000 },
    { "ppc", 0x1000 },
    { "ppc64", 0x1000 }
};
#endif

/*
 * Just in case that you are not using gcc.
 */
#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN    /*NOTHING*/
#endif

static void NORETURN fatal(const char *, ...);
static void version(void);
static void usage(void);
static void macho_dumplegion(const char *);
static void macho_buildlegion(struct mod *, int, const char *);
static uint32_t macho_read32(const void *, uint32_t);
static uint64_t macho_read64(const void *, uint64_t);
static void macho_write32(const void *,  uint32_t *, uint32_t);
static void macho_write64(const void *, uint64_t *, uint64_t);
static uint32_t macho_swap32(uint32_t);
static uint64_t macho_swap64(uint64_t);
static int macho_checkfile(struct mach_header *mh);
static int macho_writefile(int, const void *, size_t);
static void macho_getheader(const char *, struct mach_header **);
static void macho_appendheader(struct mach_header *, struct mod *, int, 
                uint32_t *, off_t *, off_t, char **, int);
static void macho_appendmodule(struct mach_header *, struct mod *, int, 
                off_t *, int);
static void macho_appendraw(struct mod *, int, off_t *, int);
static void macho_binfo(struct mod *, int, off_t *, off_t, 
            uint32_t *, int, uint32_t);
static void macho_patchkip(int, uint32_t, uint32_t, uint32_t, 
             struct mod *, int);
static void macho_countsegments(struct mod *, int, uint32_t *, uint32_t *);


#define L4_BOOTINFO_MAGIC   (0x14b0021d)
#define L4_BOOTINFO_VERSION 1

#define L4_MODULE       0x0001
#define L4_SIMPLEEXEC       0x0002

#define L4_BOOTMEMTYPE      0xe
#define L4_INITIALMAPPING   0xc

/*
 * HACKS, BITFIELD  is little-endian only for now
 */
typedef uint64_t                        L4_Word64_t;
typedef uint32_t                        L4_Word32_t;
    
#define L4_BITFIELD2(t,a,b)             t a; t b
#define L4_BITFIELD5(t,a,b,c,d,e)       t a; t b; t c; t d; t e
#define L4_BITS_PER_WORD_32             (sizeof (L4_Word32_t) * 8)
#define L4_BITS_PER_WORD_64             (sizeof (L4_Word64_t) * 8)

/*
 * Duplicating code sucks, but we need to support both 32 and 64 bit
 * from the same binary. Everything is controlled by reading the
 * macho header to determine what the bitwidth is - nt
 */
struct l4_meminfo_32 {
    L4_Word32_t offset:L4_BITS_PER_WORD_32;
    L4_Word32_t n:L4_BITS_PER_WORD_32 / 2;
    L4_Word32_t max:L4_BITS_PER_WORD_32 / 2;
};

struct l4_meminfo_64 {
    L4_Word64_t offset:L4_BITS_PER_WORD_64;
    L4_Word64_t n:L4_BITS_PER_WORD_64 / 2;
    L4_Word64_t max:L4_BITS_PER_WORD_64 / 2;
};

union l4_memdesc_32 {
    L4_Word32_t   raw[2];
    struct {
        L4_BITFIELD5( L4_Word32_t,
            type        :4,
            t           :4,
            __padding1  :1,
            v           :1,
            low         :L4_BITS_PER_WORD_32 - 10
        );
        L4_BITFIELD2( L4_Word32_t,
            __padding2  :10,
            high        :L4_BITS_PER_WORD_32 - 10
        );
    } x;
};


union l4_memdesc_64 {
    L4_Word64_t   raw[2];
    struct {
        L4_BITFIELD5( L4_Word64_t,
            type        :4,
            t           :4,
            __padding1  :1,
            v           :1,
            low         :L4_BITS_PER_WORD_64 - 10
        );
        L4_BITFIELD2( L4_Word64_t,
            __padding2  :10,
            high        :L4_BITS_PER_WORD_64 - 10
        );
    } x;
};

/*
 * XXX: nt
 *
 * This duplication sucks, but that's what you get for
 * needing both 32- and 64-bit support at run-time.
 */
struct l4_binfo_32 {
        L4_Word32_t           magic;
        L4_Word32_t           version;
        L4_Word32_t           size;
        L4_Word32_t           first_entry;
        L4_Word32_t           num_entries;
        L4_Word32_t           __reserved[3];
};

struct l4_binfo_64 {
        L4_Word64_t           magic;
        L4_Word64_t           version;
        L4_Word64_t           size;
        L4_Word64_t           first_entry;
        L4_Word64_t           num_entries;
        L4_Word64_t           __reserved[3];
};

struct l4_bootrec {
        L4_Word32_t           type;
        L4_Word32_t           version;
        L4_Word32_t           offset_next;
};

struct l4_module_32 {
        L4_Word32_t           type;
        L4_Word32_t           version;
        L4_Word32_t           offset_next;
        L4_Word32_t           start;
        L4_Word32_t           size;
        L4_Word32_t           cmdline_offset;
};
struct l4_module_64 {
        L4_Word64_t           type;
        L4_Word64_t           version;
        L4_Word64_t           offset_next;
        L4_Word64_t           start;
        L4_Word64_t           size;
        L4_Word64_t           cmdline_offset;
};

struct l4_simpleexec_32 {
        L4_Word32_t           type;
        L4_Word32_t           version;
        L4_Word32_t           offset;
        L4_Word32_t           text_pstart;
        L4_Word32_t           text_vstart;
        L4_Word32_t           text_size;
        L4_Word32_t           data_pstart;
        L4_Word32_t           data_vstart;
        L4_Word32_t           data_size;
        L4_Word32_t           bss_pstart, bss_vstart, bss_size;
        L4_Word32_t           initial_ip;
        L4_Word32_t           flags;
        L4_Word32_t           label;
        L4_Word32_t           cmdline_offset;
};

struct l4_simpleexec_64 {
        L4_Word64_t           type;
        L4_Word64_t           version;
        L4_Word64_t           offset;
        L4_Word64_t           text_pstart;
        L4_Word64_t           text_vstart;
        L4_Word64_t           text_size;
        L4_Word64_t           data_pstart;
        L4_Word64_t           data_vstart;
        L4_Word64_t           data_size;
        L4_Word64_t           bss_pstart, bss_vstart, bss_size;
        L4_Word64_t           initial_ip;
        L4_Word64_t           flags;
        L4_Word64_t           label;
        L4_Word64_t           cmdline_offset;
};

/*
 * L4 reference manual, generic booting.
 */
#define ROOTSERVER_SP       (0x40)
#define ROOTSERVER_IP       (0x44)
#define ROOTSERVER_LOW      (0x48)
#define ROOTSERVER_HIGH     (0x4c)

#define ROOTSERVER_SP_64    (0x80)
#define ROOTSERVER_IP_64    (0x88)
#define ROOTSERVER_LOW_64   (0x90)
#define ROOTSERVER_HIGH_64  (0x98)

#define MEMORY_INFO         (0x50)
#define MEMORY_INFO_64      (0xA0)
#define BOOT_INFO           (0xB8)
#define BOOT_INFO_64        (0x170)

/*
 * REMOVE ME XXX -gl
 */
#define KERNEL_OFFSET   (0xF0000000)

/*
 * XXX
 */
#define roundup(x, y)   (((x) + ((y) - 1)) & ~((y) - 1))

/*
 * Returns zero if OK, otherwise fail.
 *
 * XXX: should we return other information in checkfile?
 * XXX: 64-bit
 */
static int
macho_checkfile(mh)
    struct mach_header  *mh;
{
    if ( mh->magic == MH_MAGIC 
             || mh->magic == MH_CIGAM
             || mh->magic == MH_MAGIC_64
             || mh->magic == MH_CIGAM_64 )
        return (0);
    else
        return (-1);
}

static uint32_t
macho_swap32(val)
    uint32_t    val;
{
    return ((val & 0xff) << 24) | 
        (((val >> 8) & 0xff) << 16) | 
        (((val >> 16) & 0xff) << 8) | 
        (val >> 24);
}

static uint64_t 
macho_swap64( uint64_t val )
{
    return ((uint64_t) macho_swap32( val >> 32 ))
        | (((uint64_t) macho_swap32( val )) << 32 );
}

static void
macho_write32(f, addr, val)
    const void  *f;
    uint32_t    *addr;
    uint32_t    val;
{
    if (((const struct mach_header *)f)->magic == MH_MAGIC) 
        *addr = val;
    else if (((const struct mach_header *)f)->magic == MH_CIGAM) 
        *addr = macho_swap32(val);
    else
        fatal("unknown endian");
}

static uint32_t
macho_read32(f, val)
    const void  *f;
    uint32_t    val;
{
        uint32_t magic = ((const struct mach_header *)f)->magic;

    if ( magic == MH_MAGIC  || magic == MH_MAGIC_64) 
        return (val);
    else if ( magic == MH_CIGAM || magic == MH_CIGAM_64 ) 
        return (macho_swap32(val));
    else
        fatal("unknown endian");
}

static uint64_t 
macho_read64( const void *f, uint64_t val )
{
    uint32_t magic = ((const struct mach_header_64 *)f)->magic;

    if ( magic == MH_MAGIC  || magic == MH_CIGAM )
        fatal( "trying to access 64-bit val in 32-bit macho" );
    else if ( magic == MH_MAGIC_64 )
        return val;
    else if ( magic == MH_CIGAM_64 )
        return macho_swap64(val);
    else
        fatal("unknow endian");
}

static void
macho_write64(f, addr, v)
    const void  *f;
    uint64_t    *addr;
    uint64_t    v;
{
    uint32_t    m = ((const struct mach_header_64 *)f)->magic;

    if (m == MH_MAGIC || m == MH_CIGAM)
        fatal("trying to write 64-bit val in 32-bit macho");
    else if (m == MH_MAGIC_64)
        *addr = v;
    else if (m == MH_CIGAM_64)
        *addr = macho_swap64(v);
    else
        fatal("unknown endian");
}
static int
macho_writefile(fd, _buf, s)
    int     fd;
    const void  *_buf;
    size_t      s;
{
    const char  *buf = (const char *)_buf;
    ssize_t     rc;

    do {
        rc = write(fd, buf, s);
        if (rc < 0) 
            fatal("failed.  Error %d (%s)", 
                errno, strerror(errno));
        else if (rc == 0)
            break;

        s -= rc;
        buf += rc;
    } while (s > 0);

    return (0);
}


/* byte-swap a 32-bit segment_command structure */
static void
fix_segment_endian( struct mach_header *mh, struct segment_command  *sc )
{
#define FIX(x) x = macho_read32( mh, (x) )

    /*
     *  we do all the fields, and then swap them back to write
     * them out. Technically we only need to do the ones we
     * modify on the host, but meh.
     */
    if (sc->cmd == LC_SEGMENT) {
        FIX(sc->cmd);
        FIX(sc->cmdsize);
        /* string stays the same */
        FIX(sc->vmaddr);
        FIX(sc->vmsize);
        FIX(sc->fileoff);
        FIX(sc->filesize);
        FIX(sc->maxprot);
        FIX(sc->initprot);
        FIX(sc->nsects);
        FIX(sc->flags);
    } else { /* LC_UNIXTHREAD, for example?  We use that -gl */
            
#if 0  /* Need a -verbose option */
        printf("legion: %s non LC_SEGMENT (is %d)\n",
            __func__, sc->cmd);
#endif
    }
    
#undef FIX
}

/* 32-bit macho version of find_first_segment */
static void
find_first_segment32( struct mach_header *mh, struct segment_command  *sc, void **real_sc  )
{
    struct segment_command  *in_sc;

    in_sc = (struct segment_command *)(mh + 1);

    /* copy the whole thing */
    *sc = *in_sc;

    /* then fix endianess */
    fix_segment_endian( mh, sc );

    /* and assign the "real" pointer so we can find more segments / sections */
    *real_sc = in_sc;
}

#define SECT_DIFF (sizeof(struct section_64) - sizeof(struct section))
#define SEG_DIFF (sizeof(struct segment_command_64) - sizeof(struct segment_command))

static void
fix_segment_size( struct mach_header *mh,
                  struct segment_command *sc, 
                  struct segment_command_64 *sc64 )
{
    uint32_t diff;

    /* copy over all the fields, truncating and swapping endianess */
#define FIX(x) sc->x = macho_read32( mh, (sc64->x) )
#define FIX64(x) sc->x = macho_read64( mh, (sc64->x) )
    
    FIX( cmd );

    /* fix up the segment so we copy it */
    if( sc->cmd == LC_SEGMENT_64 )
        sc->cmd = LC_SEGMENT;

    FIX( cmdsize );
    memcpy( sc->segname, sc64->segname, sizeof(sc->segname));
    FIX64( vmaddr );
    FIX64( vmsize );

    FIX64( fileoff );
    FIX64( filesize );
    FIX( maxprot );
    FIX( initprot );
    FIX( nsects );
    FIX( flags );


    /* correct for header size difference */
    diff = 0;
    diff += SEG_DIFF;
    diff += sc->nsects * SECT_DIFF;
    sc->cmdsize -= diff;


#undef FIX
#undef FIX64
}

/* 64-bit macho version of find_first_segment */
static void
find_first_segment64( struct mach_header *mh, struct segment_command  *sc, void **real_sc )
{
    struct mach_header_64  *mh64;
    struct segment_command_64  *sc64;

    /* 64-bit header is the same as the 32, except end padding */
    mh64 = (struct mach_header_64 *) mh;

    sc64 = (struct segment_command_64 *)(mh64 + 1);

    /* and assign the "real" pointer so we can find more segments / sections */
    *real_sc = sc64;

    /* convert it to 32-bit */
    fix_segment_size( mh, sc, sc64 );
}

static int
is_32bit( struct mach_header *mh )
{
    if ( mh->magic == MH_MAGIC  || mh->magic == MH_CIGAM )
        return 1;
    else
        return 0;
}

static int
is_64bit( struct mach_header *mh )
{
    if( mh->magic == MH_MAGIC_64 || mh->magic == MH_CIGAM_64 ) 
        return 1;
    else
        return 0;
}

static int
is_tstate_32bit( struct thread_command *tc )
{
    if( tc->flavor == x86_THREAD_STATE32 )
        return 1;
    else
        return 0;
}

static int
is_tstate_64bit( struct thread_command *tc )
{
    if( tc->flavor == x86_THREAD_STATE64 )
        return 1;
    else
        return 0;
}



/* 
 * Find the first segment in the mach header. Convert it to a
 * a 32-bit, host endian version in the 'sc' parameteter.
 */
static void 
find_first_segment( struct mach_header *mh, struct segment_command  *sc, void **real_sc )
{
    if( is_32bit( mh ) )
        find_first_segment32( mh, sc, real_sc );
    else if( is_64bit( mh ) )
        find_first_segment64( mh, sc, real_sc );
    else
        fatal( "unknown word size on input macho file?" );
}


static void
find_next_segment32( struct mach_header *mh, struct segment_command  *sc, void **real_sc )
{
    struct segment_command  *in_sc;

    in_sc = (struct segment_command *)((char *)*real_sc + sc->cmdsize);

    /* copy the whole thing */
    *sc = *in_sc;

    /* then fix endianess */
    fix_segment_endian( mh, sc );

    /* and assign the "real" pointer so we can find more segments / sections */
    *real_sc = in_sc;
}

static void
find_next_segment64( struct mach_header *mh, struct segment_command  *sc, void **real_sc )
{
    struct segment_command_64  *sc64;

    /* 64-bit header is the same as the 32, except end padding 
     * we can't trust the modified size, tho
     */
    sc64 = (struct segment_command_64 *) *real_sc;
    sc64 = (struct segment_command_64 *)((char *)*real_sc + sc64->cmdsize);

    /* and assign the "real" pointer so we can find more segments / sections */
    *real_sc = sc64;

    /* convert it to 32-bit */
    fix_segment_size( mh, sc, sc64 );
}

/* find the next segment, converting it to above 32-bit style */
static void
find_next_segment( struct mach_header *mh, struct segment_command  *sc, void **real_sc )
{
    if( is_32bit( mh ) )
        find_next_segment32( mh, sc, real_sc );
    else if( is_64bit( mh ) )
        find_next_segment64( mh, sc, real_sc );
    else
        fatal( "unknown word size on input macho file?" );
    
}


static void
macho_get_section32( struct mach_header *mh, void *real_sc, int n, struct section * outsect )
{
    struct section *sect;
    struct segment_command  *sc;

    sc = real_sc;

    /* sections are in an array after the segment header */
    sect = (struct section *)(sc + 1);

    *outsect = sect[n];
}

static void
macho_get_section64( struct mach_header *mh, void *real_sc, int n, struct section * outsect )
{
    struct section_64 *sect64;
    struct segment_command_64  *sc64;

    sc64 = real_sc;

    /* sections are in an array after the segment header */
    sect64 = (struct section_64 *)(sc64 + 1);

    memcpy(outsect->sectname, sect64[n].sectname, 
        sizeof(outsect->sectname));
    memcpy( outsect->segname, sect64[n].segname, 
        sizeof(outsect->sectname));

#if 0
    printf( "section %d: '%s', '%s'\n", n, 
            outsect->sectname, outsect->segname );
#endif

    /* XXX: how to byte swap these two!? */
#warning need byte swapping here
    outsect->addr      = sect64[n].addr;
    outsect->size      = sect64[n].size;

    /* broken: size is size of the section, not including section header */
    /*outsect->size -= SECT_DIFF;*/

    outsect->offset    = sect64[n].offset;
    outsect->align     = sect64[n].align;
    outsect->reloff    = sect64[n].reloff;
    outsect->nreloc    = sect64[n].nreloc;
    outsect->flags     = sect64[n].flags;
    outsect->reserved1 = sect64[n].reserved1;
    outsect->reserved2 = sect64[n].reserved2;
}


/* pull the nth section header out of the section array. We keep it in
 * file byte-order, but trip the 64-bit values down to a 32-bit one
 */
static void
macho_get_section( struct mach_header *mh, void *real_sc, int n, struct section * outsect )
{
    if( is_32bit( mh ) )
        macho_get_section32( mh, real_sc, n, outsect );
    else if( is_64bit( mh ) )
        macho_get_section64( mh, real_sc, n, outsect );
    else
        fatal( "unknown word size on input file" );
}


static void
thread_state_copy32( struct thread_command *dest, struct thread_command *src )
{
    /* just do a memcpy... */
    memcpy( dest, src, src->cmdsize );
}

static void
thread_state_copy64( struct thread_command *dest, struct thread_command *src )
{
    struct i386_thread_state    *state32;
    struct x86_thread_state64   *state64;

    /* create a 32-bit header from the 64-bit one */
    dest->cmd = LC_UNIXTHREAD;
    dest->cmdsize = 80;  /* XXX */
    dest->flavor = i386_THREAD_STATE;
    dest->count = i386_THREAD_STATE_COUNT;

    /* now setup the IP */
    state32 = (uint32_t*) (dest + 1);
    state64 = (uint64_t*) (src + 1);
    
    /* Copy the IP */
#warning FIXME: endianess
    state32->eip = (uint32_t)state64->rip & ~KERNEL_OFFSET;
    /*
     * These are copied from what the Mach-O toolchain would  
     * generate were it generating an executable for 32-bit.
     * Yes, they need to be set. -gl
     */
    state32->ss = state32->ds = state32->es = 0x1f;
    state32->cs = 0x17;
}

static void
thread_state_copy( struct segment_command *dest, struct segment_command *src )
{
    struct thread_command *tsrc, *tdest;

    tsrc = (struct thread_command *) src;
    tdest = (struct thread_command *) dest;

    /* FIXME: start fixing here */
    /* FIXME: don't think we need mh?!?! */
    if( is_tstate_32bit( tsrc ) )
        thread_state_copy32( tdest, tsrc );
    else if( is_tstate_64bit( tsrc ) )
        thread_state_copy64( tdest, tsrc );
    else
        fatal( "can't find suitable thread state in LC_UNIXTHREAD" );
}


/*
 * XXX: this function is not endian-clean.
 * -gl
 */
static L4_Word32_t
swap_endian_32(L4_Word32_t num)
{
    return OSSwapHostToLittleInt32(num);
}

static L4_Word64_t
swap_endian_64(L4_Word64_t num)
{
    return OSSwapHostToLittleInt64(num);
}

static void
macho_binfo(mod, cnt, offset, boffset, binfosiz, fd, kernstart)
    struct mod              *mod;
    int                     cnt;
    off_t                   *offset;
    off_t                   boffset;
    uint32_t                *binfosiz;
    int                     fd;
    uint32_t                kernstart;
{
    struct segment_command  seg;
    char                    *nam, *tmp;
    int                     i;

    /* 32-bit structs */
    struct l4_binfo_32      l4_binfo_32;
    struct l4_simpleexec_32 l4_smod_32;
    struct l4_module_32     l4_mod_32;

    /* 64-bit structs */
    struct l4_binfo_64      l4_binfo_64;
    struct l4_simpleexec_64 l4_smod_64;
    struct l4_module_64     l4_mod_64;

    struct mach_header     *mh;

    /* Get the kernel's macho header to find out what mode we're in */
    mh = mmap(NULL, mod[0].siz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, kernstart);
    if (mh == MAP_FAILED)
    {
        fatal("could not map kernel file.  Error %d (%s)", errno, strerror(errno));
    }
    if (macho_checkfile(mh) != 0)
    {
        fatal("kernel mach header magic does not match");
    }

    bzero(&seg, sizeof(seg));

    if (is_32bit(mh))
    {
        /* XXX assumes LE target, also elsewhere in this function */
        l4_binfo_32.magic   = swap_endian_32(L4_BOOTINFO_MAGIC);
        l4_binfo_32.version = swap_endian_32(L4_BOOTINFO_VERSION);
        l4_binfo_32.size    = sizeof(l4_binfo_32);
        for (i = 0; i < cnt; i++) {
            switch ((mod + i)->type) {
                case M_SEXEC:
                    l4_binfo_32.size += sizeof(l4_smod_32) + strlen((mod + i)->name) + 1;
                    break;
                case M_RAW:
                    l4_binfo_32.size += sizeof(l4_mod_32) + strlen((mod + i)->name) + 1;
                    break;
                case M_KERN:
                    break;  /*NOTHING*/ 
                default:
                    printf("legion: unrecognized or unsupported " "module %s\n", (mod + i)->name);
            }
        }
        l4_binfo_32.size        = swap_endian_32(l4_binfo_32.size);
        l4_binfo_32.first_entry = swap_endian_32(sizeof(l4_binfo_32));
        l4_binfo_32.num_entries = cnt - 1; /* kernel not included */

        l4_binfo_32.num_entries += 5;
        l4_binfo_32.num_entries = swap_endian_32(l4_binfo_32.num_entries);

        macho_writefile(fd, &l4_binfo_32, sizeof(l4_binfo_32));
    }
    else if (is_64bit(mh))
    {
        /* XXX assumes LE target, also elsewhere in this function */
        l4_binfo_64.magic   = swap_endian_64(L4_BOOTINFO_MAGIC);
        l4_binfo_64.version = swap_endian_64(L4_BOOTINFO_VERSION);
        l4_binfo_64.size    = sizeof(l4_binfo_64);
        for (i = 0; i < cnt; i++) {
            switch ((mod + i)->type) {
                case M_SEXEC:
                    l4_binfo_64.size += sizeof(l4_smod_64) + strlen((mod + i)->name) + 1;
                    roundup(l4_binfo_64.size, 8);
                    break;
                case M_RAW:
                    l4_binfo_64.size += sizeof(l4_mod_64) + strlen((mod + i)->name) + 1;
                    roundup(l4_binfo_64.size, 8);
                    break;
                case M_KERN:
                    break;  /*NOTHING*/ 
                default:
                    printf("legion: unrecognized or unsupported " "module %s\n", (mod + i)->name);
            }
        }
        l4_binfo_64.size        = swap_endian_64(l4_binfo_64.size);
        l4_binfo_64.first_entry = swap_endian_64(sizeof(l4_binfo_64));
        l4_binfo_64.num_entries = cnt - 1; /* kernel not included */

        l4_binfo_64.num_entries += 5;
        l4_binfo_64.num_entries = swap_endian_64(l4_binfo_64.num_entries);

        macho_writefile(fd, &l4_binfo_64, sizeof(l4_binfo_64));
    }
        /*
     * Add one because we would like to write out a dummy info
     * which will be patched in at runtime.  It describes the
     * location of the boot argument structure of boot.efi.  We also
     * need to do this for the device tree and the drivers, and probably
     * the EFI tables.
     *
     * We would like to add this now, but we cannot as the boot.efi
     * bootloader will also end up loading other KEXTs, size unknown
     * in advance.
     *
     * Although we know that it is always placed at the end
     * of the boot image, we do this explicitly so that we are not
     * too reliant on this grey knowledge. -gl
     */

# warning layout this way leads to unaligned access -gl
    for (i = 0; i < cnt; i++) {
        switch ((mod + i)->type) {
            case M_SEXEC:
                /*
                 * XXX: not thread-safe.    
                 */
                nam = strdup(basename((mod + i)->name));
                if (!nam)
                    fatal("no memory");
                /*
                 * XXX: seedy!  But we must do this ...
                 * since miserable Iguana insists on having
                 * this name.  I think that we can add an
                 * extra flag to this program but why 
                 * bother? -gl
                 */
                tmp = rindex(nam, '.');
                if (tmp)
                {
                    *tmp = '\0';
                }

                if (is_32bit(mh))
                {
                    l4_smod_32.type             = swap_endian_32(L4_SIMPLEEXEC);
                    l4_smod_32.version          = swap_endian_32(1);
                    l4_smod_32.cmdline_offset   = swap_endian_32(sizeof(l4_smod_32));
                    l4_smod_32.offset           = swap_endian_32(sizeof(l4_smod_32) + strlen(nam) + 1);
                    l4_smod_32.text_pstart      = swap_endian_32((mod + i)->addr);
                    l4_smod_32.text_vstart      = l4_smod_32.text_pstart;
                    l4_smod_32.data_pstart      = swap_endian_32((mod + i)->addr);
                    l4_smod_32.data_vstart      = l4_smod_32.data_pstart;
                    l4_smod_32.text_size        = swap_endian_32((mod + i)->siz);
                    l4_smod_32.data_size        = l4_smod_32.text_size;
                    l4_smod_32.initial_ip       = swap_endian_32((mod + i)->ip);

                    macho_writefile(fd, &l4_smod_32, sizeof(l4_smod_32));
                    macho_writefile(fd, nam, strlen(nam) + 1);
                    seg.vmsize                  += sizeof(l4_smod_32) + strlen(nam) + 1;
                }
                else if (is_64bit(mh))
                {
                    l4_smod_64.type             = swap_endian_64(L4_SIMPLEEXEC);
                    l4_smod_64.version          = swap_endian_64(1);
                    l4_smod_64.cmdline_offset   = swap_endian_64(sizeof(l4_smod_64));
                    l4_smod_64.offset           = roundup(sizeof(l4_smod_64) + strlen(nam) + 1, 8);
                    l4_smod_64.offset           = swap_endian_64(l4_smod_64.offset);

                    l4_smod_64.text_pstart      = swap_endian_64((mod + i)->addr);
                    l4_smod_64.text_vstart      = l4_smod_64.text_pstart;
                    l4_smod_64.data_pstart      = swap_endian_64((mod + i)->addr);
                    l4_smod_64.data_vstart      = l4_smod_64.data_pstart;
                    l4_smod_64.text_size        = swap_endian_64((mod + i)->siz);
                    l4_smod_64.data_size        = l4_smod_64.text_size;
                    l4_smod_64.initial_ip       = swap_endian_64((mod + i)->ip);

                    macho_writefile(fd, &l4_smod_64, sizeof(l4_smod_64));
                    macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
                    seg.vmsize                  += roundup(sizeof(l4_smod_64) + strlen(nam) + 1, 8);
                }

                free(nam);
                break;

            case M_RAW:
                nam = basename((mod + i)->name);
                if (is_32bit(mh))
                {
                    l4_mod_32.type             = swap_endian_32(L4_MODULE);
                    l4_mod_32.version          = swap_endian_32(1);
                    l4_mod_32.start            = swap_endian_32((mod + i)->addr);
                    l4_mod_32.size             = swap_endian_32((mod + i)->siz);
                    l4_mod_32.cmdline_offset   = swap_endian_32(sizeof(l4_mod_32));
                    l4_mod_32.offset_next      = swap_endian_32(sizeof(l4_mod_32) + strlen(nam) + 1);
                    macho_writefile(fd, &l4_mod_32, sizeof(l4_mod_32));
                    macho_writefile(fd, nam, strlen(nam) + 1);
                    seg.vmsize                 += sizeof(l4_mod_32) + strlen(nam) + 1;
                }
                else if (is_64bit(mh))
                {
                    l4_mod_64.type             = swap_endian_64(L4_MODULE);
                    l4_mod_64.version          = swap_endian_64(1);
                    l4_mod_64.start            = swap_endian_64((mod + i)->addr);
                    l4_mod_64.size             = swap_endian_64((mod + i)->siz);
                    l4_mod_64.cmdline_offset   = swap_endian_64(sizeof(l4_mod_64));
                    l4_mod_64.offset_next      = roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
                    l4_mod_64.offset_next      = swap_endian_64(l4_mod_64.offset_next);

                    macho_writefile(fd, &l4_mod_64, sizeof(l4_mod_64));
                    macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
                    seg.vmsize                 += roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
                }
                break;
            case M_KERN:
                break;  /*NOTHING*/
            default:
                printf("legion: unrecognized module\n");
                break;
        }
    }

    /*
     * Finally, do boot.efi boot args
     */
    nam = "kernbootstruct";
    if (is_32bit(mh))
    {
        bzero(&l4_mod_32, sizeof(l4_mod_32));
        l4_mod_32.type              = swap_endian_32(L4_MODULE);
        l4_mod_32.version           = swap_endian_32(1);
        l4_mod_32.start             = 0;
        l4_mod_32.size              = 0;
        l4_mod_32.cmdline_offset    = swap_endian_32(sizeof(l4_mod_32));
        l4_mod_32.offset_next       = swap_endian_32(sizeof(l4_mod_32) + strlen(nam) + 1);

        macho_writefile(fd, &l4_mod_32, sizeof(l4_mod_32));
        macho_writefile(fd, nam, strlen(nam) + 1);
        seg.vmsize                  += sizeof(l4_mod_32) + strlen(nam) + 1;
    }
    else if (is_64bit(mh))
    {
        bzero(&l4_mod_64, sizeof(l4_mod_64));
        l4_mod_64.type              = swap_endian_64(L4_MODULE);
        l4_mod_64.version           = swap_endian_64(1);
        l4_mod_64.start             = 0;
        l4_mod_64.size              = 0;
        l4_mod_64.cmdline_offset    = swap_endian_64(sizeof(l4_mod_64));
        l4_mod_64.offset_next       = roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
        l4_mod_64.offset_next       = swap_endian_64(l4_mod_64.offset_next);

        macho_writefile(fd, &l4_mod_64, sizeof(l4_mod_64));
        macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
        seg.vmsize                  += roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
    }

    /*
     * No need to do EFI system table: runtime services covers the
     * lot and Iguana will do a miserable assert on us if we
     * try to tell it there is something there. -gl
     */
#if 0
    /*
     * EFI system table
     */
    nam = "efisystab";
    bzero(&l4_mod, sizeof(l4_mod));
    l4_mod.type = OSSwapHostToLittleInt32(L4_MODULE);
    l4_mod.version = OSSwapHostToLittleInt32(1);
    l4_mod.start = 0;
    l4_mod.size = 0;
    l4_mod.cmdline_offset = OSSwapHostToLittleInt32(sizeof(l4_mod));
    l4_mod.offset_next = OSSwapHostToLittleInt32(sizeof(l4_mod) + strlen(nam) + 1);
    macho_writefile(fd, &l4_mod, sizeof(l4_mod));
    macho_writefile(fd, nam, strlen(nam) + 1);
    seg.vmsize += sizeof(l4_mod) + strlen(nam) + 1;
#endif

    /*
     * EFI runtime services
     */
    nam = "efiruntime";
    if (is_32bit(mh))
    {
        bzero(&l4_mod_32, sizeof(l4_mod_32));
        l4_mod_32.type             = swap_endian_32(L4_MODULE);
        l4_mod_32.version          = swap_endian_32(1);
        l4_mod_32.start            = 0;
        l4_mod_32.size             = 0;
        l4_mod_32.cmdline_offset   = swap_endian_32(sizeof(l4_mod_32));
        l4_mod_32.offset_next      = swap_endian_32(sizeof(l4_mod_32) + strlen(nam) + 1);
        macho_writefile(fd, &l4_mod_32, sizeof(l4_mod_32));
        macho_writefile(fd, nam, strlen(nam) + 1);
        seg.vmsize                 += sizeof(l4_mod_32) + strlen(nam) + 1;
    }
    else if (is_64bit(mh))
    {
        bzero(&l4_mod_64, sizeof(l4_mod_64));
        l4_mod_64.type             = swap_endian_64(L4_MODULE);
        l4_mod_64.version          = swap_endian_64(1);
        l4_mod_64.start            = 0;
        l4_mod_64.size             = 0;
        l4_mod_64.cmdline_offset   = swap_endian_64(sizeof(l4_mod_64));
        l4_mod_64.offset_next      = roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
        l4_mod_64.offset_next      = swap_endian_64(l4_mod_64.offset_next);

        macho_writefile(fd, &l4_mod_64, sizeof(l4_mod_64));
        macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
        seg.vmsize                 += roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
    }

    /*
     * EFI memory descriptors
     *
     * XXX temporary until we get it to parse L4 memory descriptors -gl
     */
    nam = "efimemdesc";
    if (is_32bit(mh))
    {
        bzero(&l4_mod_32, sizeof(l4_mod_32));
        l4_mod_32.type                 = swap_endian_32(L4_MODULE);
        l4_mod_32.version              = swap_endian_32(1);
        l4_mod_32.start                = 0;
        l4_mod_32.size                 = 0;
        l4_mod_32.cmdline_offset       = swap_endian_32(sizeof(l4_mod_32));
        l4_mod_32.offset_next          = swap_endian_32(sizeof(l4_mod_32) + strlen(nam) + 1);
        macho_writefile(fd, &l4_mod_32, sizeof(l4_mod_32));
        macho_writefile(fd, nam, strlen(nam) + 1);
        seg.vmsize                     += sizeof(l4_mod_32) + strlen(nam) + 1;
    }
    else if (is_64bit(mh))
    {
        bzero(&l4_mod_64, sizeof(l4_mod_64));
        l4_mod_64.type                 = swap_endian_64(L4_MODULE);
        l4_mod_64.version              = swap_endian_64(1);
        l4_mod_64.start                = 0;
        l4_mod_64.size                 = 0;
        l4_mod_64.cmdline_offset       = swap_endian_64(sizeof(l4_mod_64));
        l4_mod_64.offset_next          = roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
        l4_mod_64.offset_next          = swap_endian_64(l4_mod_64.offset_next);

        macho_writefile(fd, &l4_mod_64, sizeof(l4_mod_64));
        macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
        seg.vmsize                     += roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
    }

    /*
     * Do device tree
     */
    nam = "device_tree";
    if (is_32bit(mh))
    {
        bzero(&l4_mod_32, sizeof(l4_mod_32));
        l4_mod_32.type              = swap_endian_32(L4_MODULE);
        l4_mod_32.version           = swap_endian_32(1);
        l4_mod_32.start             = 0;
        l4_mod_32.size              = 0;
        l4_mod_32.cmdline_offset    = swap_endian_32(sizeof(l4_mod_32));
        l4_mod_32.offset_next       = swap_endian_32(sizeof(l4_mod_32) + strlen(nam) + 1);
        macho_writefile(fd, &l4_mod_32, sizeof(l4_mod_32));
        macho_writefile(fd, nam, strlen(nam) + 1);
        seg.vmsize                  += sizeof(l4_mod_32) + strlen(nam) + 1;
    }
    else if (is_64bit(mh))
    {
        bzero(&l4_mod_64, sizeof(l4_mod_64));
        l4_mod_64.type              = swap_endian_64(L4_MODULE);
        l4_mod_64.version           = swap_endian_64(1);
        l4_mod_64.start             = 0;
        l4_mod_64.size              = 0;
        l4_mod_64.cmdline_offset    = swap_endian_64(sizeof(l4_mod_64));
        l4_mod_64.offset_next       = roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
        l4_mod_64.offset_next       = swap_endian_64(l4_mod_64.offset_next);

        macho_writefile(fd, &l4_mod_64, sizeof(l4_mod_64));
        macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
        seg.vmsize                  += roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);
    }
    /*
     * The device drivers come right after the boot.efi bootargs.
     * We insert a dummy segment for that too.
     * The way that it is done is not ideal, however, there isn't
     * any other way short of parsing the device tree, it is
     * probably sufficient for now.
     *
     * -gl
     */
    nam = "drivers";
    if (is_32bit(mh))
    {
        bzero(&l4_mod_32, sizeof(l4_mod_32));
        l4_mod_32.type              = swap_endian_32(L4_MODULE);
        l4_mod_32.version           = swap_endian_32(1);
        l4_mod_32.start             = 0;
        l4_mod_32.size              = 0;
        l4_mod_32.cmdline_offset    = swap_endian_32(sizeof(l4_mod_32));
        macho_writefile(fd, &l4_mod_32, sizeof(l4_mod_32));
        macho_writefile(fd, nam, strlen(nam) + 1);
        seg.vmsize                  += sizeof(l4_mod_32) + strlen(nam) + 1;

        seg.cmd                     = swap_endian_32(LC_SEGMENT);
        seg.cmdsize                 = swap_endian_32(sizeof(seg));
    }
    else if (is_64bit(mh))
    {
        bzero(&l4_mod_64, sizeof(l4_mod_64));
        l4_mod_64.type              = swap_endian_64(L4_MODULE);
        l4_mod_64.version           = swap_endian_64(1);
        l4_mod_64.start             = 0;
        l4_mod_64.size              = 0;
        l4_mod_64.cmdline_offset    = swap_endian_64(sizeof(l4_mod_64));
        macho_writefile(fd, &l4_mod_64, sizeof(l4_mod_64));
        macho_writefile(fd, nam, roundup(strlen(nam) + 1, 8));
        seg.vmsize                  += roundup(sizeof(l4_mod_64) + strlen(nam) + 1, 8);

        seg.cmd                     = swap_endian_64(LC_SEGMENT);
        seg.cmdsize                 = swap_endian_64(sizeof(seg));
    }

    /*
     * The Bootinfo is stuffed at the end of the last module.
     */
    strcpy(seg.segname, "__L4BOOTINFO");
    if (is_32bit(mh))
    {
        seg.vmaddr      = swap_endian_32(roundup(mod[cnt - 1].addr + mod[cnt - 1].siz, 0x1000/*XXX*/));
        seg.vmsize      += sizeof(struct l4_binfo_32);
        seg.fileoff     = swap_endian_32(*offset);
        seg.filesize    = swap_endian_32(seg.vmsize);
    }
    else if (is_64bit(mh))
    {
        seg.vmaddr      = swap_endian_64(roundup(mod[cnt - 1].addr + mod[cnt - 1].siz, 0x1000/*XXX*/));
        seg.vmsize      += sizeof(struct l4_binfo_64);
        seg.fileoff     = swap_endian_64(*offset);
        seg.filesize    = swap_endian_64(seg.vmsize);
    }

    /*
     * round up XXX clean me up XXX -gl
     */
    strcpy(seg.segname, "__L4BOOTINFO");
    seg.vmsize      = roundup(seg.vmsize, 0x1000/*XXX*/);
    *binfosiz       = seg.vmsize;
    if (is_32bit(mh))
    {
        seg.vmsize  = swap_endian_32(seg.vmsize);
    }
    else if (is_64bit(mh))
    {
        seg.vmsize  = swap_endian_64(seg.vmsize);
    }

    for (;;) {
        char *buf = (char *)&seg;
        ssize_t rc;
        size_t  siz = sizeof(seg);

        rc = pwrite(fd, buf, siz, boffset);
        if (rc <= 0)
            fatal("error during write.  Error %d (%s)", 
                errno, strerror(errno));
        siz -= rc;
        buf += rc;
        *offset += rc;
        if (siz == 0)
            break;
    }
}

static void
macho_freeheader(mh)
    struct mach_header  *mh;
{
    munmap(mh, sizeof(*mh) + macho_read32(mh, mh->sizeofcmds));
}

static void
macho_countsegments(mods, cnt, nsegs, segsiz)
    struct mod  *mods;
    int     cnt;
    uint32_t    *nsegs;
    uint32_t    *segsiz;
{
    int         j, i;
    struct mach_header  *mh;
    struct segment_command  sc;
        void                    *real_sc;

        *segsiz = *nsegs = 0;

    for( i = 0; i < cnt; i++ )
        {
        if (mods[i].type == M_RAW)
                {
            *segsiz += sizeof(struct segment_command);
            *nsegs += 1;
            continue;
        }

        macho_getheader( mods[i].name, &mh );

                find_first_segment( mh, &sc, &real_sc );

        for( j = 0; j < macho_read32(mh, mh->ncmds); j++ )
                {
            if( sc.cmd == LC_SEGMENT &&
                strcmp( sc.segname, "__PAGEZERO" ) )
                        {
                *segsiz += sc.cmdsize;
                *nsegs += 1;
            }

                        find_next_segment( mh, &sc, &real_sc );
        }
        macho_freeheader(mh);
    }
}

static void
macho_buildlegion(mods, cnt, out)
    struct mod  *mods;
    int     cnt;
    const char  *out;
{
    struct mach_header  buf;
    struct mach_header  *bufp;
    struct segment_command  seg;
    struct mod      *lastexec = NULL;
    off_t           modfileoff, offset, binfo;
    uint32_t        rawaddr, kernstart, binfosiz;
    uint32_t        nsegs, segsiz;
    char            *unixthrd;
    int         i;
    int         fd;

    /*
     * First thing we do is build the kernel.
     * Actually, the kernel should not be special, however,
     * there are some hacks that we must apply,
     * so it is here.  We should deprecate this flag
     * when we no longer need such hacks.  Do not depend on this
     * flag, it will change at any time without notice.
     *
     * -gl.
     */
    macho_getheader(mods[0].name, &bufp);
    if (macho_checkfile(bufp) != 0)
        fatal("kernel not a Mach-O");

    memcpy(&buf, bufp, sizeof(struct mach_header));
    macho_freeheader(bufp);

        /* make sure we're outputting a 32-bit macho file */
        if( buf.magic == MH_MAGIC_64 )
            buf.magic = MH_MAGIC;

    /* XXX -gl */
    buf.cputype = CPU_TYPE_I386;
    buf.cpusubtype = CPU_SUBTYPE_I386_ALL;

    /*
     * Steal the header from the kernel.
     *
     * Number of commands is cnt (including kernel) plus 1
     * for bootinfo.
     */
    macho_countsegments(mods, cnt, &nsegs, &segsiz);
    /*
     * Patch up for bootinfo
     */
    buf.ncmds = nsegs + 1;
    buf.sizeofcmds = segsiz + sizeof(struct segment_command);

    /*
     * And fix up LC_UNIXTHREAD: we want the one from the kernel,
     * by the way, because that's the entry point that we want to 
     * jump to.
     */
    macho_write32(&buf, &(buf.ncmds), buf.ncmds + 1);
#warning FIXME - grab off include
    macho_write32(&buf, &(buf.sizeofcmds), buf.sizeofcmds + 80);
    fd = open(out, O_RDWR|O_CREAT|O_TRUNC, 
        S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0)
        fatal("could not open output file.  Errno %d (%s)",
            errno, strerror(errno));
    (void)macho_writefile(fd, (const char *)&buf, sizeof(buf));
    offset = sizeof(buf);   /* just flushed that much to disk */
      
    modfileoff = macho_read32(&buf, buf.sizeofcmds);
    modfileoff += sizeof(buf);

    /*
     * There are basically two ways that you can do this.
     * One is one LC_SEGMENT per module.  This bloats up
     * the thing because now you must pad with zeros 
     * when you previously do not.
     *
     * Alternatively you have the same LC_SEGMENT structure
     * for the file.  This saves space but you must shuffle
     * the offsets around.  We do the latter, because we can
     * simply shift file offsets around.  This works because
     * the headers are embedded in the mh_execute_header, and
     * so the relative offsets from the mh_execute_header will
     * still be correct for programs which parse them.
     *
     * -gl. 
     */
    macho_appendheader(&buf, mods, 0, NULL, &offset, modfileoff, 
        &unixthrd, fd);
    for (i = 1; i < cnt; i++) {
        if (mods[i].type == M_SEXEC) {
            macho_appendheader(&buf, mods, i, NULL, 
                &offset, modfileoff, NULL, fd);
            lastexec = &mods[i];
        }
    }

        if( lastexec != NULL )
        {
            rawaddr = roundup(lastexec->addr + lastexec->siz, 0x1000);
            for (i = 1; i < cnt; i++) {
        if (mods[i].type == M_RAW)
                    macho_appendheader(&buf, mods, i, &rawaddr, 
                                       &offset, modfileoff, NULL, fd);
            }
        }
        else
            rawaddr = 0x1000;

    /*
     * Now, do the bootinfo.
     */
    bzero(&seg, sizeof(seg));
    /*
     * Reserve it first.  We will write it back later once
     * we find out how big those things really are.
     */
    macho_writefile(fd, &seg, sizeof(seg));
    binfo = offset;
    offset += sizeof(seg);

    /*
     * Do the LC_UNIXTHREAD
     */
    macho_writefile(fd, unixthrd, 80/*XXX*/);
    offset += 80;

    kernstart = offset;
    macho_appendmodule(&buf, mods, 0, &offset, fd); /* kernel */

    /*
     * This is done this way because the raw modules must go 
     * last, otherwise we don't know where they live.
     * -gl
     */
    for (i = 0; i < cnt; i++) {
        if (mods[i].type == M_SEXEC) 
            macho_appendmodule(&buf, mods, i, &offset, fd);
    }

    for (i = 1; i < cnt; i++) {
        if (mods[i].type == M_RAW) 
            macho_appendraw(mods, i, &offset, fd);
    }

    for (i = 1; i < cnt; i++) {
        if (mods[i].type != M_RAW && mods[i].type != M_SEXEC) {
            printf("legion: unrecognized or unsupported "
                "module %s (%d)\n", 
                (mods + i)->name,
                (mods + i)->type);
        }
    }


    /*
     * do bootinfo.
     */
    macho_binfo(mods, cnt, &offset, binfo, &binfosiz, fd, kernstart);
    /*
     * Patch up the KIP with:
     * (1) rootserver and sigma0 address
     * (2) bootinfo address
     * (3) memory descriptor to reserve this boot image as
     *     L4 bootloader-specific memory
     */
    macho_patchkip(fd, kernstart, binfo, binfosiz, mods, cnt);
    (void)close(fd);
}

static void
macho_patchkip(fd, kernstart, binfo, binfosiz, mods, cnt)
    int     fd;
    uint32_t    kernstart;
    uint32_t    binfo;
    uint32_t    binfosiz;
    struct mod  *mods;
    int     cnt;
{
    struct mach_header  *mh;
    struct segment_command  sc;
    unsigned char       *kip;
    struct mod      *rt = &mods[1];
    int         i;
        void                    *real_sc;
    struct l4_meminfo_32    l4_meminfo_32;
    struct l4_meminfo_64    l4_meminfo_64;
    union l4_memdesc_32     l4_memdesc_32;
    union l4_memdesc_64     l4_memdesc_64;
    long                    s;
    long                    o;
    L4_Word32_t           *m32;
    L4_Word64_t           *m64;

    mh = mmap(NULL, mods[0].siz, PROT_READ|PROT_WRITE, 
        MAP_SHARED, fd, kernstart);
    if (mh == MAP_FAILED)
        fatal("could not map kernel file.  Error %d (%s)",
            errno, strerror(errno));
    if (macho_checkfile(mh) != 0)
            fatal("kernel mach header magic does not match");
    /*
     * XXX: Not endian safe -gl     
     */
    // sc = (struct segment_command *)(mh + 1);
        find_first_segment( mh, &sc, &real_sc );
    kip = 0;
    for (i = 0; i < macho_read32(mh,mh->ncmds); i++) {
        if (strcmp(sc.segname, "__PAGEZERO") == 0)
            goto next;

        if (strcmp(sc.segname, "__KIP") == 0) {
            kip += (uint32_t)mh;
            break;
        }

        /*
         * Tack on the vmsize.
         */
        if ( sc.cmd == LC_SEGMENT ) 
            kip += sc.filesize;
next:
        // sc = (struct segment_command *)((char *)sc + macho_read32(mh,sc->cmdsize));
                find_next_segment( mh, &sc, &real_sc );
    }

    if (i == macho_read32(mh,mh->ncmds))
        fatal("cannot find KIP");
    /*
     * test KIP magic
     */
    if (kip[0] == 'L' && kip[1] == '4' && kip[2] == 230 && kip[3] == 'K')
        ;
    else
        fatal("KIP magic is wrong");

#define PATCH64(offset, val)    \
    macho_write64(mh, (uint64_t *)(kip + (offset)), val);
#define PATCH(offset, val)  \
    macho_write32(mh, (uint32_t *)(kip + (offset)), val);
#define READ(offset)    \
    macho_read32(mh, *((uint32_t *)(kip + (offset))));
#define READ64(offset)  \
    macho_read64(mh, *((uint64_t *)(kip + (offset))));

#define COPYIN(mem, offset, size)   \
    m32 = (unsigned long*)mem; o = offset; s = size;      \
    while(s > 0) {              \
        *m32 = READ(o);   \
        s -= 4; o += 4; m32++;            \
    }

#define COPYIN64(mem, offset, size) \
    m64 = (uint64_t *)mem; o = offset; s = size;      \
    while(s > 0) {              \
        *m64 = READ64(o); \
        s -= 8; o += 8; m64++;            \
    }

#define COPYOUT(offset, mem, size)  \
    m32 = (unsigned long *)mem; o = offset; s = size;     \
    while(s > 0) {              \
        PATCH(o, *m32);           \
        s -= 4; o += 4; m32++;        \
    }

#define COPYOUT64(offset, mem, size)    \
    m64 = (uint64_t *)mem; o = offset; s = size;      \
    while(s > 0) {              \
        PATCH64(o, *m64);         \
        s -= 8; o += 8; m64++;        \
    }

    if (is_32bit(mh)) {
        PATCH(ROOTSERVER_IP, rt->ip);
        PATCH(ROOTSERVER_LOW, rt->addr);
        PATCH(ROOTSERVER_HIGH, rt->addr + rt->siz);
        /*
         * The bootinfo is an absolute address.
         * Here, we cheat a bit because we know the bootinfo
         * will always come last (after all the raw modules)
         *
         * For now, we only patch for 32bit, we have code that
         * uses this. -gl.
         */
        PATCH(BOOT_INFO,
            roundup(mods[cnt - 1].addr + mods[cnt - 1].siz, 
            0x1000/*XXX*/));
    } else {
        PATCH64(ROOTSERVER_IP_64, rt->ip);
        PATCH64(ROOTSERVER_LOW_64, rt->addr);
        PATCH64(ROOTSERVER_HIGH_64, rt->addr + rt->siz);
        PATCH64(BOOT_INFO_64,
            roundup(mods[cnt - 1].addr + mods[cnt - 1].siz, 
            0x1000/*XXX*/));
    }

    if(is_32bit(mh)) {
        COPYIN(&l4_meminfo_32, MEMORY_INFO, sizeof(l4_meminfo_32));
        printf("L4 Memory Info %p\n", &l4_meminfo_32);
        printf("sizeof of the memory info %d\n", sizeof(struct l4_meminfo_32));
        printf("l4_meminfo_32 offset = 0x%lx, n = %d\n", l4_meminfo_32.offset, l4_meminfo_32.n);
    }
    else if(is_64bit(mh)) {
        COPYIN64(&l4_meminfo_64, MEMORY_INFO_64, sizeof(l4_meminfo_64));
        printf("L4 Memory Info %p\n", &l4_meminfo_64);
        printf("sizeof of the memory info %d\n", sizeof(struct l4_meminfo_64));
        printf("l4_meminfo_64 offset = 0x%lx, n = %d\n", l4_meminfo_64.offset, l4_meminfo_64.n);
    }
    else {
        fatal("Unrecognised macho header magic!");
    }

    if (is_32bit(mh) && (l4_meminfo_32.offset == 0)) {
        printf("WARNING: guessing memdesc offset to be 0x260\n");
        l4_meminfo_32.offset = 0x260;
    }
    else if (is_64bit(mh) && (l4_meminfo_64.offset == 0)) {
        printf("WARNING: guessing memdesc offset to be 0x360\n");
        l4_meminfo_64.offset = 0x360;
    }
#if 0
    /*
     * Both kickstart and dite overwrites the memory info.
     * This means that we can do it too. -gl
     */
    l4_memdesc.x.type = L4_BOOTMEMTYPE;
    l4_memdesc.x.v = 0;
    /*
     * We cheat here, because we know the bootinfo must be the 
     * last thing.  The kernel must be the first thing.
     *
     * For the last thing, work out what is the last module,
     * tack on the space required for the bootinfo, and then
     * shove it into the descriptor.
     */
    l4_memdesc.x.low = mods[0].addr >> 10;
    l4_memdesc.x.high = (roundup(mods[cnt - 1].addr + mods[cnt - 1].siz,
        0x1000/*XXX*/) + roundup(binfosiz, 0x1000/*XXX*/)) >> 10;
    printf("set low %p high %p\n", mods[0].addr, l4_memdesc.x.high << 10);
    COPYOUT(l4_meminfo.offset + l4_meminfo.n * sizeof(l4_memdesc), &l4_memdesc, sizeof(l4_memdesc));
    l4_meminfo.n++;
#endif
#if 1
    /* Add the rootservers initial mapping */

    /* First the physical */
    if(is_32bit(mh)) {
        l4_memdesc_32.x.type = L4_INITIALMAPPING;
        l4_memdesc_32.x.t = 0;
        l4_memdesc_32.x.v = 0;
        l4_memdesc_32.x.low = rt->addr >> 10;
        l4_memdesc_32.x.high = (rt->addr+rt->siz) >> 10;
        COPYOUT(l4_meminfo_32.offset + l4_meminfo_32.n * sizeof(l4_memdesc_32), &l4_memdesc_32, sizeof(l4_memdesc_32));
        l4_meminfo_32.n++;
    }
    else if (is_64bit(mh)) {
        l4_memdesc_64.x.type = L4_INITIALMAPPING;
        l4_memdesc_64.x.t = 0;
        l4_memdesc_64.x.v = 0;
        l4_memdesc_64.x.low = rt->addr >> 10;
        l4_memdesc_64.x.high = (rt->addr+rt->siz) >> 10;
        COPYOUT64(l4_meminfo_64.offset + l4_meminfo_64.n * sizeof(l4_memdesc_64), &l4_memdesc_64, sizeof(l4_memdesc_64));
        l4_meminfo_64.n++;
    }

    /* then the virtual */
    if(is_32bit(mh)) {
        l4_memdesc_32.x.type = L4_INITIALMAPPING;
        l4_memdesc_32.x.t = 0;
        l4_memdesc_32.x.v = 1;
        l4_memdesc_32.x.low = rt->addr >> 10;
        l4_memdesc_32.x.high = rt->addr+rt->siz >> 10;
        COPYOUT(l4_meminfo_32.offset + l4_meminfo_32.n * sizeof(l4_memdesc_32), &l4_memdesc_32, sizeof(l4_memdesc_32));
        l4_meminfo_32.n++;
    }
    else if (is_64bit(mh)) {
        l4_memdesc_64.x.type = L4_INITIALMAPPING;
        l4_memdesc_64.x.t = 0;
        l4_memdesc_64.x.v = 1;
        l4_memdesc_64.x.low = rt->addr >> 10;
        l4_memdesc_64.x.high = rt->addr+rt->siz >> 10;
        COPYOUT64(l4_meminfo_64.offset + l4_meminfo_64.n * sizeof(l4_memdesc_64), &l4_memdesc_64, sizeof(l4_memdesc_64));
        l4_meminfo_64.n++;
    }
#endif

    if(is_32bit(mh)) {
        COPYOUT(MEMORY_INFO, &l4_meminfo_32, sizeof(l4_meminfo_32));
    }
    else if (is_64bit(mh)) {
        COPYOUT64(MEMORY_INFO_64, &l4_meminfo_64, sizeof(l4_meminfo_64));
    }

#undef PATCH
#undef PATCH64
#undef READ
#undef READ64
#undef COPYIN
#undef COPYIN64
#undef COPYOUT
#undef COPYOUT64
}

/*
 * Find the space taken up on disk by mod 0 ... mod idx - 1
 */
static uint32_t
macho_findprevsiz(mods, idx)
    struct mod  *mods;
    int     idx;
{
    int     i;
    uint32_t    res;

    for (res = 0, i = 0; i < idx; res += mods[i++].filesiz)
        ;

    return (res);
}


/*
 * Should be revised to make it less complicated.  In particular
 * since we slurp in the whole struct mod anyway we can do it
 * in one go?
 */
static void
macho_appendheader(mh, mods, idx, rawaddr, offset, modfileoff, 
        threadstate, outfd)
    struct mach_header  *mh;
    struct mod      *mods;
    int         idx;
    uint32_t        *rawaddr;
    off_t           *offset;
    off_t           modfileoff;
    char            **threadstate;
    int         outfd;
{
    struct stat     sbuf;
    const char      *f = mods[idx].name;
    struct mach_header  *bufp;
    struct segment_command  sc, segbuf;
        void                    *real_sc;
    struct section      tmpsect;
    uint32_t        vmsiz;      /* XXX 64-bit */
    uint32_t        vmaddr;     /* XXX 64-bit */
    uint32_t        vmstart, vmend; /* XXX kernel */
    int         i, j;
    int         nsegs;

    mods[idx].siz = 0;
    mods[idx].filesiz = 0;

    vmstart = ~0UL;
    vmend = 0;

    if (mods[idx].type == M_SEXEC || mods[idx].type == M_KERN) {
        macho_getheader(f, &bufp);
        if (macho_checkfile(bufp) != 0)
            fatal("%s not a Mach-O", f);
        vmsiz = 0;
        vmaddr = ~0UL;

                /* find the first segment */
                find_first_segment( bufp, &sc, &real_sc );

        for (nsegs = 0, i = 0; 
            i < macho_read32((char *)mh, bufp->ncmds); i++) {
            /*
             * Sigh.  Need to read this in and then return
             * it to the caller. -gl
             */
            if ( sc.cmd == LC_UNIXTHREAD &&
                             mods[idx].type == M_KERN) 
                        {
                            /* FIXME: need to make a 32-bit threadstate
                             * from the 64-bit one!!
                             */
                            *threadstate = calloc( 1, sc.cmdsize );
                            if (!*threadstate)
                                fatal("no memory");
                            /*
                            memcpy( *threadstate, &sc, 
                                    sc.cmdsize );
                            */
                            /* Use the real (non-mangled) SC */
                            thread_state_copy( (void*) *threadstate, real_sc );
                            goto next;
            }

            /*
             * Else, if just LC_UNIXTHREAD, then fill in
             * the ip.
             */
            if(sc.cmd == LC_UNIXTHREAD) {
                /*
                 * From TGB in dite/src/macho.c: we 
                 * should fix this.  -gl
                 */

                /* 
                 * for magic numbers, see
                 * /usr/include/mach/i386/thread_status.h
                 */
#warning x86 only
#warning check the actual threadstate arch?
                /*
                 * Use bufp because that points to local,
                 * mh points to global header (which is
                 * always munged to be 32bit -gl
                 */
                if(is_32bit(bufp))
                    mods[idx].ip = macho_read32(mh, 
                        ((uint32_t *)real_sc)[14]);
                else
                    mods[idx].ip = macho_read64(bufp, 
                        ((uint64_t *)real_sc)[18]);
                goto next;
            }

            if (sc.cmd != LC_SEGMENT ||
                             strcmp(sc.segname, "__PAGEZERO") == 0) {
#if 0
                            printf( "seg %d not LC_SEGMENT (%d != %d)\n", 
                                    i, LC_SEGMENT, sc.cmd );
#endif
                            goto next;
                        }

            /*
             * We do this mainly due to the kernel,
             * there is a gap between the main
             * kernel and the __INIT.
             */
            if( (sc.vmaddr & ~KERNEL_OFFSET) < vmstart )
                            vmstart = sc.vmaddr & ~KERNEL_OFFSET;
                        if( ((sc.vmaddr & ~KERNEL_OFFSET) 
                             + sc.vmsize) > vmend)
                            vmend = ((sc.vmaddr) & ~KERNEL_OFFSET) + 
                                sc.vmsize;
            nsegs++;
                        if (mods[idx].type == M_KERN)
                        {
                /*
                 * XXX
                 *
                 * kernel is special, until we get
                 * physically addressed kernel.  We
                 * mask with the negation of KERNEL_OFFSET
                 * to remove the high bit to get the 
                 * physical address of where the kernel
                 * should be loaded.
                 * -gl
                 */
                if( (sc.vmaddr & ~KERNEL_OFFSET) < vmaddr )
                    vmaddr = sc.vmaddr & ~KERNEL_OFFSET;
                /*
                 * XXX: Kernel start is special.  This is
                 * wrong anyway, we should look at 
                 * LC_UNIXTHREAD.
                 */
                if( strcmp(sc.segname, "__INIT") == 0 )
                                    mods[idx].ip = sc.vmaddr;
            }
                        else
                        {
                            if( sc.vmaddr < vmaddr )
                                vmaddr = sc.vmaddr;
            }

                        /* now copy to the output buffer */
            memcpy(&segbuf, &sc, sizeof(segbuf));

            /*
             * Fixup the file offset
             */
                        segbuf.fileoff = segbuf.fileoff + modfileoff;

            segbuf.fileoff += macho_findprevsiz(mods, idx);

            /*
             * Chop off the kernel offset.
             */
            segbuf.vmaddr = segbuf.vmaddr & ~KERNEL_OFFSET;

            /* write segment header */
                        {
                            /* byteswap the segbuf for output */
                            struct segment_command wrbuf;

                            wrbuf = segbuf;

                            fix_segment_endian( mh, &wrbuf );

                            macho_writefile( outfd, &wrbuf, sizeof(wrbuf) );
                        }

            /* ... then section headers */
            for( j = 0; j < sc.nsects; j++ )
                        {
                            /* sections are in an array after segments */
                            macho_get_section( bufp, real_sc, j, &tmpsect );
                            
                            /*
                             * chop of the top bit for the kernel.
                             * We do this unconditionally, but it
                             * should  be harmless for everyone.
                             */
                            tmpsect.addr = macho_read32(mh, tmpsect.addr) & ~KERNEL_OFFSET;
                            macho_write32(mh, &(tmpsect.addr), tmpsect.addr);
                            tmpsect.offset = macho_read32(mh, tmpsect.offset) + modfileoff;
                            tmpsect.offset += macho_findprevsiz(mods, idx);

                            /* XXX: 64-bit test */
                            /* We stuff the whole 64-bit macho files in
                             * so no need to do this
                             */
                            // tmpsect.offset -= (j*SECT_DIFF + SEG_DIFF);

                            macho_write32(mh, &(tmpsect.offset), tmpsect.offset);
                            macho_writefile( outfd, &tmpsect, 
                                             sizeof( tmpsect ) );
            }

            *offset += segbuf.cmdsize;
            mods[idx].filesiz += segbuf.filesize;
next:
                        /* move to the next segment */
                        find_next_segment( bufp, &sc, &real_sc );
        }
        /*
         * The algorithm is kind of dodgy, but works.
         * -gl
         */
        mods[idx].addr = vmaddr;
        macho_freeheader(bufp);
    } else if (mods[idx].type == M_RAW) {
        if (stat(f, &sbuf))
            fatal("could not stat.  Error %d (%s)", errno,
                strerror(errno));
        vmsiz = sbuf.st_size;
        vmaddr = *rawaddr;
        *rawaddr = roundup(*rawaddr + vmsiz, 0x1000/*XXX*/);
        mods[idx].addr = vmaddr;
        mods[idx].filesiz = vmsiz;
        nsegs = 1;
        bzero(&segbuf, sizeof(segbuf));
        macho_write32(mh, &(segbuf.cmd), LC_SEGMENT);
        macho_write32(mh, &(segbuf.cmdsize), sizeof(segbuf));
        macho_write32(mh, &(segbuf.filesize), vmsiz);
        segbuf.fileoff += modfileoff;
        segbuf.fileoff += macho_findprevsiz(mods, idx);
                macho_write32(mh, &(segbuf.fileoff), segbuf.fileoff);
        macho_write32(mh, &(segbuf.vmsize), roundup(vmsiz, 0x1000/*XXX*/));
        macho_write32(mh, &(segbuf.vmaddr), vmaddr);
        strcpy(segbuf.segname, "__RAWMODULE");
        macho_writefile(outfd, &segbuf, sizeof(segbuf));
        *offset += sizeof(segbuf) * nsegs;
    } else {
        printf("legion: warning: unknown module\n");
        goto out;
    }

    /*
     * Finally calculate the vmsiz.
     */
    if (mods[idx].type == M_KERN || mods[idx].type == M_SEXEC) 
        vmsiz = (vmend - vmstart);

    mods[idx].siz = vmsiz;
out:
    /* err handling here */
    ;
}

static void
macho_appendraw(mods, idx, offset, outfd)
    struct mod  *mods;
    int     idx;
    off_t       *offset;
    int     outfd;
{
    const char  *f = mods[idx].name;
    int fd;
    char    *buf;
    ssize_t rc;

#define SIZ (512 * 1024)

    fd = open(f, O_RDONLY);
    if (fd < 0)
        fatal("could not open file %s.  Error %d (%s)", 
            f, errno, strerror(errno));
    buf = malloc(SIZ);
    if (!buf)
        fatal("no memory");
    for (;;) {
        rc = read(fd, buf, SIZ);
        if (rc < 0)
            fatal("error when reading file %s Error = %d (%s)",
                 f, errno, strerror(errno));
        if (rc == 0)
            break;
        macho_writefile(outfd, buf, rc);
        *offset += rc;
    }
#undef SIZ
}

static void
macho_appendmodule(mh, mods, idx, offset, outfd)
    struct mach_header  *mh;
    struct mod      *mods;
    int         idx;
    off_t           *offset;
    int         outfd;
{
    char            *f = mods[idx].name;
    char            *buf;
    ssize_t         rc;
    size_t          done;
    int         fd;

#define SIZ (512 * 1024)

    fd = open(f, O_RDONLY); 
    if (fd < 0)
        fatal("could not open file %s.  Error %d (%s)", 
            f, errno, strerror(errno));
    buf = malloc(SIZ);
    if (!buf)
        fatal("no memory");
    done = 0;
    for (;;) {
        rc = pread(fd, buf, SIZ, done);
        if (rc == 0)
            break;
        if (rc < 0)
            fatal("error reading file %s.  Error %d (%s)",
                f, errno, strerror(errno));
        done += rc;
        *offset += rc;
        macho_writefile(outfd, buf, rc);
    }
#undef SIZ
}

void
macho_getheader(f, mh)
    const char      *f;
    struct mach_header  **mh;
{
    char    buf[sizeof(struct mach_header)];
    int fd;

    fd = open(f, O_RDONLY);
    if (fd < 0)
        fatal("could not open file %s (%s)", f, strerror(errno));
    if (read(fd, buf, sizeof(buf)) != sizeof(buf))
        fatal("could not read in header, short read");
    if ((*mh = mmap(NULL, macho_read32(buf, ((struct mach_header *)buf)->sizeofcmds) +
        sizeof(buf),
        PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
        fatal("could not map in mach headers");
    (void)close(fd);
}

/*
 *  XXX not endian safe -gl
 */
static void
macho_dumplegion(f)
    const char  *f;
{
    struct l4_bootrec       *r;
    struct l4_simpleexec_32 *s = NULL;
    struct l4_module_32     *m = NULL;
    struct mach_header      *mh;
    struct segment_command  *sc;
    int                     i, fd;

    struct l4_binfo_32 *b    = NULL;


    macho_getheader(f, &mh);
    sc = (struct segment_command *)(mh + 1);
    for (i = 0; i < macho_read32((const char *)mh, mh->ncmds); i++) {
        if (macho_read32((const char *)mh, sc->cmd) == LC_SEGMENT &&
           strcmp(sc->segname, "__L4BOOTINFO") == 0) 
            break;
        sc = (struct segment_command *)((char *)sc + 
            macho_read32((const char *)mh, sc->cmdsize));
    }

    if (i == macho_read32((const char *)mh, mh->ncmds))
        goto err;

    fd = open(f, O_RDONLY);
    if (fd < 0)
        fatal("could not open %s.  Error %d (%s)", 
            f, errno, strerror(errno));
    if ((b = mmap(NULL, macho_read32((const char *)mh, sc->vmsize), PROT_READ, MAP_PRIVATE, fd, 
        macho_read32((const char *)mh, sc->fileoff))) == MAP_FAILED)
        fatal("could not map bootinfo.  Error %d (%s)",
            errno, strerror(errno));
    /*
     * XXX this stuff is not endian-safe.
     * -gl
     */
    if (macho_read32((const char *)mh, b->magic) != L4_BOOTINFO_MAGIC)
        fatal("__L4BOOTINFO does not contain valid bootinfo");

    for (i = 0, r = (struct l4_bootrec *)((char *)b + macho_read32((const char *)mh, b->first_entry));
        i < macho_read32((const char *)mh, b->num_entries);
        i++, r = (struct l4_bootrec *)((char *)r + macho_read32((const char *)mh, r->offset_next))) {
        switch (macho_read32(mh,r->type)) {
            case L4_SIMPLEEXEC:
                s = (struct l4_simpleexec *)r;
                printf("SimpleExec %s\n", 
                    (char *)s + macho_read32((const char *)mh, s->cmdline_offset));
                printf("\ttext %p (p = %p) siz %lu\n",
                    (void *)macho_read32((const char *)mh, s->text_vstart), 
                    (void *)macho_read32((const char *)mh, s->text_pstart),
                    (unsigned long)macho_read32((const char *)mh, s->text_size));
                printf("\tdata %p (p = %p) siz %lu\n",
                    (void *)macho_read32((const char *)mh, s->data_vstart),
                    (void *)macho_read32((const char *)mh, s->data_pstart),
                    (unsigned long)macho_read32((const char *)mh, s->data_size));
                printf("\tentry point %p\n",
                    (void *)macho_read32((const char *)mh, s->initial_ip));
                break;
            case L4_MODULE:
                m = (struct l4_module *)r;
                printf("Module %s\n",
                    (char *)m + macho_read32((const char *)mh, m->cmdline_offset));
                printf("\tmod start %p siz %lu\n",
                    (void *)macho_read32((const char *)mh, m->start),
                    (unsigned long)macho_read32((const char *)mh, m->size));
                break;
            default:
                printf("legion: warning: unrecognized or "
                    "unsupported L4 boot record: %lx\n", r->type);
                break;
        }
    }

    macho_freeheader(mh);
    return;
err:
    fatal("no bootinfo found, cannot dump");
}


static void
fatal(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "legion: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(EX_UNAVAILABLE);
}

static void
version(void)
{
    fprintf(stderr, "legion version %s by %s\n", VERSION, AUTHOR);
    exit(EX_USAGE);
}

static void
usage(void)
{
    fprintf(stderr, "usage: legion -D file\n");
    fprintf(stderr, "       legion -k kern [-rm mod0] [-rm mod1] ... "
        "[-rm modn] "
        "[-o outfile]\n");
    exit(EX_USAGE);
}

int
main(argc, argv)
    int argc;
    char    **argv;
{
    char        *dump, *out;
    struct mod  *mods;
    int     c, i, midx;
    int     dumpflag;
    int     buildflag;

    dumpflag = buildflag = 0;
    dump = out = "a.out";
    mods = malloc(MOD_PREALLOC * sizeof(struct mod));
    if (!mods)
        fatal("cannot allocate mods");
    for (i = 0; i < MOD_PREALLOC; i++) 
        mods[i].name = NULL;
    midx = 1;

    while ((c = getopt(argc, argv, "D:k:m:r:o:dvh?")) != -1) {
        switch (c) {
            case 'D':
                dumpflag = 1;
                dump = strdup(optarg);
                break;
            case 'v':
                version();
                break;
            case 'k':
                buildflag = 1;
                mods[0].name = strdup(optarg);
                mods[0].type = M_KERN;
                break;
            case 'o':
                out = strdup(optarg);
                break;
            case 'm':
                mods[midx].name = strdup(optarg);
                mods[midx].type = M_SEXEC;
                midx++;
                if ((midx % MOD_PREALLOC) == 0) {
                    mods = realloc(mods, 
                        midx * sizeof(struct mod) +
                        MOD_PREALLOC * 
                        sizeof(struct mod));
                    for (i = 0; i < MOD_PREALLOC; i++) 
                        mods[midx + i].name = NULL;
                }
                break;
            case 'r':
                mods[midx].name = strdup(optarg);
                mods[midx].type = M_RAW;
                midx++;
                if ((midx % MOD_PREALLOC) == 0) {
                    mods = realloc(mods, 
                        midx * sizeof(struct mod) +
                        MOD_PREALLOC * 
                        sizeof(struct mod));
                    for (i = 0; i < MOD_PREALLOC; i++)
                        mods[midx + i].name = NULL;
                }
                break;
            case 'h':
            case '?':
            default:
                usage();
                break;
        }
    }

    if (dumpflag && buildflag)
        fatal("dump and kern are mutually exclusive");

    if (dumpflag)
        macho_dumplegion(dump);
#if 0
    else if (buildflag && midx < 2)
        fatal("at least kernel and root server required\n");
#endif
    else if (buildflag) 
        macho_buildlegion(mods, midx, out);
    else
        fatal("cannot determine operation (dump or build legion)");

    exit(EX_OK);
}

