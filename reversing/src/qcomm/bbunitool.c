#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../../include/qcomm/mbn.h"
#include "../../include/qcomm/dmode.h"
#include "../../include/qcomm/dm_ssm.h"
#include "../../include/elf/elf.h"
#include "../../include/elf/elftable.h"
#include "../../include/qcomm/sahara.h"
#include "../../include/qcomm/dload.h"
#include "../../include/qcomm/streaming-dload.h"
#include "../../include/packed.h"

/* Globals */
#define PAD_BYTE_1 255
#define PAD_BYTE_0 0

#define SHA256_SIGNATURE_SIZE 256
#define MAX_NUM_ROOT_CERTS 4

#define MI_BOOT_IMG_HDR_SIZE 40
#define MI_BOOT_SBL_HDR_SIZE 80

#define IMG_MAX	26
#define BOOT_HEADER_LENGTH 20
#define SBL_HEADER_LENGTH 20
#define MBN_HEADER_LEN	0x28

#define FLASH_PARTI_VERSION 3
#define MAX_PHDR_COUNT 100

#define CERT_CHAIN_ONEROOT_MAXSIZE 6*1024 //6kb

#define VIRTUAL_BLOCK_SIZE 131072
#define MAGIC_COOKIE_LENGTH 12
#define MIN_IMAGE_SIZE_WITH_PAD 256*1024 //256kb

#define SBL_AARCH64 0xF // ARM 64-bit
#define SBL_AARCH32 0x0 // ARM 32-bit

#define HW_ID_OFFSET	0x235

#define ELF_IMAGE	0x464C457F
#define SBL_IMAGE	0x844BDCD1
#define RECOVERY_MODE	0x77665502
#define FOTA_COOKIE	0x64645343


#define FOTA_PARTITION    5
#define DSP3_PARTITION    5

/* Boot header magics */
#define FLASH_CODE_WORD 0x844BDCD1
#define UNIFIED_BOOT_COOKIE_MAGIC_NUMBER 0x33836685
#define MAGIC_NUM 0x73D71034
#define AUTODETECT_PAGE_SIZE_MAGIC_NUM 0x7D0B435A
#define AUTODETECT_PAGE_SIZE_MAGIC_NUM64 0x7D0B5436
#define AUTODETECT_PAGE_SIZE_MAGIC_NUM128 0x7D0B6577
#define SBL_VIRTUAL_BLOCK_MAGIC_NUM 0xD48B54C6

/* Mask for bits 20-27 to parse program header p_flags */
#define MI_PBT_FLAGS_MASK 0x0FF00000

/* Healper defines to help parse ELF program headers */
#define MI_PROG_BOOT_DIGEST_SIZE 20
#define MI_PBT_FLAG_SEGMENT_TYPE_MASK 0x07000000
#define MI_PBT_FLAG_SEGMENT_TYPE_SHIFT 0x18
#define MI_PBT_FLAG_PAGE_MODE_MASK 0x00100000
#define MI_PBT_FLAG_PAGE_MODE_SHIFT 0x14
#define MI_PBT_FLAG_ACCESS_TYPE_MASK 0x00E00000
#define MI_PBT_FLAG_ACCESS_TYPE_SHIFT 0x15
#define MI_PBT_FLAG_POOL_INDEX_MASK 0x08000000
#define MI_PBT_FLAG_POOL_INDEX_SHIFT 0x1B

