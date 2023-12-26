#include <common.h>

void do_syscall(Context *c);

extern Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:
      printf("=== Trigger yield event ===\n");
      yield();
      //c = schedule(c);
      break;
    case EVENT_SYSCALL:
      printf("=== Trigger syscall event ===\n");
      do_syscall(c);
      break;
    default: 
      panic("Unhandled event ID = %d", e.event);
      break;
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
