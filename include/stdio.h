#ifndef STDIO_H
#define STDIO_H

#include <cdefs.h>

#define TAB_WIDTH CHAR_WIDTH * 4

// crappy workaround for the fact w is incorrect as it's a short.
#define outports    outportw
#define inports     inportw

struct cursor {
    uint32_t x = 0;
    uint32_t y = 0;

    uint32_t fg_clr = 0xFFFFFF; // white
    uint32_t bg_clr = 0x000000; // black
};

typedef cursor cursor_t;

void        safe_cursor_pos     ();
void        cursor_backspace    ();

void        printf      (const char* format, ...);
void        putc        (char c);
void        puts        (char* str);
size_t      strlen      (char* str);
void        itoa        (char *buf, int base, int d);
size_t      strlen      (const char* str);
size_t      strcmp      (const char* s1, const char* s2);
size_t      strcmpl     (const char* s1, const char* s2, size_t d);
uint8_t     inportb     (uint16_t port);
void        outportb    (uint16_t port, uint8_t data);

uint16_t    inportw     (uint16_t port);
void        outportw    (uint16_t port, uint16_t value);

uint32_t    inportl     (uint16_t port);
void        outportl    (uint16_t port, uint32_t value);

void stdio_debug_tests();

#endif
