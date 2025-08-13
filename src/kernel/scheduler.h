#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef struct task {
    uint32_t esp;
    struct task* next;
    int killed; // NEW: 0 = running, 1 = killed/faulted
} task_t;

void init_scheduler();
void create_task(void (*entry_point)());
void schedule();
void yield(); 
void shell_task();

extern task_t* current_task;
extern task_t* head;
extern int scheduler_enabled;

#endif