#define PLL4				0
#define MI2S_OSR_SRC			1
#define MI2S_OSR_CLK			2
#define MI2S_DIV_CLK			3
#define MI2S_BIT_DIV_CLK		4
#define MI2S_BIT_CLK			5
#define PCM_SRC				6
#define PCM_CLK_OUT			7
#define PCM_CLK				8
#define SLIMBUS_SRC			9
#define AUDIO_SLIMBUS_CLK		10
#define SPS_SLIMBUS_CLK			11
#define CODEC_I2S_MIC_OSR_SRC		12
#define CODEC_I2S_MIC_OSR_CLK		13
#define CODEC_I2S_MIC_DIV_CLK		14
#define CODEC_I2S_MIC_BIT_DIV_CLK	15
#define CODEC_I2S_MIC_BIT_CLK		16
#define SPARE_I2S_MIC_OSR_SRC		17
#define SPARE_I2S_MIC_OSR_CLK		18
#define SPARE_I2S_MIC_DIV_CLK		19
#define SPARE_I2S_MIC_BIT_DIV_CLK	20
#define SPARE_I2S_MIC_BIT_CLK		21
#define CODEC_I2S_SPKR_OSR_SRC		22
#define CODEC_I2S_SPKR_OSR_CLK		23
#define CODEC_I2S_SPKR_DIV_CLK		24
#define CODEC_I2S_SPKR_BIT_DIV_CLK	25
#define CODEC_I2S_SPKR_BIT_CLK		26
#define SPARE_I2S_SPKR_OSR_SRC		27
#define SPARE_I2S_SPKR_OSR_CLK		28
#define SPARE_I2S_SPKR_DIV_CLK		29
#define SPARE_I2S_SPKR_BIT_DIV_CLK	30
#define SPARE_I2S_SPKR_BIT_CLK		31

#define MDM9X15_CDP	3675
#define MDM9X15_MTP	3681

/* Segment Type */
typedef enum MI_PBT_SEGMENT_TYPE {
	kMI_PBT_L4_SEGMENT = 0x0,
	kMI_PBT_AMSS_SEGMENT = 0x1,
	kMI_PBT_HASH_SEGMENT = 0x2,
	kMI_PBT_BOOT_SEGMENT = 0x3,
	kMI_PBT_L4BSP_SEGMENT = 0x4,
	kMI_PBT_SWAPPED_SEGMENT = 0x5,
	kMI_PBT_SWAP_POOL_SEGMENT = 0x6,
	kMI_PBT_PHDR_SEGMENT = 0x7
} mi_pbt_segment_type_t;

/* Access Types */
typedef enum MI_PBT_ACCESS_TYPE {
	kMI_PBT_RW_SEGMENT = 0x0,
	kMI_PBT_RO_SEGMENT = 0x1,
	kMI_PBT_ZI_SEGMENT = 0x2,
	kMI_PBT_NOTUSED_SEGMENT = 0x3,
	kMI_PBT_SHARED_SEGMENT = 0x4,
	kMI_PBT_RWE_SEGMENT = 0x5
} mi_pbt_access_type_t;

/* ELF Segment Flag Definitions */
typedef enum MI_PBT_SEGMENT_FLAG_TYPE {
	kMI_PBT_ELF_AMSS_NON_PAGED_RO_SEGMENT  = 0x01200000,
	kMI_PBT_ELF_AMSS_PAGED_RO_SEGMENT  = 0x01300000,
	kMI_PBT_ELF_SWAP_POOL_NON_PAGED_ZI_SEGMENT_INDEX0 = 0x06400000,
	kMI_PBT_ELF_SWAPPED_PAGED_RO_SEGMENT_INDEX0 = 0x05300000,
	kMI_PBT_ELF_SWAP_POOL_NON_PAGED_ZI_SEGMENT_INDEX1 = 0x0E400000,
	kMI_PBT_ELF_SWAPPED_PAGED_RO_SEGMENT_INDEX1 = 0x0D300000,
	kMI_PBT_ELF_AMSS_NON_PAGED_ZI_SEGMENT = 0x01400000,
	kMI_PBT_ELF_AMSS_PAGED_ZI_SEGMENT = 0x01500000,
	kMI_PBT_ELF_AMSS_NON_PAGED_RW_SEGMENT = 0x01000000,
	kMI_PBT_ELF_AMSS_PAGED_RW_SEGMENT = 0x01100000,
	kMI_PBT_ELF_AMSS_NON_PAGED_NOTUSED_SEGMENT = 0x01600000,
	kMI_PBT_ELF_AMSS_PAGED_NOTUSED_SEGMENT = 0x01700000,
	kMI_PBT_ELF_AMSS_NON_PAGED_SHARED_SEGMENT = 0x01800000,
	kMI_PBT_ELF_AMSS_PAGED_SHARED_SEGMENT = 0x01900000,
	kMI_PBT_ELF_HASH_SEGMENT = 0x02200000,
	kMI_PBT_ELF_BOOT_SEGMENT = 0x03200000,
	kMI_PBT_ELF_PHDR_SEGMENT = 0x07000000,
	kMI_PBT_ELF_NON_PAGED_L4BSP_SEGMENT = 0x04000000,
	kMI_PBT_ELF_PAGED_L4BSP_SEGMENT = 0x04100000,
	kMI_PBT_ELF_AMSS_RELOCATABLE_IMAGE = 0x8000000
} mi_pbt_segment_flag_type_t;

