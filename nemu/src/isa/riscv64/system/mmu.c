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
#include <memory/vaddr.h>
#include <memory/paddr.h>

// page table entry macros
#define PTE_V  1ul << 0
#define PTE_R  1ul << 1
#define PTE_W  1ul << 2
#define PTE_X  1ul << 3
#define PTE_U  1ul << 4
#define PTE_G  1ul << 5
#define PTE_A  1ul << 6
#define PTE_D  1ul << 7

// translation use
#define OFFSET(addr) (addr & 0x0fff)
#define VA_VPN0(va) ((va >> 12) & 0x00000000000001ff)
#define VA_VPN1(va) ((va >> 21) & 0x00000000000001ff)
#define VA_VPN2(va) ((va >> 30) & 0x00000000000001ff) 
#define PA2PTE(addr) (((addr >> 12) & 0xfffffffffff) << 10)
#define PTE2PA(pte) (((pte >> 10) & 0xfffffffffff) << 12)
#define PGROUNDDOWN(addr) ((addr) & (~((1ull << 12) - 1)))

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) { 
  printf("vaddr: 0x%lx  vaddr + len: 0x%lx\n", vaddr, vaddr + len);
  assert(PGROUNDDOWN(vaddr) == PGROUNDDOWN(vaddr + len));
  uint64_t offset = vaddr & 0x0000000000000fff;
  uint64_t root = cpu.csr.satp << 12;
  
  uint32_t va_vpn2 = VA_VPN2(vaddr);
  uint32_t va_vpn1 = VA_VPN1(vaddr);
  uint32_t va_vpn0 = VA_VPN0(vaddr);

  uint64_t pte2 = paddr_read(root + va_vpn2 * 8, 8);
  assert((pte2 & PTE_V) != 0);
  
  uint64_t pte1 = paddr_read((PTE2PA(pte2)) + va_vpn1 * 8, 8);
  assert((pte1 & PTE_V) != 0);

  uint64_t pte0 = paddr_read((PTE2PA(pte1)) + va_vpn0 * 8, 8);
  assert((pte0 & PTE_V) != 0);

  uint64_t paddr = PTE2PA(pte0) | offset;
  
  assert(vaddr == paddr);
  return paddr;
}
