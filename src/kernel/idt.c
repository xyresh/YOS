#include "idt.h"
#include "io.h"
#include "timer.h"
#include "scheduler.h"
#include "debug.h"
#include <stdint.h>
#include "mouse.h"
#include "paging.h"

extern void (*isr_stubs[])();

struct idt_entry_struct idt[256];
struct idt_ptr_struct idt_ptr;

static uint8_t shift_pressed = 0;

const char keyboard_map_unshifted[] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char keyboard_map_shifted[] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void page_fault_handler(struct registers* regs) {
    uint32_t fault_addr;
    __asm__ __volatile__("mov %%cr2, %0" : "=r"(fault_addr));
    print("\nPage fault at 0x", 0xC4); print_hex(fault_addr, 0, 0, 0xC4);
    print(" (EIP 0x", 0xC4); print_hex(regs->eip, 0, 0, 0xC4); print(")\n", 0xC4);

    extern task_t* current_task;
    current_task->killed = 1; // NEW: mark as killed

    yield(); // Switch to next task
    // Never returns
}

void isr_handler(struct registers *regs) {
    if (regs->interrupt_number >= 0x20 && regs->interrupt_number <= 0x2F) {
        if (regs->interrupt_number < 0x20) {
            // Exception!
            print_registers(regs);
            print("Fatal exception! Halting...\n", 0xC4);
            while (1) { __asm__ __volatile__("cli; hlt"); }
        }
        if (regs->interrupt_number == 0x20) {
            timer_handler();
        } else if (regs->interrupt_number == 0x21) {
            uint8_t scancode = inb(0x60);
            if (scancode == 0x2A || scancode == 0x36) {
                shift_pressed = 1;
            } else if (scancode == 0xAA || scancode == 0xB6) {
                shift_pressed = 0;
            } else if (scancode < 0x80) {
                char c;
                if (shift_pressed) {
                    c = keyboard_map_shifted[scancode];
                } else {
                    c = keyboard_map_unshifted[scancode];
                }
                if ((c >= ' ' && c <= '~') || c == '\b' || c == '\n' || c == '\t') {
                    put_char_to_buffer(c);
                }
            }
        }else if (regs->interrupt_number == 0x2C) {  // IRQ12 = 0x2C = 44
            mouse_handler();
        }

        if (regs->interrupt_number == 0x0E) {
            page_fault_handler(regs);
            return;
        }

        if (regs->interrupt_number >= 0x28) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
    }
}



void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_init() {
    __asm__ __volatile__("cli");

    idt_ptr.limit = sizeof(struct idt_entry_struct) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)isr_stubs[i], 0x08, 0x8E);
    }
    
    __asm__ __volatile__("lidt %0" : : "m"(idt_ptr));
}