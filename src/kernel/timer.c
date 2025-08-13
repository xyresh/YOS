#include "timer.h"
#include "io.h"
#include "scheduler.h"
#include <stdint.h>

volatile uint32_t timer_ticks = 0;

extern int scheduler_enabled;


void timer_handler() {
    timer_ticks++;
    if (scheduler_enabled) {
        yield();
    }
}

void init_timer(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}