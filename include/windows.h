#ifndef WINDOWS_H
#define WINDOWS_H
/* some sort of simple window manager. NOT TO BE CONFUSED WITH THE POPULAR OS "Windows" */

#include <cdefs.h>
#include <shell_cmd.h>

#define MAX_WINDOW_OBJECTS 128

class Window {
    uint32_t window_id          = NULL;
    const char*     name        = NULL;
    uint32_t        x = NULL, y = NULL,
                    w = 200,  h = 200,
                    bgclr       = 0xFFFFFF,
                    needs_update= true;
    cmd_callback_t* draw_func   = NULL;

public:

    Window(uint32_t id, const char* szName, uint32_t px, uint32_t py, cmd_callback_t draw_func);
    ~Window();
    void Draw();
    void Dimensions(uint32_t w, uint32_t h);
    void Background(uint32_t colour);
};

Window* CreateWindow(const char* szName, uint32_t px, uint32_t py, cmd_callback_t draw_func);

void win_init();
void win_main();
bool CMD_refresh_win(int argc, char** argv);
bool CMD_start_win(int argc, char** argv);

#endif
