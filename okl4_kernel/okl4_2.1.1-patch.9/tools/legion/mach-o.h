/*
 * mach-o.h
 *
 * Geoffrey Lee < glee at cse.unsw.edu.au >
 *
 * XXX THIS IS INCOMPLETE XXX
 * XXX These headers are not 64-bit clean!
 */

#ifndef _LIBMACHO_
#define _LIBMACHO_

#ifdef __cplusplus
extern "C" {
#endif

#define EMACHOSUCCESS	0
#define EMACHOBADFILE	1

#if 1
#define dprintf(args...)	printf(args)
#else
#define dprintf(args...)
#endif


#define FAT_MAGIC	0xcafebabe
#define FAT_CIGAM	0xbebafeca

/* thread state constants */
#define x86_THREAD_STATE32		1
#define x86_THREAD_STATE64		4


/* Constant for the magic field of the mach_header */
#define MH_MAGIC        0xfeedface
#define MH_CIGAM        0xcefaedfe

#define MH_EXECUTE	0x0
#define MH_OBJECT	0x1


typedef int		cpu_type_t;	/* integer_t */
typedef int		cpu_subtype_t;	/* integer_t */

struct fat_header {
	uint32_t	magic;
	uint32_t	nfat_arch;
};

struct fat_arch {
	cpu_type_t	cputype;
	cpu_subtype_t	cpusubtype;
	uint32_t	offset;
	uint32_t	size;
	uint32_t	align;
};

/*
 * The mach header appears at the very beginning of the object file.
 */
struct mach_header {
        uint32_t   magic;
        cpu_type_t      cputype;
        cpu_subtype_t   cpusubtype;
        uint32_t   filetype;
        uint32_t        ncmds;
        uint32_t        sizeofcmds;
        uint32_t   flags;
};

struct load_command {
        uint32_t cmd;
        uint32_t cmdsize;
};

struct thread_command {
    uint32_t    cmd;		/* LC_THREAD or  LC_UNIXTHREAD */
    uint32_t    cmdsize;	/* total size of this command */
    uint32_t    flavor;	        /* flavor of thread state */
    uint32_t    count;	        /* count of longs in thread state */
    /* struct XXX_thread_state state   thread state for this flavor */
    /* ... */
};


/*
 * XXX
 *
 * We only care about LKMs for now.  So far only these have been observed
 * in Mach-O MH_OBJECTs.
 */
#define LC_SEGMENT		0x1
#define LC_SYMTAB		0x2
#define LC_UNIXTHREAD		0x5


struct segment_command {
        uint32_t   cmd;
        uint32_t   cmdsize;
        char            segname[16];
        uint32_t   vmaddr;
        uint32_t   vmsize;
        uint32_t   fileoff;
        uint32_t   filesize;
        int/*vm_prot_t*/	maxprot;
        int/*vm_prot_t*/	initprot;
        uint32_t   nsects;
        uint32_t   flags;
};

struct section {
        char            sectname[16];
        char            segname[16];
        uint32_t   addr;
        uint32_t   size;
        uint32_t   offset;
        uint32_t   align;
        uint32_t   reloff;
        uint32_t   nreloc;
        uint32_t   flags;
        uint32_t   reserved1;
        uint32_t   reserved2;
};

struct symtab_command {
        uint32_t   cmd;
        uint32_t   cmdsize;
        uint32_t   symoff;
        uint32_t   nsyms;
        uint32_t   stroff;
        uint32_t   strsize;
};

struct relocation_info {
   long         r_address;
   unsigned int r_symbolnum:24,
                r_pcrel:1,
                r_length:2,
                r_extern:1,
                r_type:4;
};
#define R_ABS   0

struct nlist {
        union {
                char *n_name;
                long  n_strx;
        } n_un;
        unsigned char n_type;
        unsigned char n_sect;
        short         n_desc;
        uint32_t n_value;
};

#define N_STAB  0xe0
#define N_PEXT  0x10
#define N_TYPE  0x0e
#define N_EXT   0x01

#define N_UNDF  0x0
#define N_ABS   0x2
#define N_SECT  0xe
#define N_PBUD  0xc
#define N_INDR  0xa

#define R_SCATTERED 0x80000000


/*
 * 64-bit Mach-O support added by Charles Gray
 */

/*
 * The 64-bit mach header appears at the very beginning of object files for
 * 64-bit architectures.
 */
struct mach_header_64 {
        uint32_t        magic;          /* mach magic number identifier */
        cpu_type_t      cputype;        /* cpu specifier */
        cpu_subtype_t   cpusubtype;     /* machine specifier */
        uint32_t        filetype;       /* type of file */
        uint32_t        ncmds;          /* number of load commands */
        uint32_t        sizeofcmds;     /* the size of all the load commands */
        uint32_t        flags;          /* flags */
        uint32_t        reserved;       /* reserved */
};

/* Constant for the magic field of the mach_header_64 (64-bit architectures) */
#define MH_MAGIC_64 0xfeedfacf /* the 64-bit mach magic number */
#define MH_CIGAM_64 0xcffaedfe /* NXSwapInt(MH_MAGIC_64) */

#define LC_SEGMENT_64   0x19    /* 64-bit segment of this file to be
                                   mapped */
#define LC_ROUTINES_64  0x1a    /* 64-bit image routines */

struct segment_command_64 { /* for 64-bit architectures */
        uint32_t        cmd;            /* LC_SEGMENT_64 */
        uint32_t        cmdsize;        /* includes sizeof section_64 structs */
        char            segname[16];    /* segment name */
        uint64_t        vmaddr;         /* memory address of this segment */
        uint64_t        vmsize;         /* memory size of this segment */
        uint64_t        fileoff;        /* file offset of this segment */
        uint64_t        filesize;       /* amount to map from the file */
        int/*vm_prot_t*/       maxprot;        /* maximum VM protection */
        int/*vm_prot_t*/       initprot;       /* initial VM protection */
        uint32_t        nsects;         /* number of sections in segment */
        uint32_t        flags;          /* flags */
};

struct section_64 { /* for 64-bit architectures */
        char            sectname[16];   /* name of this section */
        char            segname[16];    /* segment this section goes in */
        uint64_t        addr;           /* memory address of this section */
        uint64_t        size;           /* size in bytes of this section */
        uint32_t        offset;         /* file offset of this section */
        uint32_t        align;          /* section alignment (power of 2) */
        uint32_t        reloff;         /* file offset of relocation entries */
        uint32_t        nreloc;         /* number of relocation entries */
        uint32_t        flags;          /* flags (section type and attributes)*/
        uint32_t        reserved1;      /* reserved (for offset or index) */
        uint32_t        reserved2;      /* reserved (for count or sizeof) */
        uint32_t        reserved3;      /* reserved */
};


/*
 * typedef soup: in Darwin mach_msg_type_number_t -> natural_t ->
 * __darwin_natural_t -> unsigned int on x86
 */
typedef unsigned int mach_msg_type_number_t;

/*
 * XXX mach/i386/thread_status.h
 * XXX make 64-bit clean -gl
 */
struct i386_thread_state {
    unsigned int        eax;
    unsigned int        ebx;
    unsigned int        ecx;
    unsigned int        edx;
    unsigned int        edi;
    unsigned int        esi;
    unsigned int        ebp;
    unsigned int        esp;
    unsigned int        ss;
    unsigned int        eflags;
    unsigned int        eip;
    unsigned int        cs;
    unsigned int        ds;
    unsigned int        es;
    unsigned int        fs;
    unsigned int        gs;
};

struct x86_thread_state64 {
    uint64_t            rax;
    uint64_t            rbx;
    uint64_t            rcx;
    uint64_t            rdx;
    uint64_t            rdi;
    uint64_t            rsi;
    uint64_t            rbp;
    uint64_t            rsp;
    uint64_t            r8;
    uint64_t            r9;
    uint64_t            r10;
    uint64_t            r11;
    uint64_t            r12;
    uint64_t            r13;
    uint64_t            r14;
    uint64_t            r15;
    uint64_t            rip;
    uint64_t            rflags;
    uint64_t            cs;
    uint64_t            fs;
    uint64_t            gs;
} ;

/*
 * XXX mach/i386/thread_status.h
 */
#define i386_THREAD_STATE	1
#define i386_THREAD_STATE_COUNT ((mach_msg_type_number_t) \
    ( sizeof (struct i386_thread_state) / sizeof (int) ))

#define CPU_SUBTYPE_INTEL(f, m) ((cpu_subtype_t) (f) + ((m) << 4))

/*
 * mach/machine.h
 */
#define CPU_TYPE_I386			7
#define CPU_SUBTYPE_I386_ALL		CPU_SUBTYPE_INTEL(3, 0)

#ifdef __cplusplus
};
#endif

#endif	/* _LIBMACHO_ */
