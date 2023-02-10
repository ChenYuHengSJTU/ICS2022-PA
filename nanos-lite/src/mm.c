#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *old_pf = pf;
  pf = (void *)((char *)pf + nr_page * PGSIZE);
  return old_pf;
}

// added by me
#ifdef HAS_VME
static void* pg_alloc(int n) {
  #define PAGE_SIZE 4096
  void* start=new_page(n/PAGE_SIZE);
  memset(start,0,n);
  return start;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  // _sbrk(brk);
  return 0;
}

// 初始化工作，将TRM提供的堆区起始地址作为空闲物理页的首地址
void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
