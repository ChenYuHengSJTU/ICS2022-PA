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

const char *Regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

uint32_t reg_tmp[32]={0},reg_ref[32]={};
uint32_t prepc=0x80000000;
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc){
  bool flag=true;
  // printf("called!\n");
  for(int i=1;i<32;++i){
    // if(i==10||i==2) continue;
    if(gpr(i)!=ref_r->gpr[i]){
        flag=false;
        printf("\nreg %s do not match!\nDut:0x%02x\tRef:0x%02x\tPre Dut:0x%02x\tPre Ref:0x%02x\n",Regs[i],gpr(i),ref_r->gpr[i],reg_tmp[i],reg_ref[i]);
        printf("DutPC:0x%02x\tRefPC:0x%02x\n\n",prepc,pc);
        // return false;
    }
    reg_tmp[i]=gpr(i);
    reg_ref[i]=ref_r->gpr[i];
  }
  // if(pc!=cpu.pc){
  //   printf("PC does not match!!!\nDut: %02x\tRef: %02x\n",cpu.pc,pc);
  //   return false;
  // }
  // printf("DutPc:0x%02x\tRefPc:0x%02x\n",cpu.pc,pc);
  prepc=cpu.pc;
  return flag;
}


void isa_difftest_attach() {
}