/* New definitions for EOS demap paging requirement */
#define MI_PBT_ELF_RESIDENT_SEGMENT 0x00000000
#define MI_PBT_ELF_PAGED_LOCKED_SEGMENT 0x00100000
#define MI_PBT_ELF_PAGED_UNLOCKED_SEGMENT 0x01100000
#define MI_PBT_ELF_UNSECURE_SEGMENT 0x03100000

typedef enum MBN_IMAGE_TYPE {
	kMBN_IMAGE_TYPE_NONE = 0,
	kMBN_IMAGE_TYPE_OEM_SBL = 1,
	kMBN_IMAGE_TYPE_AMSS = 2,
	kMBN_IMAGE_TYPE_QCSBL = 3,
	kMBN_IMAGE_TYPE_HASH = 4,
	kMBN_IMAGE_TYPE_APPSBL = 5,
	kMBN_IMAGE_TYPE_APPS = 6,
	kMBN_IMAGE_TYPE_HOSTDL = 7,
	kMBN_IMAGE_TYPE_DSP1 = 8,
	kMBN_IMAGE_TYPE_FSBL = 9,
	kMBN_IMAGE_TYPE_DBL = 10,
	kMBN_IMAGE_TYPE_OSBL = 11,
	kMBN_IMAGE_TYPE_DSP2 = 12,
	kMBN_IMAGE_TYPE_EHOSTDL = 13,
	kMBN_IMAGE_TYPE_NANDPRG = 14,
	kMBN_IMAGE_TYPE_NORPRG = 15,
	kMBN_IMAGE_TYPE_RAMFS1 = 16,
	kMBN_IMAGE_TYPE_RAMFS2 = 17,
	kMBN_IMAGE_TYPE_ADSP_Q5 = 18,
	kMBN_IMAGE_TYPE_APPS_KERNEL = 19,
	kMBN_IMAGE_TYPE_BACKUP_RAMFS = 20,
	kMBN_IMAGE_TYPE_SBL1 = 21,
	kMBN_IMAGE_TYPE_SBL2 = 22,
	kMBN_IMAGE_TYPE_RPM = 23,
	kMBN_IMAGE_TYPE_SBL3 = 24,
	kMBN_IMAGE_TYPE_TZ = 25,
	kMBN_IMAGE_TYPE_PSI = 32
} mbn_image_type_t;

typedef struct mbn_image_id {
	const char *name;
	const mbn_image_type_t image_type_id;
	const char *image_type;
	const char *mbn_type;
} mbn_image_id_collection_t;

