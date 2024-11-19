#include "sched/sched.h"
#include "libc/string.h"
#include "mem/fixed_alloc.h"
#include "vid/term.h"

static scheduler_t scheduler;
// Dispatch a new process while running within an interrupt routine.
static void dispatch_interrupt(uint32_t stack_loc);
static void sched_interrupt_store(unsigned int idx, uint32_t stack_loc);
static void sched_interrupt_replace(unsigned int idx, uint32_t stack_loc);

// Define the round-robin time slice.
#define TIME_SLICE_CONSTANT 4

// Initialize the scheduler.
int sched_init() {
  memset(&scheduler, 0, sizeof(scheduler));
  return 0;
}

void sched_admit(uint32_t eip) {
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (scheduler.process_table[i].state == PROCESS_UNUSED) {
      scheduler.process_table[i].eip = eip;

      // Allocate some stack for this process. TODO: Hope this doesn't fail!
      uint32_t stack = (uint32_t) fixed_alloc();
      scheduler.process_table[i].register_ctx.ebp = stack;
      scheduler.process_table[i].register_ctx.esp = stack;

      uint32_t eflags;
      asm volatile ("pushfl;\
                                    popl %%eax;       \
                                    movl %%eax, %0;"  \
                                    :"=m" (eflags)     \
                                    ); 

      scheduler.process_table[i].register_ctx.EFLAGS = eflags;
      scheduler.process_table[i].register_ctx.cs = 0x8; // TODO: fix this later.
      scheduler.process_table[i].state = PROCESS_READY;
      return;
    }
  }
}

// Handle an interrupt.
void sched_interrupt(uint32_t counter, uint32_t stack_loc) {
  if (counter % TIME_SLICE_CONSTANT == 0) {
    dispatch_interrupt(stack_loc);
  }
}

// Dispatch a new process while running within an interrupt routine.
static void dispatch_interrupt(uint32_t stack_loc) {
  int cur = -1;
  for (int i = 0; i < MAX_PROCESSES; i++) {
    if (scheduler.process_table[i].state == PROCESS_RUNNING) {
      cur = i;
      break;
    }
  }

  if (cur == -1) {
    // If we couldn't find a process that is running, we're at program startup.
    // Throw the process at '0' on the stack.
    sched_interrupt_replace(0, stack_loc);
    scheduler.process_table[0].state = PROCESS_RUNNING;
    uint32_t eip = *(uint32_t *)(stack_loc + 4);
    term_format("UNABLE TO FIND RUNNING PROCESS; EIP: %x\n", &eip);
    return;
  }

  for (int i = 0; i < MAX_PROCESSES; i++) {
    // Start at the last process, searching for something that is
    // 'READY' and not blocked.
    int idx = (cur + i) % MAX_PROCESSES;
    if (scheduler.process_table[idx].state == PROCESS_READY) {
      // 1. The interrupted process state exists on the stack -> store
      // it to the 'cur' idx.
      sched_interrupt_store(cur, stack_loc);
      // 2. Replace the interrupted process on the stack with the new one.
      sched_interrupt_replace(idx, stack_loc);
      // 3. Set the previous process to READY.
      scheduler.process_table[cur].state = PROCESS_READY;
      // 4. Set the next process to RUNNING.
      scheduler.process_table[idx].state = PROCESS_RUNNING;
      return;
    }
  }
}

static void sched_interrupt_store(unsigned int idx, uint32_t stack_loc) {
  uint32_t EFLAGS = *(uint32_t *)(stack_loc + 12);
  uint32_t cs  = *(uint32_t *)(stack_loc + 8);
  uint32_t eip = *(uint32_t *)(stack_loc + 4);

  uint32_t eax = *(uint32_t *)(stack_loc - (4 * 0));
  uint32_t ecx = *(uint32_t *)(stack_loc - (4 * 1));
  uint32_t edx = *(uint32_t *)(stack_loc - (4 * 2));
  uint32_t ebx = *(uint32_t *)(stack_loc - (4 * 3));
  uint32_t esp = *(uint32_t *)(stack_loc - (4 * 4));
  uint32_t ebp = *(uint32_t *)(stack_loc - (4 * 5));
  uint32_t esi = *(uint32_t *)(stack_loc - (4 * 6));
  uint32_t edi = *(uint32_t *)(stack_loc - (4 * 7));

  scheduler.process_table[idx].eip = eip;
  scheduler.process_table[idx].register_ctx.cs = cs;
  scheduler.process_table[idx].register_ctx.EFLAGS = EFLAGS;
  scheduler.process_table[idx].register_ctx.eax = eax;
  scheduler.process_table[idx].register_ctx.ebx = ebx;
  scheduler.process_table[idx].register_ctx.ecx = ecx;
  scheduler.process_table[idx].register_ctx.edx = edx;
  scheduler.process_table[idx].register_ctx.esp = esp;
  scheduler.process_table[idx].register_ctx.ebp = ebp;
  scheduler.process_table[idx].register_ctx.esi = esi;
  scheduler.process_table[idx].register_ctx.edi = edi;
}

static void sched_interrupt_replace(unsigned int idx, uint32_t stack_loc) {
  uint32_t *EFLAGS = (uint32_t *)(stack_loc + 12);
  uint32_t *cs  = (uint32_t *)(stack_loc + 8);
  uint32_t *eip = (uint32_t *)(stack_loc + 4);

  uint32_t *eax = (uint32_t *)(stack_loc - (4 * 0));
  uint32_t *ecx = (uint32_t *)(stack_loc - (4 * 1));
  uint32_t *edx = (uint32_t *)(stack_loc - (4 * 2));
  uint32_t *ebx = (uint32_t *)(stack_loc - (4 * 3));
  uint32_t *esp = (uint32_t *)(stack_loc - (4 * 4));
  uint32_t *ebp = (uint32_t *)(stack_loc - (4 * 5));
  uint32_t *esi = (uint32_t *)(stack_loc - (4 * 6));
  uint32_t *edi = (uint32_t *)(stack_loc - (4 * 7));
  
  *eip = scheduler.process_table[idx].eip;
  *cs = scheduler.process_table[idx].register_ctx.cs;
  *EFLAGS = scheduler.process_table[idx].register_ctx.EFLAGS;
  *eax = scheduler.process_table[idx].register_ctx.eax;
  *ecx = scheduler.process_table[idx].register_ctx.ecx;
  *edx = scheduler.process_table[idx].register_ctx.edx;
  *ebx = scheduler.process_table[idx].register_ctx.ebx;
  *esp = scheduler.process_table[idx].register_ctx.esp;
  *ebp = scheduler.process_table[idx].register_ctx.ebp;
  *esi = scheduler.process_table[idx].register_ctx.esi;
  *edi = scheduler.process_table[idx].register_ctx.edi;
}
