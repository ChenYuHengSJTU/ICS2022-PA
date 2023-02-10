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
#include <memory/vaddr.h>

#define MEPC 0 //0x341
#define MSTATUS 1 //0x300
#define MCAUSE 2 //0x342
#define MTVEC 3 // 0x305
#define SATP 4 //0x180
// extern uint32_t CSR[5];

// 使用assertion检查页目录项和页表项的present/valid位
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  // printf("start translate!!!\n");
#define SATP_MASK 0x3fffff
#define SHIFT 12
#define VPN1(va) (va>>22)
#define VPN2(va) ((va & 0x3fffff) >> 12)
#define OFFSET(va) (va & 0xfff)
  uint32_t va=(uint32_t)(vaddr);
  paddr_t *pde=(paddr_t*)(guest_to_host((paddr_t)(cpu.CSR[SATP]<<SHIFT)));
  // printf("pde is at %p\n",pde);
  assert(pde!=NULL);
  paddr_t *pte=(paddr_t*)(guest_to_host(((paddr_t)pde[VPN1(va)])));
  // printf("pte is at %p\n",pte);
  assert(pte!=NULL);
  paddr_t paddr=(paddr_t)((pte[VPN2(va)]&(~0xfff))|OFFSET(va));
  // printf("paddr is at %u\n",paddr);
  assert(vaddr>=0x40000000 && vaddr <= 0xa1200000);
  assert(vaddr==paddr);
  // printf("end of translation!!\n");
  return paddr;
}
