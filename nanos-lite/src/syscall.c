#include <common.h>
#include "syscall.h"
#include<stdint.h>
#include<proc.h>

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);
extern void naive_uload(PCB *pcb, const char *filename);

struct timeval{
  long int tv_sec;
  long int tv_usec;
}tv;

struct timezone{
  int tz_minuteswest;
  int tz_dsttime;
}tz;

void sys_yield(Context *c){
  yield();
  c->GPRx=0;
}

void sys_exit(Context *c){
  halt(c->GPRx);
}

extern int mm_brk(uintptr_t brk);
// extern int sbrk(void* addr);
void sys_brk(Context *c){
  // uintptr_t addr = (uintptr_t)(c->GPR2);
  // sbrk((void*)addr);
  // c->GPRx = mm_brk(addr);
  // printf("sys_brk!!!\n");
  c->GPRx=0;
}

void gettimeofday(struct timeval* tv,struct timezone* tz){
  uint64_t uptime=0;
  ioe_read(AM_TIMER_UPTIME, &uptime);
  tv->tv_usec = (int32_t)uptime;
  tv->tv_sec = (int32_t)uptime / 1000000;
  return;
}

void sys_gettimeofday(Context *c){
  gettimeofday((struct timeval*)(c->GPR2),(struct timezone*)(c->GPR3));
  c->GPRx=0;
}

void sys_read(Context *c){
  int res=fs_read(c->GPR2,(void*)(c->GPR3),c->GPR4);
  c->GPRx=res;
}

void sys_open(Context *c){
  int res=fs_open((char*)(c->GPR2),c->GPR3,c->GPR4);
  c->GPRx=res;
}

void sys_write(Context *c){
  int res=fs_write(c->GPR2,(void*)(c->GPR3),c->GPR4);
  c->GPRx=res;
}

void sys_lseek(Context *c){
  int res=fs_lseek(c->GPR2,c->GPR3,c->GPR4);
  c->GPRx=res;
}

void sys_close(Context *c){
  int res=fs_close(c->GPR2);
  c->GPRx=res;
}

extern int execve(const char *pathname, char *const argv[], char *const envp[]);
void sys_execve(Context *c){
  c->GPRx=execve((char*)c->GPR2,(char**)c->GPR3,(char**)c->GPR4);
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  
  switch (a[0]) {
    case SYS_yield:
      printf("sys_yield!!!-->\n");
      sys_yield(c);
      break;
    case SYS_exit:
      printf("sys exit!!!-->\n");
      sys_exit(c);
      break;
    case SYS_brk:
      sys_brk(c);
      // printf("sys brk!!!-->\n");
      break;
    case SYS_gettimeofday:
      sys_gettimeofday(c);
      // printf("sys gettimeofday!!! -->\n");
      break;
    case SYS_read:
      sys_read(c);
      // printf("sys read!!! -->\n");
      break;
    case SYS_open:
      sys_open(c);
      // printf("sys open!!! -->\n");
      break;
    case SYS_close:
      sys_close(c);
      // printf("sys close!!! -->\n");
      break;
    case SYS_write:
      sys_write(c);
      // printf("sys write!!! -->\n");
      break;
    case SYS_lseek:
      sys_lseek(c);
      // printf("sys lseek!!! -->\n");
      break;
    case SYS_execve:
      printf("sys execve!!!\n");
      sys_execve(c);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

