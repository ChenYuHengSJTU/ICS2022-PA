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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include<stdbool.h>
// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static void gen_rand_expr() {
  buf[0] = '\0';
  // 使用memset后解决问题
  memset(buf,0,65536);
  // FILE* file=fopen("./inputs.txt","w+");
  // srand((unsigned)time(NULL));
  int a=0;
  int flag=0;
  int len=31,position=0,preop=0;
  char num[20]="";
   for(int i=0;i<len;++i){
    if(i%2){
      a=rand()%4;
      a=abs(a);
      preop=a;
      switch(a){
        case 0:
          buf[position++]='+';
          break;
        case 1:
          buf[position++]='-';
          break;
        case 2:
          buf[position++]='*';
          break;
        case 3:
          buf[position++]='/';
          break;
        }
        // printf("a:%d\n",a);
        a=abs(rand())%2;
        if(a){
          buf[position++]='(';
          flag++;
          i+=2;
        }
      }
      else{
        num[0]='\0';
        a=abs(rand());
        a%=50;
        if(preop==3&&a==0){
          while((a=abs(rand())%100)==0){
          }
        }
        int b=abs(rand())%4;
        sprintf(num,"%d",a);
        strcat(buf,num);
        position+=strlen(num);

        a=abs(rand())%3;
        if(a==0){
          if(flag){
            flag--;
            buf[position++]=')';
          }
        }
        

        // printf("Num: %s\t",num);
        // if(b){
        //   sprintf(num,"%d",a);
        //   strcat(buf,num);
        //   position+=strlen(num);
        // }
        // else{
        //   num[0]='0';
        //   num[1]='x';
        //   sprintf(num+2,"%x",a);
        //   strcat(buf,num);
        //   position+=strlen(num);
        // }
      }
    }
    while(flag){
      flag--;
      buf[position++]=')';
    }
    buf[position++]='\0';
    //printf("Buf:%s\n",buf);
    // fputs(buf,file); 
    // fclose(file);
    
        // printf('\n');
}

int main(int argc, char *argv[]) {
  // int seed = time(0);

  srand((unsigned)time(NULL));
  // srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();
    //printf("buf:%s\n",buf);
    sprintf(code_buf, code_format, buf);
    //printf("code_buf:%s\n",code_buf);
    FILE *fp = fopen("/tmp/.code.c", "w+");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    int res=fscanf(fp, "%d", &result);
    printf("%d",res);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}