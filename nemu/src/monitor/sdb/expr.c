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

// #include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include<string.h>
#include<assert.h>
#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <memory/paddr.h>
enum {
  TK_NOTYPE = 256, TK_EQ=0,
  NUM=1,
  REGISTER=2,
  HEX=3,
  EQ=4,
  NOTEQ=5,
  OR=6,
  AND=7,
  NOT=8,
  PLUS=9,
  MINUS=10,
  MUL=11,
  DIV=12,
  OPEN=13,
  CLOSE=14,
  DEREF=15
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  // 需要注意元字符需要转义，但同样需要考虑C语言中的转义，\\ = \ ，所以有两个\ ；
  {" +", TK_NOTYPE},    // spaces
  {"\\+", PLUS},         // plus
  {"\\-", MINUS},
  {"\\*", MUL},
  {"\\/", DIV},
  // 
  {"\\(", OPEN},
  {"\\)", CLOSE},
  {"\\|\\|", OR},
  {"&&" ,AND},
  {"!", NOT},
  // 
  {"[0-9]+" ,NUM},

  // 
  {"\\$[a-z]+" ,REGISTER},
  // {"\\*"}
  {"\\0[xX][0-9a-fA-F]+", HEX},


  {"==", TK_EQ},        // equal
  {"!=", NOTEQ}
};

// #define NR_REGEX ARRLEN(rules)
enum{ 
  NR_REGEX=sizeof(rules)/sizeof(struct rule)
};

static regex_t re[NR_REGEX]={};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      //panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  int pretoken=256;
  //bool pre=false;
  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        // char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        // 必须清空str，避免溢出
        if(rules[i].token_type==TK_NOTYPE) break;
        tokens[nr_token].type=rules[i].token_type;
        
        // if(pretoken==11&&(tokens[nr_token].type==NUM||tokens[nr_token].type==HEX)){
        //   nr_token--;
        //   tokens[nr_token].type=DEREF;
        // }

        if(tokens[nr_token].type==MUL&&(pretoken==TK_NOTYPE||pretoken==MUL||pretoken==DIV||pretoken==PLUS||pretoken==EQ||pretoken==NOTEQ||pretoken==AND||pretoken==OR||pretoken==NOT||pretoken==OPEN)){
          //pre=true;
          pretoken=DEREF;
          break;
        }

        if(pretoken==DEREF){
          tokens[nr_token].type=DEREF;
        }
        // 需要记录数据内容的type
        if(rules[i].token_type==NUM||rules[i].token_type==REGISTER||rules[i].token_type==HEX||tokens[nr_token].type==DEREF){
          strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
        }
        pretoken=tokens[nr_token].type;
        nr_token++;
        // switch (rules[i].token_type) {
        //   default: //TODO();
        // }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

// 用于判断表达式是否被一对匹配的括号包围着
bool check_parentheses(int l,int r){
  int i=l,p=0,q=0;
  if(tokens[l].type==OPEN&&tokens[r].type==CLOSE){
    for(;i<=r;++i){
      if(tokens[i].type==OPEN) p++;
      else if(tokens[i].type==CLOSE) q++;
      if(i!=r&&q>=p) return false;
    }
    return p==q;
  }
  return false;
}

//TODO 考虑：十六进制数值转化，递归进行求值

u_int32_t hex_to_decimal(char* str){
  int len=strlen(str),res=0,base=1<<(len<<2);
  for(int i=0;i<len;++i){
    if(str[i]>='0'&&str[i]<='9'){
      res+=base*(str[i]-'0');
    }
    else if(str[i]>='a'&&str[i]<='f'){
      res+=base*(str[i]-'a');
    }
    else if(str[i]>='A'&&str[i]<='F'){
      res+=base*(str[i]-'A');
    }
    base>>=4;
  }
  return res;
}

u_int32_t get_value(int pos){
  switch(tokens[pos].type){
    case NUM:
      return atoi(tokens[pos].str);
    case HEX:
      return hex_to_decimal(tokens[pos].str);
    case REGISTER:
      bool flag;
      u_int32_t res=isa_reg_str2val(tokens[pos].str,&flag);
      if(flag) return res;
      else {
        printf("READ REG FAIL!!!\n");
        assert(0);
      }
    case DEREF:
      return paddr_read(atoi(tokens[pos].str),4);
  }
  return 0;
}

u_int32_t calculate_aux(u_int32_t val1,u_int32_t val2,u_int32_t pos){
  int t=tokens[pos].type;
  // OR=6,AND=7,NOT=8,PLUS=9,MINUS=10,MUL=11,DIV=12,OPEN=13,CLOSE=14,
  switch(t){
    case 4:
      return val1==val2;
    case 5:
      return val1!=val2;
    case 6:
      return val1|val2;
    case 7:
      return val1&val2;
    case 8:
      return !val1;
    case 9:
      return val1+val2;
    case 10:
      return val1-val2;
    case 11:
      return val1*val2;
    case 12:
      if(val2==0){
        printf("DIVIDED BY ZERO!!!");
        assert(0);
      }
      else return val1/val2;
    default:
      printf("DEFAULT CASR???\n");
      assert(0);
      return 0;
  }
}

bool check(int l,int r){
  int flag=0;
  for(int i=l;i<=r;++i){
    if(tokens[i].type==OPEN){
      flag++;
    }
    else if(tokens[i].type==CLOSE){
      if(flag==0) return false;
      flag--;
    }
  }
  return true;
}

u_int32_t dominant(int* l,int* r){
  int ll=*l,rr=*r;
  if(ll>rr){
    printf("ERROR OCCURS IN dominant!!!\n");
    assert(0);
  } 
  if(ll==rr) return -1;
  while(tokens[*l].type==OPEN&&tokens[*r].type==CLOSE){
    if(check(*l+1,*r-1)){
      (*l)++;
      (*r)--;
    }
    else break;
  }
  int position=*l;
  ll=*l,rr=*r; 
  int flag=0; // 记录是否在括号中,但是需要考虑（long expr）的情形
  for(int i=ll;i<=rr;++i){
    if(tokens[i].type==OPEN) flag++;
    if(tokens[i].type==CLOSE) flag--;
    if(flag) continue;
    if(tokens[i].type==PLUS||tokens[i].type==MINUS) position=i;
    if((tokens[i].type==MUL||tokens[i].type==DIV)&&(tokens[position].type==MUL||tokens[position].type==DIV||tokens[position].type==NUM||tokens[position].type==OPEN)) position=i;
    //if(tokens[position].type==OPEN) position=i;
  } 
  return (position==ll)?-1:position;
}

u_int32_t calculate(int l,int r){
  u_int32_t pos=dominant(&l,&r);
  if(pos==-1){
    return get_value(l);
  }
  u_int32_t val1,val2;
  if(pos==l){
    val1=calculate(l,l);
  }
  else val1=calculate(l,pos-1);
  if(pos==r){
    val2=calculate(r,r);
  }
  else val2=calculate(pos+1,r);
  return calculate_aux(val1,val2,pos);
}

u_int32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  // printf("Make Toke!!!\nnr_token:%d\n",nr_token-1);
  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  int res=calculate(0,nr_token-1);
  // printf("the res is %d\n",res);

  *success=true;
  return res;
}