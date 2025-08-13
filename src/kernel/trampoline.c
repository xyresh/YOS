#include "io.h"

// Called as trampoline(entry_point);
void task_trampoline(void (*entry_point)()) {
    entry_point();
    print("\nTask returned! Halting.\n", 0xC4);
    while (1) { __asm__ __volatile__("cli; hlt"); }
}