mbn_image_id_collection_t image_id_table[] = 
{
	{"appsbl", kMBN_IMAGE_TYPE_APPSBL, "APPSBL_IMG", "bin"},
	{"dbl", kMBN_IMAGE_TYPE_DBL, "DBL_IMG", "bin"},
	{"osbl", kMBN_IMAGE_TYPE_OSBL, "OSBL_IMG", "bin"},
	{"amss", kMBN_IMAGE_TYPE_AMSS, "AMSS_IMG", "elf"},
	{"amss_mbn", kMBN_IMAGE_TYPE_HASH, "HASH_IMG", "elf"},
	{"apps", kMBN_IMAGE_TYPE_APPS, "APPS_IMG", "bin"},
	{"hostdl", kMBN_IMAGE_TYPE_HOSTDL, "HOSTDL_IMG", "bin"},
	{"ehostdl", kMBN_IMAGE_TYPE_EHOSTDL, "EHOSTDL_IMG","bin"},
	{"emmcbld", kMBN_IMAGE_TYPE_EHOSTDL, "EMCCBLD_IMG", "bin"},
	{"qdsp6fw", kMBN_IMAGE_TYPE_DSP1, "DSP1_IMG", "elf"},
	{"qdsp6sw", kMBN_IMAGE_TYPE_DSP2, "DSP2_IMG", "elf"},
	{"qdsp5", kMBN_IMAGE_TYPE_ADSP_Q5, "ADSP_Q5_IMG", "bin"},
	{"tz", kMBN_IMAGE_TYPE_TZ, "TZ_IMG", "elf"},
	{"tz_rumi", kMBN_IMAGE_TYPE_TZ, "TZ_IMG", "elf"},
	{"tz_virtio", kMBN_IMAGE_TYPE_TZ, "TZ_IMG", "elf"},
	{"tzbsp_no_xpu", kMBN_IMAGE_TYPE_TZ, "TZ_IMG", "elf"},
	{"tzbsp_with_test", kMBN_IMAGE_TYPE_TZ, "TZ_IMG", "elf"},
	{"rpm", kMBN_IMAGE_TYPE_RPM, "RPM_IMG", "elf"},
	{"sbl1", kMBN_IMAGE_TYPE_SBL1, "SBL1_IMG", "bin"},
	{"sbl2", kMBN_IMAGE_TYPE_SBL2, "SBL2_IMG", "bin"},
	{"sbl3", kMBN_IMAGE_TYPE_SBL3, "SBL3_IMG", "bin"},
	{"efs1", kMBN_IMAGE_TYPE_RAMFS1, "RAMFS1_IMG", "bin"},
	{"efs2", kMBN_IMAGE_TYPE_RAMFS2, "RAMFS2_IMG", "bin"},
	{"pmic", kMBN_IMAGE_TYPE_PSI, "PSI_IMG", "elf"},
};


char* enumerate_object_type(elf_ehdr_common_t* header)
{
	char* _default = "Unknown object";
	int nObjTypes = sizeof(elf_object_type_table) / sizeof(elf_object_type_table[0]);

	for(int i = 0; i < nObjTypes; i++) {

		if(header->e_type == elf_object_type_table[i].type) {
			return elf_object_type_table[i].name;
		}
	
	}

	return _default;
}

char* enumerate_machine_type(elf_ehdr_common_t* header)
{
	char* _default = "unknown";
	int nMachineTypes = sizeof(elf_machine_type_table) / sizeof(elf_machine_type_table[0]);

	for(int i = 0; i < nMachineTypes; i++) {

		if(header->e_machine == elf_machine_type_table[i].type) {
			return elf_machine_type_table[i].name;
		}
	
	}

	return _default;
}

char* enumerate_endianness_and_bits(elf_ehdr_common_t* header)
{
	int _BIT32 = 1;
	int _BIT64 = 2;

	int _LITTLE_ENDIAN = 1;
	int _BIG_ENDIAN = 2;

	uint8_t* _header = (uint8_t*)header;

	if(_header[4] == _BIT32 && _header[5] == _LITTLE_ENDIAN)
		return "32-bits little endian";
	if(_header[4] == _BIT64 && _header[5] == _LITTLE_ENDIAN)
		return "64-bits little endian";
	if(_header[4] == _BIT32 && _header[5] == _BIG_ENDIAN)
		return "32-bits big endian";
	if(_header[4] == _BIT64 && _header[5] == _BIG_ENDIAN)
		return "64-bits big endian";
	return "Unknown";

}

void print_elf32_ehdr(elf32_ehdr_t* header)
{
	elf_ehdr_common_t* common_header = (elf_ehdr_common_t*)header;
	print_elf_ehdr_common(common_header);
	printf("elf32_ehdr:\n");
	printf("\te_entry: %#x\n", header->e_entry);
	printf("\te_phoff: %#x\n", header->e_phoff);
	printf("\te_shoff: %#x\n", header->e_shoff);
	printf("\te_flags: %#x\n", header->e_flags);
	printf("\te_ehsize: %d\n", header->e_ehsize);
	printf("\te_phentsize: %d\n", header->e_phentsize);
	printf("\te_phnum: %d\n", header->e_phnum);
	printf("\te_shentsize: %d\n", header->e_shentsize);
	printf("\te_shentsize: %d\n", header->e_shnum);
	printf("\te_shstrndx: %d\n\n", header->e_shstrndx);
}

