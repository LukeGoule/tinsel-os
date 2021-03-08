#include <vga.h>
#include <shell_font.h>
#include <stdio.h>
#include <memory.h>

vga_info_t vga;

void vga_install(multiboot_info_t* mbi) {
    vga.frame       = (void*)(uint32_t)mbi->framebuffer_addr;
    vga.pitch       = mbi->framebuffer_pitch;
    vga.bitdepth    = mbi->framebuffer_bpp;
    vga.width       = mbi->framebuffer_width;
    vga.height      = mbi->framebuffer_height;
    uint32_t malloc_sz = vga.width * vga.height * vga.bitdepth;
    vga.backbuffer  = (uint32_t*)kmalloc(malloc_sz);
    // std::cout << (int)malloc_sz << " bytes allocated for VGA" << std::endl;
}

uint8_t byte_inverse(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// refer to https://jsandler18.github.io/extra/framebuffer.html
// to understand how this works.
void vga_plotpixel(uint32_t x, uint32_t y, uint32_t clr, bool bBackBuffer ) 
{
    uint32_t* buf = vga.frame;
    if (bBackBuffer)
    {
        buf = vga.backbuffer;
    }

    uint32_t *pix = (uint32_t)buf + (vga.pitch * y) + (vga.bitdepth / 8) * x;
	clr = clr | 0xFF000000; // add the alpha component.
 	*pix = clr;
}

uint32_t vga_getpixel(uint32_t x, uint32_t y, bool bBackBuffer) 
{
    uint32_t* buf = vga.frame;
    if (bBackBuffer)
    {
        buf = vga.backbuffer;
    }

    uint32_t* pix = (uint32_t)buf + (vga.pitch * y) + (vga.bitdepth / 8) * x;
    return *pix;
}

void vga_push_backbuffer()
{
    for (uint32_t x = 0; x < vga.width; x++)
    {
        for (uint32_t y = 0; y < vga.width; y++)
        {
            uint32_t *pix = (uint32_t)/*vga.frame*/vga.frame + (vga.pitch * y) + (vga.bitdepth / 8) * x;
            *pix = vga_getpixel(x,y) | 0xFF000000;
        }
    }
}

#include <ints/ints.h>
#include <mouse.h>
#include <windows.h>
extern "C" void cpp_video_int() {
    _DISABLE_INTS;

    vga_push_backbuffer();
    win_main();
    mouse_draw();

    _END_OF_IRQ(9);

    _ENABLE_INTS;
}

void vga_prims_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t clr, bool bBackBuffer) {
    size_t xi = 0;
    size_t yi = 0;

    for (xi=x; xi < x + w; xi++) {
        for (yi=y; yi < y + h; yi++) {
            vga_plotpixel(xi,yi, clr, bBackBuffer);
        }
    }
}

void vga_prims_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t clr, bool bBackBuffer) {
    uint32_t dx = x2 - x1;
    uint32_t dy = y2 - y1;
    for (uint32_t x = x1; x < x2; x++) { //x from x1 to x2 {
        uint32_t y = y1 + dy * (x - x1) / dx;
        vga_plotpixel(x, y, clr, bBackBuffer);
    }
}

void vga_printc(char c, uint32_t x, uint32_t y, uint32_t fg_clr, uint32_t bg_clr, bool bBackBuffer) {
    for (size_t j = 0; j < 16; j++) {
		for (size_t i = 0; i < 8; i++) {
            uint8_t character_data = byte_inverse(VidFontIso8x16Data[((c - ' ')*16)+j]);
			if (bit(character_data, i)==1) {
				vga_plotpixel(x + i, y + j, fg_clr, bBackBuffer);
			} else {
				vga_plotpixel(x + i, y + j, bg_clr, bBackBuffer);
			}
		}
	}
}

void vga_fill_bottom_flat_triangle(vec2f v1, vec2f v2, vec2f v3, uint32_t Colour, bool bBackBuffer)
{
    float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
    float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);

    float curx1 = v1.x;
    float curx2 = v1.x;

    for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++)
    {
        vga_prims_line((int)curx1, scanlineY, (int)curx2, scanlineY, Colour, bBackBuffer);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}


void vga_fill_top_flat_triangle(vec2f v1, vec2f v2, vec2f v3, uint32_t Colour, bool bBackBuffer)
{
    float invslope1 = (v3.x - v1.x) / (v3.y - v1.y);
    float invslope2 = (v3.x - v2.x) / (v3.y - v2.y);

    float curx1 = v3.x;
    float curx2 = v3.x;

    for (int scanlineY = v3.y; scanlineY > v1.y; scanlineY--)
    {
        vga_prims_line((int)curx1, scanlineY, (int)curx2, scanlineY, Colour, bBackBuffer);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void vga_fill_full_triangle(vec2f vt1, vec2f vt2, vec2f vt3, uint32_t Colour, bool bBackBuffer) 
{
    /* get the bounding box of the triangle */
    int maxX = std::max(vt1.x, std::max(vt2.x, vt3.x));
    int minX = std::min(vt1.x, std::min(vt2.x, vt3.x));
    int maxY = std::max(vt1.y, std::max(vt2.y, vt3.y));
    int minY = std::min(vt1.y, std::min(vt2.y, vt3.y));

    /* spanning vectors of edge (v1,v2) and (v1,v3) */
    vec2f vs1 = vec2f(vt2.x - vt1.x, vt2.y - vt1.y);
    vec2f vs2 = vec2f(vt3.x - vt1.x, vt3.y - vt1.y);

    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            vec2f q = vec2f(x - vt1.x, y - vt1.y);

            float s = q.crossproduct(vs2) / vs1.crossproduct(vs2);
            float t = vs1.crossproduct(q) / vs1.crossproduct(vs2);

            if ( (s >= 0) && (t >= 0) && (s + t <= 1))
            {
                vga_plotpixel(x, y, Colour, bBackBuffer);
            }
        }
    }
}

void vga_fill_full_triangle_front_buf(vec2f vt1, vec2f vt2, vec2f vt3, uint32_t Colour, bool bBackBuffer) 
{
    /* get the bounding box of the triangle */
    int maxX = std::max(vt1.x, std::max(vt2.x, vt3.x));
    int minX = std::min(vt1.x, std::min(vt2.x, vt3.x));
    int maxY = std::max(vt1.y, std::max(vt2.y, vt3.y));
    int minY = std::min(vt1.y, std::min(vt2.y, vt3.y));

    /* spanning vectors of edge (v1,v2) and (v1,v3) */
    vec2f vs1 = vec2f(vt2.x - vt1.x, vt2.y - vt1.y);
    vec2f vs2 = vec2f(vt3.x - vt1.x, vt3.y - vt1.y);

    for (int x = minX; x <= maxX; x++)
    {
        for (int y = minY; y <= maxY; y++)
        {
            vec2f q = vec2f(x - vt1.x, y - vt1.y);

            float s = q.crossproduct(vs2) / vs1.crossproduct(vs2);
            float t = vs1.crossproduct(q) / vs1.crossproduct(vs2);

            if ( (s >= 0) && (t >= 0) && (s + t <= 1))
            {
                vga_plotpixel(x, y, Colour, bBackBuffer);
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

vec2f::vec2f()
:   x(0.f), y(0.f)
{

}

vec2f::vec2f(float _x, float _y)
:   x(_x), y(_y)
{

}

vec2f::vec2f(vec2f* pCpy)
:   x(pCpy->x), y(pCpy->y)
{

}

float vec2f::crossproduct(vec2f V)
{
    return (this->x * V.y) - (this->y * V.x);
}
