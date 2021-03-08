#include <windows.h>
#include <vga.h>
#include <stdio.h>
#include <memory.h>

Window::Window(uint32_t id, const char* szName, uint32_t px, uint32_t py, cmd_callback_t draw_func) {
    this->window_id = id;
    this->name = szName;
    this->x = px;
    this->y = py;
    this->draw_func = draw_func;
};

Window::~Window() {

}

void Window::Draw() 
{
    if (needs_update) {
        vga_prims_box(this->x, this->y, this->w, this->h, this->bgclr, false);
        
        for (size_t i = 0; i < std::strlen(this->name); i++) {
            vga_printc(this->name[i], this->x + 1 + (i*CHAR_WIDTH), this->y + 1, ~this->bgclr, this->bgclr, false);
        }

        vga_printc('X', this->x + this->w - (CHAR_WIDTH), this->y + 1, 0xFFFF0000, ~0, false);

        vga_prims_line(this->x, this->y + (1 + CHAR_HEIGHT), this->x + this->w, this->y + (1 + CHAR_HEIGHT), 0x0, false);
        
        this->needs_update = true;
    }
}

void Window::Dimensions(uint32_t w, uint32_t h) {
    this->w = w;
    this->h = h;
}

void Window::Background(uint32_t colour) {
    this->bgclr = colour;
    this->needs_update = true;
}

Window** windows = NULL;
uint32_t windows_num = 0;
Window* test = NULL;
bool bInitialised = false;

Window* CreateWindow(const char* szName, uint32_t px, uint32_t py, cmd_callback_t draw_func) {
    Window* nWin = new Window(windows_num, szName, px, py, draw_func);
    windows[windows_num] = nWin;
    windows_num ++;

    return nWin;
}

void win_main() {
    if (!bInitialised) return;

    test->Draw();
}

void win_init() 
{
    windows         = (Window**)kmalloc(sizeof(Window*) * MAX_WINDOW_OBJECTS);
    windows_num     = 0;

    test = CreateWindow("Test window title", 400, 400, NULL);

    bInitialised    = true;
}

bool CMD_refresh_win(int argc, char** argv) {

}
