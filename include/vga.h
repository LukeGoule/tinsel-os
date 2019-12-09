#ifndef VGA_H
#define VGA_H

#include <multiboot.h>
#include <cdefs.h>

#define bit(x, y)                       ((x >> y) & 0x01)                                                                               // bit by pos (x=input number, y=bit position)
#define byteColours(bR, bG, bB)         (uint32_t)(((uint8_t)bR << 0) | ((uint8_t)bG << 8) | ((uint8_t)bB << 16)) + 0xFF000000          // Convert rgb values to a 32-bit int (for pixel clr)
#define byteColoursA(bR, bG, bB, bA)    (uint32_t)(((uint8_t)bR << 0) | ((uint8_t)bG << 8) | ((uint8_t)bB << 16) | ((uint8_t)bA << 24)) // Convert rgba values to a 32-bit int (for pixel clr)
#define CHAR_WIDTH  8
#define CHAR_HEIGHT 16

typedef struct vga_info_t {
    uint32_t* frame;
    uint32_t pitch;
    uint32_t width      = 0x0;
    uint32_t height     = 0x0;
    uint32_t bitdepth   = 0x0;
};

void vga_install    (multiboot_info_t* mbi);
void vga_plotpixel  (uint32_t x, uint32_t y, uint32_t clr);
void vga_prims_box  (uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t clr);
void vga_printc     (char c, uint32_t x, uint32_t y, uint32_t fg_clr, uint32_t bg_clr);
vga_info_t*
     vga_get_info   ();
bool CMD_vgatest    (int argc, char** argv);

#endif
