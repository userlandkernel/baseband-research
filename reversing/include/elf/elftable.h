
typedef struct ELF_OBJECT_TYPE_TABLE_ENTRY {
	char* name;
	int type;
} elf_object_type_table_entry_t;

elf_object_type_table_entry_t elf_object_type_table[] =
{
	{"Unknown type", kELF_OBJTYPE_NONE},
	{"Relocatable file", kELF_OBJTYPE_REL},
	{"Executable", kELF_OBJTYPE_EXEC},
	{"Dynamically shared object", kELF_OBJTYPE_DYN},
	{"Core file", kELF_OBJTYPE_CORE}
};
elf_machine_type_t elf_machine_type_table[] =
{
	{"none",						0},
	{"AT&T WE 32100 (M32)", 		1},
	{"SPARC", 						2},
	{"Intel 80386", 				3},
	{"Motorola 68000",				4},
	{"Motorola 88000",				5},
	{"AMD x86-64",					62}
};
