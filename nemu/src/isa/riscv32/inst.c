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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S,TYPE_R,TYPE_B,TYPE_J,
  TYPE_N,TYPE_SH // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)

// by me
#define immB() do {*imm = (SEXT(BITS(i,31,31),1)<<12)| ((BITS(i, 11,8))<<1)|((BITS(i,7,7))<<11)|((BITS(i,30,25))<<5);} while(0)
#define immJ() do {*imm = (SEXT(BITS(i,31,31),1)<<20)|(BITS(i,30,21)<<1)|(BITS(i,20,20)<<11)|(BITS(i,19,12)<<12);} while(0)

uint32_t s1=0xffff0000;
uint32_t ss[33]={0,
                0x80000000,0xc0000000,0xe0000000,0xf0000000,
                0xf8000000,0xfc000000,0xfe000000,0xff000000,
                0xff800000,0xffc00000,0xffe00000,0xfff00000,
                0xfff80000,0xfffc0000,0xfffe0000,0xffff0000,
                0xffff8000,0xffffc000,0xffffe000,0xfffff000,
                0xfffff800,0xfffffc00,0xfffffe00,0xffffff00,
                // 0xffffff80,0xfffffc00,0xfffffe00,0xffffff00,
                0xffffff80,0xffffffc0,0xffffffe0,0xfffffff0,
                0xfffffff8,0xfffffffc,0xfffffffe,0xffffffff};




static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_SH:src1R(); src2R(); *imm=rs2;break;
  }
}

void JAL_AUX(int dest,word_t imm,Decode* s){
  R(dest)=cpu.pc+4;
  s->dnpc=cpu.pc+imm;
  // cpu.pc+=imm;
  // printf("FROM PC %02x JAL to PC:%02x\n",cpu.pc,cpu.pc+imm);
}

void JALR_AUX(int dest,int src1,word_t imm,Decode* s){
  int tmp=cpu.pc+4;
  s->dnpc=(src1+imm)&(~1);
  R(dest)=tmp;
  // printf("FROM PC %02x JALR to PC:%02x\n",cpu.pc,s->dnpc);
}

void B_AUX(bool flag,word_t imm,Decode* s){
  if(flag){
    s->dnpc=cpu.pc+imm;
    // printf("FROM PC %02x JUMP to PC:%02x\t\t\tImm is:%d\n",cpu.pc,s->dnpc,imm);
  }
}

// TODO here attention!!!
void mul_h(int dest,word_t src1,word_t src2){
  // printf("0x%02x * 0x%02x\n",src1,src2);
  uint64_t tmp1=abs(src1),tmp2=abs(src2);
  if(((int)(src1)<=0&&(int)(src2)<=0)||((int)(src1)>=0&&(int)(src2)>=0)){
    R(dest)=(tmp1*tmp2)>>32;
  }
  else{
    R(dest)=-(tmp1*tmp2)>>32;
  }
}

// 需要实现intr，并在ecall_aux中调用之
void ECALL_AUX(Decode* s){
  // printf("ecall:\n");
  // isa_reg_display();
  // printf("\n0x%02x\n",Mr(0x80000342,4));
  // word_t* tmp=get_csr_value(0x305);
  // *tmp=0x800005a0;
  s->dnpc=isa_raise_intr(0x0b,cpu.pc);
  // printf("ecall to:0x%02x\n",s->dnpc);
}


void CSR_AUX(word_t imm,word_t src1,int dest,int fcode){
  word_t* tmp=get_csr_value(imm);
  switch(fcode){
    case 0:
      *tmp=src1;
      R(dest)=*tmp;
      break;
    case 1:
      // printf("hit here!!!\nmstatus:%02x\tsrc1:%02x\timm:%02x\n",*tmp,src1,imm);
      int t=((*tmp)|src1);
      *tmp=t;
      R(dest)=*tmp;
      break;
  }
  return;
}

void MRET_AUX(Decode* s){
  s->dnpc=*get_csr_value(0x341);
  // printf("hit mret with ret:0x%02x\n",s->dnpc);
}

static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  // INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);
  // INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = Mr(src1 + imm, 4));
  // INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  
  // INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  // INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));

  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);
  // AUIPC
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = cpu.pc+imm);

  // BEG... 注意符号位扩展
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, B_AUX(src1==src2,imm,s));

  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, B_AUX(src1!=src2,imm,s));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, B_AUX((int)(src1)<(int)(src2),imm,s));
  // bgz blez
  INSTPAT("??????? ????? ????? 101 ????? 11000 11",bge     , B, B_AUX((int)(src1)>=(int)(src2),imm,s));
  // bltu
  INSTPAT("??????? ????? ????? 110 ????? 11000 11",bltu     , B, B_AUX(src1<src2,imm,s));
  // bgeu
  INSTPAT("??????? ????? ????? 111 ????? 11000 11",bgeu     , B, B_AUX(src1>=src2,imm,s));

  //L 
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(dest) =Mr(src1+imm,1));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(dest) =Mr(src1+imm,2)|((Mr(src1+imm,2)>>15)?s1:0));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = Mr(src1 + imm, 4));

  // lbu
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(dest) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(dest) = Mr(src1 + imm, 2));

