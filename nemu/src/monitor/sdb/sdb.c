/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/vaddr.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
struct WP;

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char* args){
  char * arg = strtok(NULL, " ");
  if(arg == NULL){
    cpu_exec(1);
  }else{
    uint64_t n = atoi(arg);
    cpu_exec(n);
  }
  return 0;
}

static int cmd_info(char* args){
  char* arg = strtok(NULL, " ");
  if(arg == NULL){
    Log("Miss argument!");
    return 0;
  }
  if(strcmp(arg, "r") == 0){
    isa_reg_display();
  }else if(strcmp(arg, "w") == 0){
    watchpoint_display();
  }else{
    Log("Unknown argument: %s", arg);
  }
  return 0;
}

static int cmd_x(char* args){
  char* arg1 = strtok(NULL, " ");
  if(arg1 == NULL){
    Log("Miss first argument!");
    return 0;
  }
  int nwords = atoi(arg1);


  char* arg2 = arg1 + strlen(arg1) + 1;
  if(arg2 == NULL){
    Log("Miss second argument!");
    return 0;
  }
  bool success = true;
  word_t xaddr = expr(arg2, &success);
  if(!success){
    Log("Input expression error: can't parse");
    return 0;
  }
  for(int i = 0; i < nwords; i++){
    printf(FMT_WORD "\n",vaddr_read(xaddr, 4, 0xffffffff));
    xaddr += 4;
  }
  return 0;
}

static int cmd_p(char* args){
  if(args == NULL){
    Log("Miss argument!");
    return 0;
  }

  bool success = true;
  word_t result = expr(args, &success);
  if(!success){
    Log("Input expression error: can't parse");
    return 0;
  }

  if(strstr(args, "$pc") != NULL){
    printf(FMT_WORD "\n", result);
  }else{
    printf("%lu\n", result);
  }

  return 0;
}

static int cmd_w(char* args){
  if(args == NULL){
    Log("Miss argument!");
    return 0;
  }
  WP* wpointer = new_wp(args);
  assert(wpointer != NULL);
  return 0;
}

static int cmd_d(char* args){
  char * arg = strtok(NULL, " ");
  if(arg == NULL){
    Log("Missing argument!");
    return 0;
  }

  int num = atoi(arg);
  free_wp(num);

  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si", "Steps N(default: 1) instructions", cmd_si},
  {"info", "Print program status", cmd_info},
  {"x", "Scan memory, print memory contents", cmd_x},
  {"p", "Print expression value", cmd_p},
  {"w", "Set watch point", cmd_w},
  {"d", "delete watch point", cmd_d}

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
