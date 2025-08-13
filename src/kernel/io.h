#ifndef IO_H
#define IO_H

#include <stdint.h>
#include "mouse.h"

void outw(uint16_t port, uint16_t data);
void outb(uint16_t port, uint8_t data);
uint8_t inb(uint16_t port);

void scroll_screen();
void clear_screen();
void print_char(char c, uint8_t color);
void print(const char* str, uint8_t color);
void print_hex(uint32_t num, int x, int y, uint8_t color);

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor();

void put_char_to_buffer(char c);
char get_char_from_buffer();

void pic_remap();

void print_dec(uint32_t num, uint8_t color);

uint8_t cmos_read(uint8_t reg);
void get_time(uint8_t* hour, uint8_t* min, uint8_t* sec);

void draw_top_bar(const char* time_str);

void draw_mouse_cursor();
void restore_mouse_cell(int x, int y);
#endif