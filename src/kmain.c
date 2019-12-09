#include <cdefs.h>
#include <kernel.h>
#include <multiboot.h>
#include <vga.h>
#include <stdio.h>
#include <input.h>
#include <acpi.h>
#include <cpu.h>
#include <memory.h>
#include <shell_cmd.h>

multiboot_info_t* info = NULL;

// things for me to read:
//  https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc
//  https://wiki.osdev.org/Model_Specific_Registers

char* buf;
uint32_t iter = 0;
bool die = false;

// Run the string entered by the user.
bool exec_cmd() {
    printf("\n"); // hackkk

    bool cmd_found = false;
    bool output    = true;
    command_t current_cmd;

    cmd_found = KFindCommand(buf, current_cmd);

    if (cmd_found) {
        output = current_cmd.callback(buf);
    } else {
        printf("Command '%s' was not understood.\n", buf);
    }

    // wipe the buffer.
    for (size_t i = 0; i < MAX_INPUT_LEN; i++) {
        buf[i] = '\0';
    }

    iter = 0;

    return output;
}

// basic kernel information
bool CMD_Info(char* inp) {
    size_t used_memory = kmalloc_used_bytes();
    size_t used_blocks = kmalloc_used_blocks();
    size_t unused_blocks = kmalloc_unused_blocks();

    printf("[%5Info%0] Kernel V: %3tinsel v1%0\n");
    printf("Memory Info:\n");
    printf("\t[%5Info%0] Used memory:   %dB\n", used_memory);
    printf("\t[%5Info%0] Used blocks:   %d\n", used_blocks);
    printf("\t[%5Info%0] Unused blocks: %d\n", unused_blocks);
    printf("Video Info:\n");
    printf("\t[%5Info%0] FB Address:    0x%x\n", (uint32_t)info->framebuffer_addr);
    printf("\t[%5Info%0] FB Width:      %dpx\n", (uint32_t)info->framebuffer_width);
    printf("\t[%5Info%0] FB Height:     %dpx\n", (uint32_t)info->framebuffer_height);
    printf("\t[%5Info%0] FB Depth:      %dbpp\n", (uint32_t)info->framebuffer_bpp);

    return true;
}

extern "C" void kmain(unsigned int magic_number, multiboot_info_t* mbi) {
    info = mbi;

    vga_install(mbi);
    acpi_init();
    memory_init(0x100000); // starting offset of 0x100000

    // Setup commands.
    KCreateCommand("help",          CMD_Help, "Show helptext of each command.");
    KCreateCommand("shutdown",      CMD_Shutdown, "Shutdown the computer.");
    KCreateCommand("stdiotest",     CMD_StdioTest, "Test standard input output library.");
    KCreateCommand("cpuid",         CMD_cpuid, "CPUID instruction tests.");
    KCreateCommand("kmalloc",       CMD_kmalloc, "Kernel level memory allocation tests.");
    KCreateCommand("info",          CMD_Info, "Show information about the kernel & system.");

    printf("%3tinsel v1%0 ready.\ntinsel> ");

    buf = (char*)kmalloc(MAX_INPUT_LEN);

    while (!die) {
        char* next_char = input_getchar();

        // ignore some characters
        if (next_char == "\0" ||
			next_char[0] == '\0' ||
			(strcmp(next_char, "<CAPSL>") == 0) ||
			(strcmp(next_char, "<LSHIFT>") == 0) ||
			(strcmp(next_char, "<RSHIFT>") == 0) ||
			(strcmp(next_char, "<LALT>") == 0)) continue;

        if (next_char[0] == '\n') { // when the user presses enter execute what they typed.
            if (exec_cmd()) printf("tinsel> ");
        }
        else if (next_char[0] == '\b' && (iter > 0)) { // backspace
            cursor_backspace();

            iter--;
            buf[iter] = '\0';
        }
        else {
            if (next_char[0] == '\b' && (iter == 0)) continue;

            printf("%s", next_char);
            buf[iter] = next_char[0];
            iter++;
        }
    }

    return;
}
