#ifndef DEBUG_H
#define DEBUG_H

#include "idt.h"

// Prints the contents of the registers struct (for exceptions and debugging)
void print_registers(struct registers *regs);

#endif