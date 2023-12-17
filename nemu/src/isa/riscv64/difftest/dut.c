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
#include <cpu/difftest.h>
#include "../local-include/reg.h"

#define NR_GPR MUXDEF(CONFIG_RVE, 16, 32)

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {

  for(int i = 0; i < NR_GPR; i++){
    if(i == 5) continue;
    if(cpu.gpr[i] != ref_r->gpr[i]){
      Log("difftest fail at 0x%016lx ==> for %s, expect %lu, but get %lu", pc, regs[i], cpu.gpr[i], ref_r->gpr[i]);
      return false;
    }
  }

  if(cpu.csr.mstatus != ref_r->csr.mstatus){
    Log("difftest fail at 0x%016lx ==> for %s, expect %lu, but get %lu", pc, "mstatus", cpu.csr.mstatus, ref_r->csr.mstatus);
    return false;
  }

  // if(cpu.csr.mcause != ref_r->csr.mcause){
  //   Log("difftest fail at 0x%016lx ==> for %s, expect %lu, but get %lu", pc, "mcause", cpu.csr.mcause, ref_r->csr.mcause);
  //   return false;
  // }

  if(cpu.csr.mtvec != ref_r->csr.mtvec){
    Log("difftest fail at 0x%016lx ==> for %s, expect %lu, but get %lu", pc, "mtvec", cpu.csr.mtvec, ref_r->csr.mtvec);
    return false;
  }

  if(cpu.csr.mepc != ref_r->csr.mepc){
    Log("difftest fail at 0x%016lx ==> for %s, expect %lu, but get %lu", pc, "mepc", cpu.csr.mepc, ref_r->csr.mepc);
    return false;
  }

  if(cpu.pc != ref_r->pc){
    Log("difftest fail at 0x%016lx ==> for %s, expect %lu, but get %lu", pc, "pc", cpu.pc, ref_r->pc);
    return false;
  }

  return true;
}

void isa_difftest_attach() {
}
