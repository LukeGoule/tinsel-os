#include <mouse.h>
#include <stdio.h>
#include <ints/ints.h>
#include <vga.h>
#include <geometry/vec.h>

/*
https://forum.osdev.org/viewtopic.php?t=10247
*/

uint8_t  mouse_cycle = 0;
int8_t   mouse_byte[4];
int32_t  mouse_x = 1024 / 2;
int32_t  mouse_y = 768  / 2;
uint32_t prev_pixel = 0;

void mouse_pos(int* x, int* y) {
    *x = mouse_x;
    *y = mouse_y;
}

uint32_t mouse_state = 0;
void mouse_logic() {
    mouse_state = mouse_byte[0];

    if (mouse_byte[0] & MOUSE_LBUTTON) {
        // std::printf("Left down.\n");
    }
    if (mouse_byte[0] & MOUSE_RBUTTON) {
        // std::printf("Right down.\n");
    }
    if (mouse_byte[0] & MOUSE_MBUTTON) {
        // std::printf("Middle down.\n");
    }

    uint32_t pmouse_x = mouse_x;
    uint32_t pmouse_y = mouse_y;
    uint32_t dmouse_x = mouse_x + mouse_byte[1] / 3;
    uint32_t dmouse_y = mouse_y-(mouse_byte[2] / 3);
    vga_info_t* vga = vga_get_info();

    if (!(dmouse_x <= 0) && !(dmouse_x >= vga->width)) {
        mouse_x = dmouse_x;
    }

    if (!(dmouse_y <= 0) && !(dmouse_y >= vga->height)) {
        mouse_y = dmouse_y;
    }
}

extern "C" void cpp_mouse_handler() {
    _DISABLE_INTS;

    switch (mouse_cycle) {
    case 0:
        mouse_byte[0] = inportb(0x60);
        mouse_cycle++;
        break;
    case 1:
        mouse_byte[1] = inportb(0x60);
        mouse_cycle++;
        break;
    case 2:
        mouse_byte[2] = inportb(0x60);
        mouse_cycle=0;

        mouse_logic();
        break;
    }

    _END_OF_IRQ(12);

    _ENABLE_INTS;
}


void mouse_wait(uint8_t a_type) {
    uint32_t _time_out = 100000;
    if(a_type == 0) {
        while(_time_out--) {
            if((inportb(0x64) & 1)==1) {
                return;
            }
        }
        return;
    }
    else {
        while(_time_out--) {
            if((inportb(0x64) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

void mouse_write(uint8_t a_write) //unsigned char
{
  //Wait to be able to send a command
  mouse_wait(1);
  //Tell the mouse we are sending a command
  outportb(0x64, 0xD4);
  //Wait for the final part
  mouse_wait(1);
  //Finally write
  outportb(0x60, a_write);
}

uint8_t mouse_read() {
    //Get's response from mouse
    mouse_wait(0);
    return inportb(0x60);
}

void mouse_install() {
    uint8_t _status;  //unsigned char

    //Enable the auxiliary mouse device
    mouse_wait(1);
    outportb(0x64, 0xA8);

    //Enable the interrupts
    mouse_wait(1);
    outportb(0x64, 0x20);
    mouse_wait(0);
    _status=(inportb(0x60) | 2);
    mouse_wait(1);
    outportb(0x64, 0x60);
    mouse_wait(1);
    outportb(0x60, _status);

    //Tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();  //Acknowledge

    //Enable the mouse
    mouse_write(0xF4);
    mouse_read();  //Acknowledge
}

// Drawn to the front buffer, not the backbuffer, so it can be updated every "frame".
void mouse_draw()
{
    // Black outline
    vec2f main_point2(mouse_x - 1, mouse_y - 2);
    vec2f pB2(mouse_x + 10 + 2, mouse_y + 10 + 1);
    vec2f pC2(mouse_x - 1, mouse_y + 15 + 1);

    vga_fill_full_triangle(main_point2, pB2, pC2, 0x0, false);

    // White triangle
    vec2f main_point(mouse_x, mouse_y);
    vec2f pB(mouse_x + 10, mouse_y + 10);
    vec2f pC(mouse_x, mouse_y + 15);
    vga_fill_full_triangle(main_point, pB, pC, ~(0), false);    
}