void print_elf32_phdr(elf32_phdr_t* header) 
{
	printf("elf32_phdr:\n");
	printf("\tp_type: %d\n", header->p_type);
	printf("\tp_offset: %#x\n", header->p_offset);
	printf("\tp_vaddr: %#x\n", header->p_vaddr);
	printf("\tp_paddr: %#x\n", header->p_paddr);
	printf("\tp_filesz: %#x\n", header->p_filesz);
	printf("\tp_memsz: %#x\n", header->p_memsz);
	printf("\tp_flags: %#x\n", header->p_flags);
	printf("\tp_align: %#x\n\n", header->p_align);
}

void print_elf64_ehdr(elf64_ehdr_t* header)
{
	elf_ehdr_common_t* common_header = (elf_ehdr_common_t*)header;
	print_elf_ehdr_common(common_header);
	printf("elf64_ehdr:\n");
	printf("\te_entry: %#llx\n", header->e_entry);
	printf("\te_phoff: %#llx\n", header->e_phoff);
	printf("\te_shoff: %#llx\n", header->e_shoff);
	printf("\te_flags: %#x\n", header->e_flags);
	printf("\te_ehsize: %d\n", header->e_ehsize);
	printf("\te_phentsize: %d\n", header->e_phentsize);
	printf("\te_phnum: %d\n", header->e_phnum);
	printf("\te_shentsize: %d\n", header->e_shentsize);
	printf("\te_shentsize: %d\n", header->e_shnum);
	printf("\te_shstrndx: %d\n\n", header->e_shstrndx);
}

void print_elf64_phdr(elf32_phdr_t* header) {
	printf("elf64_phdr:\n");
	printf("\tp_type: %d\n", header->p_type);
	printf("\tp_flags: %#x\n", header->p_flags);
	printf("\tp_offset: %#llx\n", header->p_offset);
	printf("\tp_vaddr: %#llx\n", header->p_vaddr);
	printf("\tp_paddr: %#llx\n", header->p_paddr);
	printf("\tp_filesz: %lld\n", header->p_filesz);
	printf("\tp_memsz: %lld\n", header->p_memsz);
	printf("\tp_align: %#llx\n\n", header->p_align);
}

void print_elf_ehdr_common(elf_ehdr_common_t* header)
{
	printf("elf_ehdr_common:\n");
//n	printf("\te_ident: %s\n", header->e_ident);
	printf("\tEndianess and bitsize: %s\n", enumerate_endianness_and_bits(header));
	printf("\tType: %s (%d)\n", enumerate_object_type(header));
	printf("\tMachine: %s (%d)\n", enumerate_machine_type(header), header->e_machine);
	printf("\tVersion: %d\n\n", header->e_version);
}

void parse_elf_header(void* data) {
	uint8_t* _header = (uint8_t*)data;
	if(_header[4] == 1) {
		print_elf32_ehdr((elf32_ehdr_t*)data);
		return;
	}
	else if(_header[4] == 2) {
		print_elf64_ehdr((elf64_ehdr_t*)data);
		return;
	}
	print_elf_ehdr_common(data);
}

void  print_elf_segment_info(elf_segment_info_t* info)
{
	printf("Flag: %d\n", info->flag);
}

typedef struct regular_boot_header {
	mbn_image_id_collection_t image_id;
	uint64_t flash_parti_ver;
	uint64_t image_src;
	uint64_t image_dest_ptr;
	uint64_t image_size;
	uint64_t code_size;
	uint64_t sig_ptr;
	uint64_t sig_size;
	uint64_t cert_chain_ptr;
	uint64_t cert_chain_size;
	uint64_t magic_number1;
	uint32_t version;
	elf_ostype_t ostype;
	uint64_t boot_apps_parti_entry;
	uint64_t boot_apps_size_entry;
	uint64_t boot_apps_ram_loc;
	uint64_t reserved_ptr;
	uint64_t reserved_1;
	uint64_t reserved_2;
	uint64_t reserved_3;
} regular_boot_header_t;

int 
get_regular_boot_header_length(void)
{
	return BOOT_HEADER_LENGTH;
}

