#include "io.h"
#include "idt.h"

void print_registers(struct registers *regs) {
    print("\n--- Exception Occurred ---\n", 0xC0);
    print("EAX: ", 0xC0); print_hex(regs->eax, 0, 0, 0xC0); print("  ", 0xC0);
    print("EBX: ", 0xC0); print_hex(regs->ebx, 0, 0, 0xC0); print("\n", 0xC0);
    print("ECX: ", 0xC0); print_hex(regs->ecx, 0, 0, 0xC0); print("  ", 0xC0);
    print("EDX: ", 0xC0); print_hex(regs->edx, 0, 0, 0xC0); print("\n", 0xC0);
    print("ESI: ", 0xC0); print_hex(regs->esi, 0, 0, 0xC0); print("  ", 0xC0);
    print("EDI: ", 0xC0); print_hex(regs->edi, 0, 0, 0xC0); print("\n", 0xC0);
    print("EBP: ", 0xC0); print_hex(regs->ebp, 0, 0, 0xC0); print("  ", 0xC0);
    print("ESP: ", 0xC0); print_hex(regs->esp, 0, 0, 0xC0); print("\n", 0xC0);
    print("EIP: ", 0xC0); print_hex(regs->eip, 0, 0, 0xC0); print("  ", 0xC0);
    print("EFLAGS: ", 0xC0); print_hex(regs->eflags, 0, 0, 0xC0); print("\n", 0xC0);
    print("INT#: ", 0xC0); print_hex(regs->interrupt_number, 0, 0, 0xC0); print("  ", 0xC0);
    print("ERR: ", 0xC0); print_hex(regs->error_code, 0, 0, 0xC0); print("\n", 0xC0);
}