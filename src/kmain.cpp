#include <cdefs.h>
#include <kernel.h>
#include <multiboot.h>
#include <exceptions.h>
#include <vga.h>
#include <stdio.h>
#include <input.h>
#include <acpi.h>
#include <cpu.h>
#include <memory>
#include <shell_cmd.h>
#include <rtc.h>
#include <ata.h>
#include <ext2.h>
#include <pci.h>
#include <mouse.h>
#include <windows.h>

extern "C" void gdt_install();

multiboot_info_t* info = NULL;
bool ata_enabled = false;

// things to read:
//  https://www.aldeid.com/wiki/X86-assembly/Instructions/rdtsc
//  https://wiki.osdev.org/Model_Specific_Registers

char* buf;
uint32_t iter = 0;
bool die = false;

// Run the string entered by the user.
bool exec_cmd() {
    printf("\n"); // hackkk

    bool cmd_found = false, output = true, understood = true;
    command_t current_cmd;

    size_t argc = strexplode(buf, ' ');
    char** argv;

    if (argc == 0) {
        understood = false;
        goto not_understood;
    }

    argv = (char**)kmalloc(sizeof(char*) * argc); // im shook how well this works

    for (size_t i = 0; i < argc; i++) {
        argv[i] = get_explode_output(i);
    }

    cmd_found = KFindCommand(argv[0], current_cmd);

    if (cmd_found) {
        output = current_cmd.callback(argc, argv); // pass arg count and arg list to the command callback, like main(int argc, char** argv)
    } else {
        understood = false;
        goto not_understood;
    }

    // ghetto ass label usage
    not_understood:
    if (!understood) {
        printf("[%2Error%0] Command '%s' was not understood.\n", argv[0]);
    }/* code */

    // wipe the buffer.
    for (size_t i = 0; i < MAX_INPUT_LEN; i++) {
        buf[i] = '\0';
    }

    iter = 0;

    return output;
}

