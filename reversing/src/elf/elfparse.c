#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../include/elf/elf.h"
#include "../../include/elf/elftable.h"


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


elf_ehdr_common_t* 
getElfHeaderCommon(void* data)
{
	
	elf_ehdr_common_t* _header = (elf_ehdr_common_t*)data;
	return _header; 
}

int
find_elf_headers(void* data, size_t size, uint32_t* headers)
{

	if(!data || size <= 0) return 0;

	uint32_t off = 0;
	int numheaders = 0;

	while(off < size)
	{
		elf_ehdr_common_t* header = (elf_ehdr_common_t*)(data+off);
		
		if(header != NULL) {
			if(!strncmp((char*)header->e_ident, "\177ELF", 4)) {
				headers = realloc(headers, 1 * sizeof(uint32_t) + sizeof(uint32_t) * numheaders);
				headers[numheaders] = off;
				numheaders++;
			}
		}

		off+=sizeof(elf_ehdr_common_t);
	}
	return numheaders > 0;
}

int 
parse_elf_headers(int numheaders, uint32_t* headers, void *data)
{
	for(int i = 0; i < numheaders; i++) {

		elf_ehdr_common_t* header = getElfHeaderCommon(data+headers[i]);
		printf("off = %#x\n", headers[i]);
		print_elf_ehdr_common(header);
	}
	return 1;
}

int 
parse_elf_binary(const char* path)
{

	int bitsize;
	char* os_type;
	
	FILE* binaryFile = NULL;
	char* data = NULL;

	int i = 0;
	uint32_t size, result;
	
	
	binaryFile = fopen(path, "rb");

	if (!binaryFile)
	{
		printf("Error opening image %s\n", path);
		return 1;
	}
	
	fseek(binaryFile, 0, SEEK_END);
	size = ftell(binaryFile);
	rewind(binaryFile);

	data = (char*)malloc(sizeof(char)*(size));
	if (!data)
	{
		printf("Failed to allocate buffer!\n");
		return 1;
	}
	
	result = fread((data), 1, size, binaryFile);
	if (result != size)
	{
		printf("Error reading image %s\n\r", path);
		return 1;
	}
	fclose(binaryFile);
	
	uint32_t* headers = malloc(sizeof(uint32_t));

	int numheaders = find_elf_headers((void*)data, size, headers);
	if(numheaders <= 0)
	{
		printf("This image does not contain an ELF binary.\n");
	}
	else {
		printf("Found %d elf header(s).\n", numheaders);
		parse_elf_headers(numheaders, headers, data);
	}
	if(headers)
		free(headers);
	if(data)
		free(data);
}

int main(int argc, char *argv[])
{
	parse_elf_binary(argv[1]);
	return 0;
}