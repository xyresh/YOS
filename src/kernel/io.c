#include "io.h"
#include <stdint.h>



void outw(uint16_t port, uint16_t data) {
    __asm__ __volatile__("outw %1, %0" : : "dN"(port), "a"(data));
}

void outb(uint16_t port, uint8_t data) {
    __asm__ __volatile__("outb %1, %0" : : "dN"(port), "a"(data));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;
volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;
static int cursor_x = 0;
static int cursor_y = 0;

void scroll_screen() {
    for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }
    for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)(' ' | (0xF0 << 8));
    }
    cursor_y = VGA_HEIGHT - 1;
    update_cursor();
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)(' ' | (0xF0 << 8));
    }
    cursor_x = 0;
    cursor_y = 1;
    update_cursor();
    draw_top_bar("00:00:00");
}

void print_char(char c, uint8_t color) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = ' ' | (color << 8);
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~(8 - 1);
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = c | (color << 8);
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
    }
    
    update_cursor();
}

void print(const char* str, uint8_t color) {
    for (int i = 0; str[i] != '\0'; i++) {
        print_char(str[i], color);
    }
}

const char hex_digits[] = "0123456789ABCDEF";

void print_hex(uint32_t num, int x, int y, uint8_t color) {
    char result[9];
    result[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        result[i] = hex_digits[num & 0xF];
        num >>= 4;
    }
    print(result, color);
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void update_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

#define CHAR_BUFFER_SIZE 256
volatile char char_buffer[CHAR_BUFFER_SIZE];
volatile int buffer_head = 0;
volatile int buffer_tail = 0;

void put_char_to_buffer(char c) {
    int next_head = (buffer_head + 1) % CHAR_BUFFER_SIZE;
    if (next_head != buffer_tail) {
        char_buffer[buffer_head] = c;
        buffer_head = next_head;
    }
}

char get_char_from_buffer() {
    if (buffer_head == buffer_tail) {
        return 0;
    }
    char c = char_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % CHAR_BUFFER_SIZE;
    return c;
}

#define PIC1_COMMAND_PORT 0x20
#define PIC1_DATA_PORT    0x21
#define PIC2_COMMAND_PORT 0xA0
#define PIC2_DATA_PORT    0xA1

void pic_remap() {
    outb(PIC1_COMMAND_PORT, 0x11);
    outb(PIC2_COMMAND_PORT, 0x11);
    outb(PIC1_DATA_PORT, 0x20);
    outb(PIC2_DATA_PORT, 0x28);
    outb(PIC1_DATA_PORT, 0x04);
    outb(PIC2_DATA_PORT, 0x02);
    outb(PIC1_DATA_PORT, 0x01);
    outb(PIC2_DATA_PORT, 0x01);
    outb(PIC1_DATA_PORT, 0x0);
    outb(PIC2_DATA_PORT, 0x0);
}
void print_dec(uint32_t num, uint8_t color) {
    char buf[11];
    int i = 10;
    buf[i] = '\0';
    if (num == 0) {
        print("0", color);
        return;
    }
    while (num > 0 && i > 0) {
        buf[--i] = '0' + (num % 10);
        num /= 10;
    }
    print(&buf[i], color);
}
// Read a single RTC register
uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

// Convert BCD to binary if needed the RTC may use BCD format
uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

void get_time(uint8_t* hour, uint8_t* min, uint8_t* sec) {
    uint8_t h, m, s, regB;
    s = cmos_read(0x00);
    m = cmos_read(0x02);
    h = cmos_read(0x04);
    regB = cmos_read(0x0B);

    // If not in binary mode, convert from BCD
    if (!(regB & 0x04)) {
        s = bcd_to_bin(s);
        m = bcd_to_bin(m);
        h = bcd_to_bin(h);
    }

    *hour = h;
    *min  = m;
    *sec  = s;
}

#define TOPBAR_COLOR 0x3F  
#define TOPBAR_TEXT_COLOR 0x3F 

void draw_top_bar(const char* time_str) {
    // Fill the top row with spaces
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_buffer[i] = (' ' | (TOPBAR_COLOR << 8));
    }
    // " at left
    vga_buffer[0] = ('Y' | (TOPBAR_TEXT_COLOR << 8));
    vga_buffer[1] = ('O' | (TOPBAR_TEXT_COLOR << 8));
    vga_buffer[2] = ('S' | (TOPBAR_TEXT_COLOR << 8));
    vga_buffer[3] = (' ' | (TOPBAR_TEXT_COLOR << 8));
    vga_buffer[4] = ('0' | (TOPBAR_TEXT_COLOR << 8));
    vga_buffer[5] = ('.' | (TOPBAR_TEXT_COLOR << 8));
    vga_buffer[6] = ('1' | (TOPBAR_TEXT_COLOR << 8));
    // Time at right
    int len = 0;
    while (time_str[len] && len < 20) len++;
    int offset = VGA_WIDTH - len;
    for (int i = 0; i < len; i++) {
        vga_buffer[offset + i] = (time_str[i] | (TOPBAR_TEXT_COLOR << 8));
    }
}

#define MOUSE_CURSOR_COLOR 0xF0

char saved_char = ' ';
uint8_t saved_color = 0xF0;

// Draw a mouse cursor
void draw_mouse_cursor() {
    uint16_t cell = vga_buffer[mouse_y * VGA_WIDTH + mouse_x];
    saved_char = (char)(cell & 0xFF);
    saved_color = (cell >> 8) & 0xFF;
    // Draw the cursor (e.g., a 'X')
    vga_buffer[mouse_y * VGA_WIDTH + mouse_x] = ('X' | (MOUSE_CURSOR_COLOR << 8));
}

// Restore the previous cell
void restore_mouse_cell(int x, int y) {
    vga_buffer[y * VGA_WIDTH + x] = (saved_char | (saved_color << 8));
}
