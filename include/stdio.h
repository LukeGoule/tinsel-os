#ifndef STDIO_H
#define STDIO_H

#include <cdefs.h>
#include <vga.h>

#define TAB_WIDTH CHAR_WIDTH * 4

// crappy workaround for the fact w is incorrect as it's a short.
#define outports    outportw
#define inports     inportw

typedef struct cursor_t {
    uint32_t x = 0;
    uint32_t y = CHAR_HEIGHT + 1;

    uint32_t fg_clr = 0xFFFFFF; // white
    uint32_t bg_clr = 0x000000; // black
};


void        safe_cursor_pos     ();
cursor_t*   get_cursor          ();
void        cursor_backspace    ();

void        printf      (const char* format, ...);
void        putc        (char c);
void        puts        (char* str);
size_t      strlen      (char* str);
void        itoa        (char *buf, int base, int d);
size_t      strlen      (const char* str);
size_t      strcmp      (const char* s1, const char* s2);
size_t      strcmpl     (const char* s1, const char* s2, size_t d);
void        strcpy      (char *d, char *s);
size_t      strexplode  (char* string, char delimiter);
char*       get_explode_output
                        (size_t index);
void* memcpy(void* dstptr, const void* srcptr, size_t size);

uint8_t     inportb     (uint16_t port);
void        outportb    (uint16_t port, uint8_t data);

uint16_t    inportw     (uint16_t port);
void        outportw    (uint16_t port, uint16_t value);

uint32_t    inportl     (uint16_t port);
void        outportl    (uint16_t port, uint32_t value);

bool        CMD_StdioTest
                        (int argc, char** argv);
bool        CMD_ArgTest (int argc, char** argv);
bool        CMD_cls     (int argc, char** argv);

#endif
