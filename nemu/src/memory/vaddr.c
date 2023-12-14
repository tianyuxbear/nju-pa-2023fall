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
#include <memory/paddr.h>

static char rtracebuf[128], wtracebuf[128];

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  word_t ret = paddr_read(addr, len);
  memset(rtracebuf, 0, sizeof(rtracebuf));
  snprintf(rtracebuf, sizeof(rtracebuf), "mem read  ===> addr: " FMT_WORD "    len: %d    read value: %lu", addr, len, ret);
  IFDEF(CONFIG_MTRACE, puts(rtracebuf));
#ifdef CONFIG_MTRACE_COND
  if(MTRACE_COND) {log_write("%s\n", rtracebuf);}
#endif
  return ret;
}

void vaddr_write(vaddr_t addr, int len, word_t data) {  
  paddr_write(addr, len, data);
  memset(wtracebuf, 0, sizeof(wtracebuf));
  snprintf(wtracebuf, sizeof(wtracebuf), "mem write ===> addr: " FMT_WORD "    len: %d    write data: %lu", addr, len, data);
  IFDEF(CONFIG_MTRACE, puts(wtracebuf));
  if(addr == 0x80002a58) {
    printf("mem write ===> addr: " FMT_WORD "    len: %d    write data: 0x%016lx\n", addr, len, data);
    nemu_state.state = NEMU_ABORT;
  }
#ifdef CONFIG_MTRACE_COND
  if(MTRACE_COND) {log_write("%s\n", wtracebuf);}
#endif
}
