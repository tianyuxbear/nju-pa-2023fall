#include <common.h>
#include <elf.h>

#define SYM_NUM 128
#define STR_SIZE 1024

Elf64_Sym symtab[SYM_NUM];
char strtab[STR_SIZE];
int sym_num = 0, str_size = 0;

#define FTRACE_BUF_SIZE 256
char ftrace_buf[FTRACE_BUF_SIZE];
int depth = 0;

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
	if(ret != 1){
		Log("Read elf header error ==> ret: %d", ret);
		fclose(fp);
		return;
	}

	ret = fseek(fp, elf_header.e_shoff, SEEK_SET);
	if(ret != 0){
		Log("Seek section headers error ==> ret: %d", ret);
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
	if(ret != elf_header.e_shnum){
		Log("Read section headers error ==> ret: %d", ret);
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

	sym_num = section_headers[symtab_index].sh_size / sizeof(Elf64_Sym);
	if(sym_num > SYM_NUM){
		Log("Too many symbol entry ==> %d", sym_num);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fseek(fp, section_headers[symtab_index].sh_offset, SEEK_SET);
	if(ret != 0){
		Log("Seek symbol table error ==> ret: %d", ret);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fread(symtab, sizeof(Elf64_Sym), sym_num, fp);
	if(ret != sym_num){
		Log("Read symbol table error ==> ret: %d", ret);
		fclose(fp);
		free(section_headers);
		return;
	}

	str_size = section_headers[strtab_index].sh_size;
	if(str_size > STR_SIZE){
		Log("String table too long ==> %d", str_size);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fseek(fp, section_headers[strtab_index].sh_offset, SEEK_SET);
	if(ret != 0){
		Log("Seek string table error ==> ret: %d", ret);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fread(strtab, 1, str_size, fp);
	if(ret != str_size){
		Log("Read string table error ==> ret: %d", ret);
		fclose(fp);
		free(section_headers);
		return;
	}

	Log("The elf file is %s", elf_file);
	ret = fclose(fp);
	if(ret != 0){
		Log("Close file pointer error ==> ret: %d", ret);
		free(section_headers);
		return;
	}
	free(section_headers);
	return;
}

void check_jal(word_t pc, word_t dnpc, int rd){
	if(rd != 1) return;
	
	//may be a function call instruction
	for(int i = 0; i < sym_num; i++){
		if(symtab[i].st_info !=  STT_FUNC) continue;
		if(dnpc >= symtab[i].st_value && dnpc < symtab[i].st_value + symtab[i].st_size)
		{
			memset(ftrace_buf, 0, FTRACE_BUF_SIZE);
			snprintf(ftrace_buf, 21, "0x%016lx: ", pc);
			for(int j = 0; j < depth; j++){
				snprintf(ftrace_buf + strlen(ftrace_buf), 3, " ");
			}
			char* ptr = strtab + symtab[i].st_name;
			snprintf(ftrace_buf + strlen(ftrace_buf), FTRACE_BUF_SIZE - strlen(ftrace_buf), "call [%s@0x%016lx]", ptr, dnpc);
			puts(ftrace_buf);
			depth++;
			return;
		}
	}
	return;
}

// void check_jalr(word_t pc, word_t dnpc, int rd, int rs1, int offset){
// 	// ret instruction
// 	if(rd == 0 && rs1 == 1 && offset == 0){
// 		for(int i = 0; i < sym_num; i++){
// 			if(symtab[i].st_info !=  STT_FUNC) continue;
// 			if(dnpc >= symtab[i].st_value && dnpc < symtab[i].st_value + symtab[i].st_size)
// 			{
// 				memset(ftrace_buf, 0, FTRACE_BUF_SIZE);
// 				snprintf(ftrace_buf, 20, FMT_WORD ": ", pc);
// 				for(int j = 0; j < depth; j++){
// 					snprintf(ftrace_buf, 2, "  ");
// 				}
// 				char* ptr = strtab + symtab[i].st_name;
// 				snprintf(ftrace_buf, FTRACE_BUF_SIZE - 20 - depth * 2, "ret [%s@" FMT_WORD "]", ptr, dnpc);
// 				puts(ftrace_buf);
// 				depth--;
// 				return;
// 			}
// 		}
// 	}
// 	check_jal(pc, dnpc, rd);
// }