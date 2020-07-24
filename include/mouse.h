#ifndef MOUSE_H
#define MOUSE_H

#include <cdefs.h>

#define MOUSE_LBUTTON   (1 << 0)
#define MOUSE_RBUTTON   (1 << 1)
#define MOUSE_MBUTTON   (1 << 2)

void    mouse_pos(int* x, int* y);
void    mouse_wait(uint8_t a_type);
void    mouse_write(uint8_t a_write);
uint8_t mouse_read();
void    mouse_install();

#endif
