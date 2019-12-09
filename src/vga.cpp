#include <vga.h>
#include <shell_font.h>

vga_info_t vga;

uint8_t byte_inverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// refer to https://jsandler18.github.io/extra/framebuffer.html
// to understand how this works.
void vga_plotpixel(uint32_t x, uint32_t y, uint32_t clr) {
    uint32_t *pix = (uint32_t)vga.frame + (vga.pitch * y) + (vga.bitdepth / 8) * x;
	clr = clr | 0xFF000000; // add the alpha component.
 	*pix = clr;
}

void vga_prims_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t clr) {
    size_t xi = 0;
    size_t yi = 0;

    for (xi=x; xi < x + w; xi++) {
        for (yi=y; yi < y + h; yi++) {
            vga_plotpixel(xi,yi, clr);
        }
    }
}

void vga_prims_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t clr) {
    uint32_t dx = x2 - x1;
    uint32_t dy = y2 - y1;
    for (uint32_t x = x1; x < x2; x++) { //x from x1 to x2 {
        uint32_t y = y1 + dy * (x - x1) / dx;
        vga_plotpixel(x, y, clr);
    }
}

void vga_install(multiboot_info_t* mbi) {
    vga.frame       = (void*)(uint32_t)mbi->framebuffer_addr;
    vga.pitch       = mbi->framebuffer_pitch;
    vga.bitdepth    = mbi->framebuffer_bpp;
    vga.width       = mbi->framebuffer_width;
    vga.height      = mbi->framebuffer_height;
}

void vga_printc(char c, uint32_t x, uint32_t y, uint32_t fg_clr, uint32_t bg_clr) {
    for (size_t j = 0; j < 16; j++) {
		for (size_t i = 0; i < 8; i++) {
            uint8_t character_data = byte_inverse(VidFontIso8x16Data[((c - ' ')*16)+j]);
			if (bit(character_data, i)==1) {
				vga_plotpixel(x + i, y + j, fg_clr);
			} else {
				vga_plotpixel(x + i, y + j, bg_clr);
			}
		}
	}
}

vga_info_t* vga_get_info() {
    return &vga;
}

bool CMD_vgatest(int argc, char** argv) {

    vga_prims_line(200,200, 250, 400, 0xFF00FF);

    return true;
}
