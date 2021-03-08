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


namespace std 
{
    void        safe_cursor_pos     ();
    cursor_t*   get_cursor          ();
    void        cursor_backspace    ();

    void        printf      (const char* format, ...);
    void        putc        (char c);
    void        puts        (char* str);

    void        itoa        (char *buf, int base, int d);
    size_t      strlen      (char* str);
    size_t      strlen      (const char* str);
    size_t      strcmp      (const char* s1, const char* s2);
    size_t      strcmpl     (const char* s1, const char* s2, size_t d);
    void        strcpy      (char *d, char *s);
    
    size_t      strexplode  (char* string, char delimiter);
    char*       get_explode_output(size_t index);
    
    void* memcpy(void* dstptr, const void* srcptr, size_t size);
    
    float max(float a, float b);
    float min(float a, float b);

    class ostream
    {
    public:
        ostream& operator << (char v);
        ostream& operator << (char* v);
        ostream& operator << (const char* v);
        ostream& operator << (int v);
        ostream& operator << (short v);
        ostream& operator << (long v);
        ostream& operator << (long long v);
        ostream& operator << (void* v);
        ostream& operator << (float v);
        ostream& operator << (double v);

        // modifier functions i.e. endl
        ostream& operator << (ostream&(*func)(ostream&));
    };

    ostream& endl(ostream& os);

    // Colour modifier functions

    ostream& _clr_normal(ostream& os);
    ostream& _clr_invert(ostream& os);
    ostream& _clr_red(ostream& os);
    ostream& _clr_green(ostream& os);
    ostream& _clr_blue(ostream& os);
    ostream& _clr_orange(ostream& os);

    extern std::ostream cout;
}

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