void 
write_regular_boot_header(const char* target, regular_boot_header_t* header, int write_full_header)
{
	void* data = malloc(get_regular_boot_header_length());

	if(header->image_dest_ptr >= 0x100000000) {
		uint32_t* values = (uint32_t*)header;
		values[3] = 0xFFFFFFFF;
	}
	
	if(header->cert_chain_ptr >= 0x100000000) {
		uint32_t* values = (uint32_t*)header;
		values[6] = 0xFFFFFFFF;
	}

	if(header->sig_ptr >= 0x100000000) {
		uint32_t* values = (uint32_t*)header;
		values[8] = 0xFFFFFFFF;
	}

	if(!write_full_header)
	{
		uint32_t* values = (uint32_t*)header;
		memcpy(data, values, 10*sizeof(uint32_t));
	}
	else {
		memcpy(data, (void*)header, sizeof(regular_boot_header_t));
	}
	FILE* fp = fopen(target, "wb");
	fwrite(data, 1, get_regular_boot_header_length(), fp);
	fclose(fp);
}

typedef struct sbl_boot_header {
	uint64_t codeword;
	uint64_t magic;
	uint64_t image_id;
	uint64_t reserved_1;
	uint64_t reserved_2;
	uint64_t img_src;
	void* img_dest_ptr;
	uint64_t img_size;
	uint64_t code_size;
	void* sig_ptr;
	uint64_t sig_size;
	void* cert_chain_ptr;
	uint64_t cert_chain_size;
	uint64_t oem_root_cert_sel;
	uint64_t oem_num_root_certs;
	uint64_t booting_image_config;
	uint64_t reserved_6;
	uint64_t reserved_7;
	uint64_t reserved_8;
	uint64_t reserved_9;
} sbl_boot_header_t;

int get_sbl_boot_header_length()
{
	return SBL_HEADER_LENGTH;
}

void write_sbl_boot_header(const char* target, sbl_boot_header_t* header)
{
	void* data = malloc(sizeof(get_sbl_boot_header_length()));
	memcpy(data, header, sizeof(sbl_boot_header_t));
	FILE* fp = fopen(target, "wb");
	fwrite(data, 1, get_sbl_boot_header_length(), fp);
	fclose(fp);
}


struct sbl_header {
	uint32_t offset;
	char* name;
};

struct mbn_header {
	uint32_t offset;
	char* name;
};

struct mbn_type {
	uint32_t image_id;
	char* image;
	char* name;
};



static struct sbl_header sbl[] = {
	{ 0, "Codeword" },
	{ 4, "Magic" },
	{ 8, "Image ID" },
	{ 20, "Header size" },
	{ 24, "Load address" },
	{ 28, "Image size" },
	{ 32, "Code size" },
	{ 36, "Signature pointer" },
	{ 40, "Signature size" },
	{ 44, "Certificate chain pointer" },
	{ 48, "Certificate chain size" },
	{ 52, "OEM Root Certificate Select" },
	{ 56, "OEM Root Certificates" }
};

static struct mbn_header mbn[] = {
	{ 0, "Image ID" },
	{ 4, "Flash Partition Version" },
	{ 8, "Image source" },
	{ 12, "Load address" },
	{ 16, "Image size" },
	{ 20, "Code size" },
	{ 24, "Signature pointer" },
	{ 28, "Signature size" },
	{ 32, "Certificate chain pointer" },
	{ 36, "Certificate chain size" }
}; 
	
static struct mbn_type type[] = {
	{ 0, "NONE_IMG", "No image" },
	{ 1, "OEM_SBL_IMG", "OEM SBL image" },
	{ 2, "AMSS_IMG", "AMSS image" },
	{ 3, "QCSBL_IMG", "Qualcomm SBL image" },
	{ 4, "HASH_IMG", "Hash image" },
	{ 5, "APPSBL_IMG", "ABOOT/Apps Bootloader image" },
	{ 6, "APPS_IMG", "Apps image" },
	{ 7, "HOSTDL_IMG", "Host DLOAD image" },
	{ 8, "DSP1_IMG", "DSP1 image" },
	{ 9, "FSBL_IMG", "FSBL image" },
	{ 10, "DBL_IMG", "DBL image" },
	{ 11, "OSBL_IMG", "OSBL image" },
	{ 12, "DSP2_IMG", "DSP2 image" },
	{ 13, "EHOSTDL_IMG", "Emergency Host DLOAD image" },
	{ 14, "NANDPRG_IMG", "NAND Programmer image" },
	{ 15, "NORPRG_IMG", "NOR Programmer image" },
	{ 16, "RAMFS1_IMG", "RAM Filesystem 1 image" },
	{ 17, "RAMFS2_IMG", "RAM Filesystem 2 image" },
	{ 18, "ADSP_Q5_IMG", "ADSP Q5 Hexagon image" },
	{ 19, "APPS_KERNEL_IMG", "Apps Kernel image" },
	{ 20, "BACKUP_RAMFS_IMG", "Backup RAM Filesystem image" },
	{ 21, "SBL1_IMG", "First stage Secondary Bootloader image" },
	{ 22, "SBL2_IMG", "Second stage Secondary Bootloader image" },
	{ 23, "RPM_IMG", "Resource and Power Management image" },
	{ 24, "SBL3_IMG", "Third stage Secondary Bootloader image" },
	{ 25, "TZ_IMG", "TrustZone image" }
};

