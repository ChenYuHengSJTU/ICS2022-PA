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

#include "sdb.h"


typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[320];
  uint32_t preval;
  uint32_t curval;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

void free_wp(int NO);

int cmd_d(char *args){
  int NO;
  sscanf(args,"%d",&NO);
  free_wp(NO);
  return 0;
}

void new_wp(char* e);

int cmd_watch(char* args){
  new_wp(args);
  return 0;
}

void new_wp(char* e){
  WP* tmp=free_;
  if(free_==NULL){
    printf("WP is not enough\n");
    assert(0);
  }
  free_=free_->next;
  tmp->next=NULL;
  if(head==NULL){
    head=tmp;
  }
  else{
    WP* tmp2=head;
    while(tmp2->next!=NULL){
      tmp2=tmp2->next;
    }
    tmp2->next=tmp;
  }
  assert(strlen(e)<320);
  // int sz=strlen(e);
  // strncpy(tmp->expr,e,sz+1);
  strcpy(tmp->expr,e);
  bool flag;
  tmp->curval=expr(tmp->expr,&flag);
  assert(flag);
}

void free_wp(int NO){
  WP* tmp=head;
  if(head==NULL){
    printf("No watchpoint %d\n",NO);
    return;
  }
  if(head->NO==NO){
    head=head->next;
  }
  else {
      while(tmp->next!=NULL&&tmp->next->NO!=NO){
      tmp=tmp->next;
    }
  }

  if(tmp->next==NULL){
    printf("No watchpoint %d\n",NO);
    return;
  }
  else{
    tmp->next=tmp->next->next;
  }
  tmp->next=free_;
  free_=tmp;
  memset(tmp->expr,0,320);
  return;
}

void print_wp(){
  WP* tmp=head;
  if(head==NULL){
    printf("No watchpoints to be print\n");
    return;
  }
  while(tmp!=NULL){
    printf("Num: %d\tWhat: %s\tValue: %d\n",tmp->NO,tmp->expr,tmp->curval);

    // bool flag;
    // uint32_t res=expr(tmp->expr,&flag);
    // if(flag)
    //   printf("Num: %d\tWhat: %s\tValue: %d\n",tmp->NO,tmp->expr,res);
    // else{
    //   printf("Some error occurs while calculating the value\n");
    //   assert(0);
    tmp=tmp->next;
  }
  return;
}

bool traverse_wp(){
  WP* tmp=head;
  int val;
  bool flag=true,flag2=true;
  while(tmp!=NULL){
    val=expr(tmp->expr,&flag);
    assert(flag);
    if(tmp->curval!=val) flag2=false;
    printf("%d\tPreval:%d\tCurval:%d\n",tmp->NO,tmp->curval,val);
    tmp->curval=val;
    tmp=tmp->next;
  }  
  return flag2;
} 

/* TODO: Implement the functionality of watchpoint */

