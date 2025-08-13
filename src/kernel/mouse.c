#include "io.h"
#include <stdint.h>

volatile int mouse_x = 40, mouse_y = 12; // Start near center
static int mouse_cycle = 0;
static uint8_t mouse_bytes[3];

// Mouse write command to controller
void mouse_wait() {
    // Wait for the controller to be ready for I/O
    for (int i = 0; i < 100000; i++) {
        if ((inb(0x64) & 1) == 0) return;
    }
}
void mouse_wait_input() {
    // Wait for input buffer to be clear
    for (int i = 0; i < 100000; i++) {
        if ((inb(0x64) & 2) == 0) return;
    }
}
void mouse_write(uint8_t a_write) {
    mouse_wait_input();
    outb(0x64, 0xD4);
    mouse_wait_input();
    outb(0x60, a_write);
}
uint8_t mouse_read() {
    mouse_wait();
    return inb(0x60);
}

void mouse_handler() {
    uint8_t status = inb(0x64);
    if (!(status & 1)) return;
    uint8_t data = inb(0x60);

    mouse_bytes[mouse_cycle++] = data;
    if (mouse_cycle == 3) {
        int dx = (int8_t)mouse_bytes[1];
        int dy = (int8_t)mouse_bytes[2];
        // Update mouse position, clamp to screen
        mouse_x += dx;
        mouse_y -= dy; // Y is inverted (up is negative)
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_x >= 80) mouse_x = 79;
        if (mouse_y < 1) mouse_y = 1; // row 0 is top bar
        if (mouse_y >= 25) mouse_y = 24;

        mouse_cycle = 0;
    }
    // Send EOI
    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

// Real mouse setup
void mouse_install() {
    // Enable the auxiliary device - mouse
    mouse_wait_input();
    outb(0x64, 0xA8);

    // Enable the interrupts
    mouse_wait_input();
    outb(0x64, 0x20);
    uint8_t status = mouse_read();
    status |= 2;
    mouse_wait_input();
    outb(0x64, 0x60);
    mouse_wait_input();
    outb(0x60, status);

    // Tell the mouse to use default settings
    mouse_write(0xF6); mouse_read();
    // Enable data reporting
    mouse_write(0xF4); mouse_read();

    // Now, IRQ12 will fire on mouse packets!
}