#pragma mark - MOBILE BASEBAND IMAGE PARSER

int parse_mbn(char* image)
{
	int i = 0;
	char str[16] = {0};
	uint32_t offset = 0;
	uint32_t buf[10] = {0};

	for (i=0; i<10; i++)
	{
		buf[i] = *(uint32_t*)(image + (i*4));
	}

	for (i=0; i<26; i++)
	{
		if (buf[0] == type[i].image_id)
		{
			printf("%s [0x%x]\n\n", type[i].image, type[i].image_id);
			printf("***** %s\n\n", type[i].name);
			break;
		}
	}
	for (i=0; i<10; i++)
	{
		printf("%s 0x%X\n", mbn[i].name, buf[i]);
	}
	printf("\n");
	offset = ((buf[8] - buf[3]) + (HW_ID_OFFSET + MBN_HEADER_LEN));

	memcpy(str, (image+offset), 8);
	printf("HW_ID:    %s\n", str);

	offset -= 0x24;
	memcpy(str, (image+offset), 8);
	printf("Rollback: %s\n\n", str);
	return 0;
}

int parse_sbl(char* image)
{
	int i = 0;
	uint32_t buf[13] = {0};

	if (*(uint32_t*)(image+8) == 0x15)
	{
		printf("***********************");
		printf("\nSBL1 Image Detected!\n");
		printf("***********************\n\n");
	}
	else if (*(uint32_t*)(image+8) == 0x0D)
	{
		printf("***********************");
		printf("\neHostDL Image Detected!\n");
		printf("***********************\n\n");
	}
	else
	{
		printf("Could not identify image type!\n");
	}
	
	for (i=0; i<13; i++)
	{
		buf[i] = *(uint32_t*)(image+sbl[i].offset);
		printf("%s: 0x%X\n", sbl[i].name, buf[i]);
	}
	printf("\n");
	return 0;
}

int parse_header(char* image)
{
	FILE* inf = NULL;
	char* data = NULL;

	int i = 0;
	uint32_t size, result;
	
	
	inf = fopen(image, "rb");
	if (!inf)
	{
		printf("Error opening image %s\n", image);
		return 1;
	}
	
	fseek(inf, 0, SEEK_END);
	size = ftell(inf);
	rewind(inf);

	data = (char*)malloc(sizeof(char)*(size));
	if (!data)
	{
		printf("Failed to allocate buffer!\n");
		return 1;
	}
	
	result = fread((data), 1, size, inf);
	if (result != size)
	{
		printf("Error reading image %s\n\r", image);
		return 1;
	}
	fclose(inf);
	
	result = *(uint32_t*)data;
	if (result == SBL_IMAGE)
	{
		printf("Detected SBL1 image!\n\n");
		parse_sbl(data);
	}

	else if (result < IMG_MAX)
	{
		printf("Detected MBN image!\n\n");
		parse_mbn(data);
	}
	else if (result == ELF_IMAGE)
	{
		elf_ehdr_common_t* header = (elf_ehdr_common_t*)data;
		parse_elf_header(header);
	}
	else
	{
		printf("Image could not be identified!\n\n");
	}	

	if(data)
		free(data);

	return 0;
}

int print_usage(void)
{
	printf("\nbbreverse [/path/to/baseband_firmware].\n\n");
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2) return print_usage();
	return parse_header(argv[1]);
}