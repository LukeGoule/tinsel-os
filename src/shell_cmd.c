#include <shell_cmd.h>
#include <cdefs.h>
#include <input.h>
#include <stdio.h>

command_t Commands[128];
uint32_t  command_cnt = 0;

void KCreateCommand(const char* cmd, cmd_callback_t* callback) {
    KCreateCommand(cmd, callback, "Helptext hasn't been setup for this command, sorry.");
}

void KCreateCommand(const char* cmd, cmd_callback_t* callback, const char* helptext) {
    command_t ncmd;
    ncmd.cmd = cmd;
    ncmd.callback = callback;
    ncmd.helptext = helptext;
    Commands[command_cnt] = ncmd;

    command_cnt += 1;
}

// Search for a command by name:
//  cmd => the name of it
//  output => second output thingy
bool KFindCommand(const char* cmd, command_t &output) {
    bool cmd_found = false;

    for (size_t i = 0; i < command_cnt; i++) {
        if (strcmp(Commands[i].cmd, cmd) == 0) {
            output = Commands[i];
            cmd_found   = true;
        }
    }

    return cmd_found;
}

bool CMD_Help(char* inp) {
    for (size_t i = 0; i < command_cnt; i++) {
        printf("[%5%s%0] %s\n", Commands[i].cmd, Commands[i].helptext);
    }

    return true;
}