// sb
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  // sh
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm ,2, src2));
  
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  // INSTPAT("")
  // addi
  INSTPAT("??????? ????? ????? 000 ????? 00100 11",addi    , I, R(dest) = src1 + imm);
  // INSTPAT("??????? ????? ????? 000 ????? 00100 11",li      , I, R(dest) = src1 + imm);

  // add
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11",add     , R, R(dest) = src1 + src2);

  // sub neg
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11",sub     , R, R(dest) = src1 - src2);

    // sltu
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11",sltu    , R , R(dest) = (src1 < src2));

  // and
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R , R(dest) = src1&src2);

  // andi
  INSTPAT("??????? ????? ????? 111 ????? 00100 11",andi    , I, R(dest) = src1&imm);

  // xor
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11",xor      , R, R(dest) = src1 ^ src2);
  // xori not
  INSTPAT("??????? ????? ????? 100 ????? 00100 11",xori     , I ,R(dest) = src1 ^ imm);

  // or
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11",or       , R, R(dest) = src1 | src2);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11",ori       , I, R(dest) = src1 | imm);

  // sll
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11",sll      , R, R(dest) = (src1<<src2));

  // slli
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11",slli     ,SH ,R(dest)=(src1<<imm));
  // srl
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11",srl      , R ,R(dest)=(src1>>(src2&0x1f)));
  // srli
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11",srli      , SH ,R(dest)=(src1>>imm));
  // slt
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11",slt      , R, R(dest) = ((signed)src1<(signed)src2));
  // SLTIU
  INSTPAT("??????? ????? ????? 011 ????? 00100 11",sltiu   , I, R(dest) = (src1<imm));

  // mul
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11",mul     , R, R(dest) = src1 * src2);
  // mulh
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11",mulh    ,R ,mul_h(dest,src1,src2));
  // INSTPAT("0000001 ????? ????? 001 ????? 01100 11",mulh    ,R ,R(dest)=(((int64_t)(src1)*(int64_t)(src2))>>32););
  // div
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11",div     , R, R(dest) = (int)(src1)/(int)(src2));
  // divu
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11",divu    , R, R(dest) = src1/src2);

  // rem
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11",rem     , R, R(dest) = src1%src2);
  // remu
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11",remu    , R, R(dest) = src1%src2);

  // sra
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11",sra    , R, R(dest) = (((signed)(src1) >> src2)));
  // srai
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11",srai    , SH, R(dest) = ((signed)(src1) >> imm));

  // jal
  INSTPAT("??????? ????? ????? ??? ????? 11011 11",jal     , J, JAL_AUX(dest,imm,s));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11",jalr    , I, JALR_AUX(dest,src1,imm,s));

// 80000614:	30200073   0011000 00010 00000 000 00000 11100 11       	mret
  INSTPAT("0011000 00010 00000 000 00000 11100 11",mret    ,N,MRET_AUX(s));
  // ret
  // INSTPAT("??????? ????? ????? 111 ????? 11001 11",ret     , J, JALR_AUX(dest,src1,imm));
  
  // ecall
  INSTPAT("0000000 00000 00000 000 00000 11100 11",ecall   , I,ECALL_AUX(s));
  // mret

  // CSRRW  80000610:	305 79073  0011000 00101 01111 001 00000 11100 11        	csrw	mtvec,a5  csrrw
  // 800006cc:	300 31073          	csrw	mstatus,t1
  // 800006e4:	341 39073          	csrw	mepc,t2
//   800006ac:	342022f3   0011010 00010 00000 010 00101 1110011       	csrr	t0,mcause  csrrs
// 800006b0:	30002373     0011000 00000 00000 010 00110 1110011      	csrr	t1,mstatus
// 800006b4:	341023f3          	csrr	t2,mepc
  INSTPAT("??????? ????? ????? 001 ????? 11100 11",csrw   , I, CSR_AUX(imm,src1,dest,0));
  INSTPAT("??????? ????? ????? 010 ????? 11100 11",csrr   , I, CSR_AUX(imm,src1,dest,1));

// 
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  
  INSTPAT_END();


  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
