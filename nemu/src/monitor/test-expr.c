#include<stdlib.h>
#include<stdio.h>
#include"./sdb/sdb.h"

int mian(){

  FILE* file=fopen("../tools/gen-expr/input","r+");
  char e[65535]={};
  int res=-1;
  int fres=fscanf(file,"%d",&res);
  if(fres){}
  fres=fscanf(file,"%s",e);
  bool flag=-1;
  int ans=expr(e,&flag);
  if(ans!=res){
    printf("ERROR IN CALCULATION!!!\n");
  }
    return 0;
}