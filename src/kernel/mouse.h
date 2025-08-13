#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

void mouse_install();
void mouse_handler();
extern volatile int mouse_x, mouse_y;

#endif