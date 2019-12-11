#include <exceptions.h>
#include <stdio.h>
#include <cpu.h>

const char* exception_messages[34] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

extern "C" void exception_hndlr(register_t* regs) {
    printf("ds:%x edi:%x esi:%x ebp:%x esp:%x\nebx:%x edx:%x ecx:%x eax:%x\n", regs->ds, regs->edi, regs->esi, regs->ebp, regs->esp, regs->ebx, regs->edx, regs->ecx, regs->eax);
    printf("int_no:%u err_code:%u\n", regs->int_no, regs->err_code);
    printf("eip:%x cs:%x eflags:%x useresp:%x ss:%x\n", regs->eip, regs->cs, regs->eflags, regs->useresp, regs->ss);
    __asm__("hlt");
}

#define IDT_DEF(x, y) void idt ## x () {printf("Interrupt(%d): %s\n", x, exception_messages[x]);if (y) {__asm__("hlt");}}
#define GATE_DEF(x) set_idt_gate(x, idt ## x);
#define NULL_IRQ(x) set_idt_gate(x, null_handlr);

IDT_DEF(0, false);
IDT_DEF(1, false);
IDT_DEF(2, true);
IDT_DEF(3, false);
IDT_DEF(4, false);
IDT_DEF(5, false);
IDT_DEF(6, false);
IDT_DEF(7, false);
IDT_DEF(8, true);
IDT_DEF(9, false);
IDT_DEF(10, false);
IDT_DEF(11, false);
IDT_DEF(12, false);
IDT_DEF(13, false);
IDT_DEF(14, false);
IDT_DEF(15, false);
IDT_DEF(16, false);
IDT_DEF(17, false);
IDT_DEF(18, true);
IDT_DEF(19, false);
IDT_DEF(20, false);
IDT_DEF(21, false);
IDT_DEF(22, false);
IDT_DEF(23, false);
IDT_DEF(24, false);
IDT_DEF(25, false);
IDT_DEF(26, false);
IDT_DEF(27, false);
IDT_DEF(28, false);
IDT_DEF(29, false);
IDT_DEF(30, false);
IDT_DEF(31, false);

extern "C" void kbd_int();
extern "C" void mouse_int();
extern "C" void null_handlr();

void isr_init() {
    GATE_DEF(0);
    GATE_DEF(1);
    GATE_DEF(2);
    GATE_DEF(3);
    GATE_DEF(4);
    GATE_DEF(5);
    GATE_DEF(6);
    GATE_DEF(7);
    GATE_DEF(8);
    GATE_DEF(9);
    GATE_DEF(10);
    GATE_DEF(11);
    GATE_DEF(12);
    GATE_DEF(13);
    GATE_DEF(14);
    GATE_DEF(15);
    GATE_DEF(16);
    GATE_DEF(17);
    GATE_DEF(18);
    GATE_DEF(19);
    GATE_DEF(20);
    GATE_DEF(21);
    GATE_DEF(22);
    GATE_DEF(23);
    GATE_DEF(24);
    GATE_DEF(25);
    GATE_DEF(26);
    GATE_DEF(27);
    GATE_DEF(28);
    GATE_DEF(29);
    GATE_DEF(30);
    GATE_DEF(31);

    /* remapping the PIC */
	outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 40);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);

    NULL_IRQ(32);               // irq0
    set_idt_gate(33, kbd_int);  // irq1
    NULL_IRQ(34);               // irq2
    NULL_IRQ(35);               // irq3
    NULL_IRQ(36);               // irq4
    NULL_IRQ(37);               // irq5
    NULL_IRQ(38);               // irq6
    NULL_IRQ(39);               // irq7
    NULL_IRQ(40);               // irq8
    NULL_IRQ(41);               // irq9
    NULL_IRQ(42);               // irq10
    NULL_IRQ(43);               // irq11
    set_idt_gate(44, mouse_int);// irq12
    NULL_IRQ(45);               // irq13
    NULL_IRQ(46);               // irq14
    NULL_IRQ(47);               // irq15

    set_idt(); // Load with ASM

    __asm__("sti"); // enable interrupts
}
