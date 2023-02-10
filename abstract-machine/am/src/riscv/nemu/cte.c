#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
// #include<proc.h>

static Context* (*user_handler)(Event, Context*) = NULL;

void _isa_reg_display(Context *c) {
  for(int i=0;i<32;++i){
    printf("%d:%d\t",i,*(uint32_t*)(c+i*4));
    if((i%4-3)==0) printf("\n");
  }
}
// do_event

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
      switch (c->mcause) {
      // case 0xffffffff:
      //   ev.event=EVENT_YIELD;
      //   printf("handled!!! 1\n");
      //   break;
        case 8:
        case 0x0b:
          if(c->GPR1==0xffffffff)
            ev.event=EVENT_YIELD;
          else{
            ev.event=EVENT_SYSCALL;
            // printf("event sys call!!!\n");
          }
          // printf("handled!!!\n");
          // _isa_reg_display(c);
          c->mepc+=4;
          break;
        // default: ev.event = EVENT_YIELD; break;
        default: ev.event = EVENT_ERROR; break;
      }
    // assert(c->mepc >= 0x40000000 && c->mepc <= 0x88000000);

    // do_event
    c = user_handler(ev, c);
    // assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  // why -4
  uintptr_t *t0_buf = kstack.end;
  *t0_buf = 0;

  Context *context = kstack.end - sizeof(Context);
  context->mepc=(uintptr_t)(entry);
  context->gpr[10]=(uintptr_t)(arg);
  context->mstatus = 0x1800 | 0x80;
  // context->mstatus = 0x1800 | 0x80;
  // context->mepc    = (uintptr_t)entry;
  // context->gpr[10] = (uintptr_t)arg;
  context->gpr[10]=(uintptr_t)context;
  context->gpr[2] =(uintptr_t)kstack.end;
  // context->pdir    = NULL;
  //为了Real VME
  // context->np      = 3;
  // context->gpr[2]  = (uintptr_t)kstack.end - 4;
  //TODO: 还需要添加一些
  return context;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
