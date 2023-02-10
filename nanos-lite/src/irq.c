#include <common.h>
// #include<stdio.h>
extern Context* schedule(Context *);
extern void do_syscall(Context* c);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:
      printf("do event yield!!!\n");
      return schedule(c);
      break;
    case EVENT_SYSCALL:
      printf("do system call!!!\n");
      do_syscall(c);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
