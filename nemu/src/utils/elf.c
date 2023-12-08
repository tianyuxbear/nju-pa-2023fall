#include <common.h>
#include <elf.h>

#define SH_STR_TAB_SIZE 4096
char shstrtab[SH_STR_TAB_SIZE];

#define SYM_NUM 128
#define STR_TAB_SIZE 4096

Elf64_Sym symtab[SYM_NUM];
char strtab[STR_TAB_SIZE];
int sym_num = 0, str_size = 0;

#define FTRACE_BUF_SIZE 256
char ftrace_buf[FTRACE_BUF_SIZE];

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

	Elf64_Shdr shstr = section_headers[elf_header.e_shstrndx];
	if(shstr.sh_size > SH_STR_TAB_SIZE){
		Log("SH string table too long");
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fseek(fp, shstr.sh_offset, SEEK_SET);
	if(ret != 0){
		Log("Seek section headers string error ==> ret: %d", ret);
		fclose(fp);
		free(section_headers);
		return;
	}
	ret = fread(shstrtab, 1, shstr.sh_size, fp);
	if(ret != shstr.sh_size){
		Log("Read section headers string error ==> ret: %d", ret);
		fclose(fp);
		free(section_headers);
		return;
	}

	int symtab_index = -1, strtab_index = -1;
	for(int i = 0; i < elf_header.e_shnum; i++){
		if(i == elf_header.e_shstrndx) continue;
		Elf64_Shdr temp = section_headers[i];
		if(strncmp(".symtab", shstrtab + temp.sh_name, strlen(".symtab")) == 0){
			symtab_index = i;
		}
		if(strncmp(".strtab", shstrtab + temp.sh_name, strlen(".strtab")) == 0){
			strtab_index = i;
		}
		if(symtab_index != -1 && strtab_index != -1) break;
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

	printf("str_offset: 0x%lx, str_size: 0x%lx\n", section_headers[strtab_index].sh_offset, section_headers[strtab_index].sh_size);

	str_size = section_headers[strtab_index].sh_size;
	if(str_size > STR_TAB_SIZE){
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

#define MAX_DEPTH 64 
char ftrace[MAX_DEPTH][32];
int depth = 0;

void check_jal(word_t pc, word_t dnpc){
	//may be a function call instruction
	int pc_index = -1, dnpc_index = -1;
	for(int i = 0; i < sym_num; i++){
		if((symtab[i].st_info & 0xf) != STT_FUNC) continue;
		if(pc >= symtab[i].st_value && pc < symtab[i].st_value + symtab[i].st_size)
			pc_index = i;
		if(dnpc >= symtab[i].st_value && dnpc < symtab[i].st_value + symtab[i].st_size)
			dnpc_index = i;
		if(pc_index != -1 && dnpc_index != -1) break;
	}

	memset(ftrace_buf, 0, FTRACE_BUF_SIZE);
	snprintf(ftrace_buf, 21, "0x%016lx: ", pc);
	for(int j = 0; j < depth; j++)
		snprintf(ftrace_buf + strlen(ftrace_buf), 3, "  ");
	
	char* pc_ptr = strtab + symtab[pc_index].st_name;
	char* dnpc_ptr = strtab + symtab[dnpc_index].st_name;
	snprintf(ftrace_buf + strlen(ftrace_buf), FTRACE_BUF_SIZE - strlen(ftrace_buf), "call %s [%s ==> %s]", dnpc_ptr, pc_ptr, dnpc_ptr);
	strncpy(ftrace[depth], pc_ptr, strlen(pc_ptr) + 1);
	depth++;
	IFDEF(CONFIG_FTRACE, puts(ftrace_buf));
#ifdef CONFIG_FTRACE_COND
	if(FTRACE_COND) {log_write("%s\n", ftrace_buf);};
#endif

	return;
}

void check_jalr(word_t pc, word_t dnpc, int rd, int rs1, int offset){
	// ret instruction
	if(rd == 0 && rs1 == 1 && offset == 0){
		int pc_index = -1, dnpc_index = -1;
		for(int i = 0; i < sym_num; i++){
			if((symtab[i].st_info & 0xf) != STT_FUNC) continue;
			if(pc >= symtab[i].st_value && pc < symtab[i].st_value + symtab[i].st_size)
				pc_index = i;
			if(dnpc >= symtab[i].st_value && dnpc < symtab[i].st_value + symtab[i].st_size)
				dnpc_index = i;
			if(pc_index != -1 && dnpc_index != -1) break;
		}

		memset(ftrace_buf, 0, FTRACE_BUF_SIZE);
		snprintf(ftrace_buf, 21, "0x%016lx: ", pc);
		char* pc_ptr = strtab + symtab[pc_index].st_name;
		char* dnpc_ptr = strtab + symtab[dnpc_index].st_name;

		depth--;
		while(strncmp(dnpc_ptr, ftrace[depth], strlen(dnpc_ptr)) != 0) depth--;
		for(int j = 0; j < depth; j++)
			snprintf(ftrace_buf + strlen(ftrace_buf), 3, "  ");
	
		snprintf(ftrace_buf + strlen(ftrace_buf), FTRACE_BUF_SIZE - strlen(ftrace_buf), "ret from %s [%s <== %s]", pc_ptr, dnpc_ptr, pc_ptr);

		IFDEF(CONFIG_FTRACE, puts(ftrace_buf));
#ifdef CONFIG_FTRACE_COND
	if(FTRACE_COND) {log_write("%s\n", ftrace_buf);};
#endif

		return;
	}
	check_jal(pc, dnpc);
}