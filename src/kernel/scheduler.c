#include "scheduler.h"
#include "io.h"
#include <stdint.h>
#include "trampoline.h"

#define STACK_SIZE 4096

task_t* head = 0;
task_t* tail = 0;
task_t* current_task = 0;
int scheduler_enabled = 0;

void context_switch_asm(uint32_t* old_task_esp, uint32_t* new_task_esp);

void init_scheduler() {
    head = 0;
    tail = 0;
    current_task = 0;
    scheduler_enabled = 0;
}

void create_task(void (*entry_point)()) {
    static uint32_t next_task_base = 0x200000;
    task_t* new_task = (task_t*)next_task_base;
    uint32_t stack_top = next_task_base + STACK_SIZE;
    uint32_t* stack = (uint32_t*)stack_top;

    // Argument to trampoline: entry_point
    *(--stack) = (uint32_t)entry_point;
    // Fake return address (never used, but needed for calling convention)
    *(--stack) = 0;
    // Address of trampoline (this is where ret will go)
    extern void task_trampoline(void (*entry_point)());
    *(--stack) = (uint32_t)task_trampoline;

    // Optionally, push dummy registers for popad if your context switch expects it
    for (int i = 0; i < 8; i++)
        *(--stack) = 0;

    new_task->esp = (uint32_t)stack;

    if (head == 0) {
        head = new_task;
        tail = new_task;
        new_task->next = new_task;

    } else {
        tail->next = new_task;
        new_task->next = head;
        tail = new_task;
    }

    next_task_base += STACK_SIZE; 

}

void schedule() {
    task_t* old_task = current_task;
    do {
        current_task = current_task->next;
        // If all tasks are killed, prevent infinite loop
        if (current_task == old_task) break;
    } while (current_task->killed);

    context_switch_asm(&old_task->esp, &current_task->esp);
}

void yield() {
    if (scheduler_enabled) {
        schedule();
    }
}

void blinker_task() {
    while(1) {
        disable_cursor();
        for (volatile int i = 0; i < 100000000; i++) {}
        enable_cursor(14, 15);
        for (volatile int i = 0; i < 100000000; i++) {}
        yield();
    }
}