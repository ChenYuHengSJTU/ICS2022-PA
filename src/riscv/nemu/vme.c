#include <am.h>
#include <nemu.h>
#include <klib.h>
#include<stdint.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

// mode最高为表示是否开启虚拟内存模式
static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

// 设置页面分配和回收的回调函数
bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

// 填写内核虚拟地址映射，为恒等映射
  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  printf("end of vme init!!!\n");
  return true;
}

// 创建一个地址空间
void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {

}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

// added by me
// as->area -- 用户态范围
// ptr 空间描述符指针
// map 将地址空间中as中虚拟地址va所在的虚拟页，以prot权限
// 映射到pa所在的物理页
void map(AddrSpace *as, void *va, void *pa, int prot) {
  // printf("start of map!!!\n");
#define VPN1(va) (va >> 22)
#define VPN2(va) ((va & 0x3fffff)>>12)
  uint32_t _pa=(uint32_t)(pa);
  uint32_t _va=(uint32_t)(va);
  uint32_t **pde=(uint32_t**)(as->ptr);
  if(pde[VPN1(_va)] == NULL){
    pde[VPN1(_va)]=(uint32_t*)pgalloc_usr(PGSIZE);
  }
  uint32_t* pte=pde[VPN1(_va)];
  if(pte[VPN2(_va)]==0){
    pte[VPN2(_va)]=(_pa & (~0xfff)) | prot;
  }
  else assert(0);
  // printf("end of map!!!\n");
}

#define PGSIZE 4096
#define STACK_SIZE 8*PGSIZE

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  // kstack.start=heap.end;
  // kstack.end=kstack.start+STACK_SIZE;
  Context* context=kstack.end-sizeof(Context)-4;
  context->mepc=(uintptr_t)entry;
  // TODO cannot be fully aware of the mstatus
  context->mstatus = 0x1800;
  // context->GPRx = (uintptr_t)(heap.end);
  // context->GPRx = (uintptr_t)(kstack.end);
  // context->gpr[0] = 0;
  return context;
}
