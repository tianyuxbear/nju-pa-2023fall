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

	FILE *fp = fopen(elf_file, "rb");
	if(fp == NULL){
		Log("Open ELF file error");
		return;
	}

	int ret;

	Elf64_Ehdr elf_header;
	ret = fread(&elf_header, sizeof(Elf64_Ehdr), 1, fp);
	if(ret != sizeof(Elf64_Ehdr)){
		Log("Read elf header error");
		fclose(fp);
		return;
	}

	ret = fseek(fp, elf_header.e_shoff, SEEK_SET);
	if(ret != 0){
		Log("Seek section headers error");
		fclose(fp);
		return;
	}
	Elf64_Shdr* section_headers = malloc(elf_header.e_shnum * elf_header.e_shentsize);
	if(section_headers == NULL){
		Log("Malloc section headers error");
		fclose(fp);
		return;
	}
	ret = fread(section_headers, elf_header.e_shentsize, elf_header.e_shnum, fp);
	if(ret != elf_header.e_shentsize){
		Log("Read section headers error");
		fclose(fp);
		free(section_headers);
		return;
	}

	int symtab_index = -1, strtab_index = -1;
	for(int i = 0; i < elf_header.e_shnum; i++){
		if(section_headers[i].sh_type == SHT_SYMTAB){
			symtab_index = i;
		}else if(section_headers[i].sh_type == SHT_STRTAB){
			strtab_index = i;
		}
	}

	if(symtab_index == -1 || strtab_index  == -1){
		Log("Symbol table or String table not found.");
		fclose(fp);
		free(section_headers);
		return;
	}

	int sym_num = section_headers[symtab_index].sh_size / sizeof(Elf64_Sym);
	if(sym_num > SYM_NUM){
		Log("Too many symbol entry ==> %d", sym_num);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fseek(fp, section_headers[symtab_index].sh_offset, SEEK_SET);
	if(ret != 0){
		Log("Seek symbol table error");
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fread(symtab, sym_num, sizeof(Elf64_Sym), fp);
	if(ret != sym_num){
		Log("Read symbol table error");
		fclose(fp);
		free(section_headers);
		return;
	}

	int str_size = section_headers[strtab_index].sh_size;
	if(str_size > STR_SIZE){
		Log("String table too long ==> %d", str_size);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fseek(fp, section_headers[strtab_index].sh_offset, SEEK_SET);
	if(ret != 0){
		Log("Seek string table error");
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fread(strtab, str_size, 1, fp);
	if(ret != str_size){
		Log("Read string table error");
		fclose(fp);
		free(section_headers);
		return;
	}

	Log("The elf file is %s", elf_file);
	ret = fclose(fp);
	if(ret != 0){
		Log("Close file pointer error");
		free(section_headers);
		return;
	}
	free(section_headers);
	return;
}
