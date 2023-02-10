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

#ifndef __ISA_RISCV32_H__
#define __ISA_RISCV32_H__

#include <common.h>
#include <isa.h>

typedef struct {
  word_t gpr[32];
  vaddr_t pc;
  word_t CSR[6];
} riscv32_CPU_state;



// decode
typedef struct {
  union {
    uint32_t val;
  } inst;
} riscv32_ISADecodeInfo;



#define MEPC 0 //0x341
#define MSTATUS 1 //0x300
#define MCAUSE 2 //0x342
#define MTVEC 3 // 0x305
#define SATP 4 //0x180
// extern uint32_t CSR[5];

#define isa_mmu_check(vaddr, len, type) (cpu.CSR[SATP] >> 31)

#endif