RealTimeClock* test_rtc;
// basic kernel information
bool CMD_Info(int argc, char** argv) {
    size_t used_memory = kmalloc_used_bytes();
    size_t used_blocks = kmalloc_used_blocks();
    size_t unused_blocks = kmalloc_unused_blocks();

    if (!test_rtc) {
        test_rtc = new RealTimeClock();
    }

    test_rtc->read_rtc();

    printf("[%5Info%0] Kernel V: %3tinsel v1%0\n");
    printf("[%5Info%0] Time: %d:%d\n", test_rtc->hour, test_rtc->minute);
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

bool CMD_ForceBreak(int argc, char** argv) {
    int i = 0 / 0; // won't be caught by the compiler, causes an exception that //should// be ignored and continued on with.
    return true;
}

void khandle_character(char* next_char) {
    // ignore some characters
    if (next_char == "\0" ||
        next_char[0] == '\0' ||
        (strcmp(next_char, "<CAPSL>") == 0)  ||
        (strcmp(next_char, "<LSHIFT>") == 0) ||
        (strcmp(next_char, "<RSHIFT>") == 0) ||
        (strcmp(next_char, "<LALT>") == 0)   ||
        (strcmp(next_char, "<LCTRL>") == 0)  ||
        (strcmp(next_char, "<RCTRL>") == 0)) return;

    if (next_char[0] == '\n') { // when the user presses enter, execute what they typed.
        if (exec_cmd()) {
            if (ata_enabled) {
                printf("%s:%s# ", ide_drive_name(ata_get_main_dev()), ext2_current_dir());
            } else {
                printf("TiOS# ");
            }
        }
    }
    else if (next_char[0] == '\b' && (iter > 0)) { // backspace
        cursor_backspace();

        iter--;
        buf[iter] = '\0';
    }
    else {
        if (next_char[0] == '\b' && (iter == 0)) return;

        printf("%s", next_char);
        buf[iter] = next_char[0];
        iter++;
    }
}

extern "C" void cpp_kbd_handler(void) {
    uint8_t c = inportb(0x60);
    char* next_char = scan_2_char(c);
    khandle_character(next_char);
}

extern "C" void kmain(unsigned int magic_number, multiboot_info_t* mbi) {
    info = mbi;

    memory_init     (0x100000); // Kernel memory manager
    gdt_install     ();         // GDT
    mouse_install   ();         // Talk to mouse (must be before ISR)
    isr_init        ();         // ISR
    vga_install     (mbi);      // VGA Graphics stuff
    acpi_init       ();         // Enable ACPI for shutdown
    win_init        ();         // Setup the basic window manager
    rtc_irq         ();
    ata_enabled     = ata_init ();
    ext2_init       ();

    // Setup commands.
    KCreateCommand("help",          CMD_Help        /*shell_cmd.cpp*/,  "Show helptext of each command.");
    KCreateCommand("shutdown",      CMD_Shutdown    /*acpi.cpp*/,       "Shutdown the computer.");
    KCreateCommand("reboot",        CMD_reboot      /*acpi.cpp*/,       "Reboot the computer.");
    KCreateCommand("stdiotest",     CMD_StdioTest   /*stdio.cpp*/,      "Test standard input output library.");
    KCreateCommand("cpuid",         CMD_cpuid       /*cpu.cpp*/,        "CPUID instruction tests.");
    KCreateCommand("kmalloc",       CMD_kmalloc     /*memory.cpp*/,     "Kernel level memory allocation tests.");
    KCreateCommand("vgatest",       CMD_vgatest     /*vga.cpp*/,        "VGA Graphical tests (running off CPU not accel'd card).");
    KCreateCommand("info",          CMD_Info        /*kernel.cpp*/,     "Show information about the kernel & system.");
    KCreateCommand("argtest",       CMD_ArgTest     /*stdio.cpp*/,      "Test commandline argument passing stuff.");
    KCreateCommand("acpi_debug",    CMD_ACPI_debug  /*acpi.cpp*/,       "Show debug information for ACPI code.");
    KCreateCommand("time",          CMD_Time        /*rtc.cpp*/,        "Show the time.");
    KCreateCommand("cls",           CMD_cls         /*stdio.cpp*/,      "Clear the screen.");
    KCreateCommand("_forcebrk",     CMD_ForceBreak  /*kernel.cpp*/,     "Create an exception.");
    KCreateCommand("pci",           CMD_pci         /*pci.cpp*/,        "PCI bus controller (-D -B -S -RTL).");
    KCreateCommand("winstart",      CMD_start_win   /*windows.cpp*/,    "Start the basic window system.");
    KCreateCommand("refresh",       CMD_refresh_win /*windows.cpp*/,    "Refresh the windows.");

    if (ata_enabled) {
        KCreateCommand("ata",           CMD_ata         /*ata.cpp*/,        "Some ATA tests.");
        KCreateCommand("cd",            CMD_cd          /*ext2.cpp*/,       "Change directory.");
        KCreateCommand("ls",            CMD_ls          /*ext2.cpp*/,       "List files in current directory.");
        KCreateCommand("cat",           CMD_cat         /*ext2.cpp*/,       "File read.");
        KCreateCommand("nfile",         CMD_nfile       /*ext2.cpp*/,       "Create a new file.");
        KCreateCommand("atainit",       CMD_atainit     /*ata.cpp*/,        "Initialise an ATA drive (ata_init <P/S> <M/S>)");
        
        printf("%3tinsel v1%0 ready.\n%s:%s# ", ide_drive_name(ata_get_main_dev()), ext2_current_dir());
    } else {
        printf("ATA is disabled for this session!\n");

        printf("%3tinsel v1%0 ready.\nTiOS# ");
    }

    buf = (char*)kmalloc(MAX_INPUT_LEN);

    while (!die) {
        win_main();
    }

    return;
}
