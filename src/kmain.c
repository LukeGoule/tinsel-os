#include <cdefs.h>
#include <kernel.h>
#include <multiboot.h>
#include <vga.h>
#include <stdio.h>
#include <input.h>
#include <acpi.h>

char buf[MAX_INPUT_LEN];
uint32_t iter = 0;
bool die = false;

// do something with the user input.
// it will only return false if something has
//  A) Gone terribly wrong,
//  B) The user wants a shutdown / restart.
bool exec_cmd() {
    printf("\n"); // hackkk

    if (strcmp(buf, "shutdown") == 0) {
        die = true;

        vga_info_t* vga = vga_get_info();
        vga_prims_box(0,0, vga->width, vga->height, 0xFF0000);
        printf("Death imminent!!!!!!!!!");

        acpi_poweroff();

        return false;
    } else {
        printf("Command '%s' was not understood.\n", buf);
    }

    // wipe the buffer.
    for (size_t i = 0; i < MAX_INPUT_LEN; i++) {
        buf[i] = '\0';
    }

    iter = 0;

    return true;
}

extern "C" void kmain(unsigned int magic_number, multiboot_info_t* mbi) {
    vga_install(mbi);
    acpi_init();

    printf("%2T%3i%4n%3s%2e%3l%0 Ready.\n");

    stdio_debug_tests();

    printf("tinsel> ");
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
