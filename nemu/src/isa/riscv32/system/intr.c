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

// #include <isa-def.h>
#include<isa.h>
#include<assert.h>
// // for intr
// enum{
//   MEPC=0, //0x341
//   MSTATUS=1, //0x300
//   MCAUSE=2, //0x342
//   MTVEC=3 ,// 0x305
//   SATP=4 //0x180
// };
// static uint32_t CSR[5]={0,0x1800,0x08,0,0};
// extern uint32_t CSR[5];

#define MEPC 0 //0x341
#define MSTATUS 1 //0x300
#define MCAUSE 2 //0x342
#define MTVEC 3 // 0x305
#define SATP 4 //0x180
// extern uint32_t CSR[5];



word_t* get_csr_value(word_t imm){
  switch(imm){
    case 0x341:
      return &(cpu.CSR[MEPC]);
    case 0x300:
      cpu.CSR[MSTATUS]|=0x1800;
      return &(cpu.CSR[MSTATUS]);
    case 0x342:
      // assert(CSR[MCAUSE]==0x1800);
      // CSR[MCAUSE]=0x0b;
      // printf("CSR:0x%02x\n",CSR[2]);
      return &(cpu.CSR[MCAUSE]);
    case 0x305:
      return &(cpu.CSR[MTVEC]);
    case 0x180:
      return &(cpu.CSR[SATP]);
  }
  assert(0);
  return NULL;
}

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.CSR[MCAUSE]=NO;
  // 当前指令还是下一条指令？
  cpu.CSR[MEPC]=epc;
  // printf("mtvec:0x%02x\n",CSR[MTVEC]);
  return cpu.CSR[MTVEC];
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
