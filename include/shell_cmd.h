#ifndef SHELL_CMD_H
#define SHELL_CMD_H

#include <cdefs.h>
#include <input.h>

typedef bool (cmd_callback_t)(char* inp);

typedef struct command_t {
    const char* cmd;
    const char* helptext;
    cmd_callback_t* callback;
};

void    KCreateCommand  (const char* cmd, cmd_callback_t* callback);
void    KCreateCommand  (const char* cmd, cmd_callback_t* callback, const char* helptext);
bool    KFindCommand    (const char* cmd, command_t &output);
bool    CMD_Help        (char* inp);
#endif
