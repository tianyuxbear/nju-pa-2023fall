#include <common.h>
#include <elf.h>

#define SYM_NUM 128
#define STR_SIZE 1024

Elf64_Sym symtab[SYM_NUM];
char strtab[STR_SIZE];


void init_elf(const char* elf_file){
	if(elf_file == NULL) {
		Log("ELF file is not given");
		return;
	}
	Log("The elf file is %s", elf_file);
}