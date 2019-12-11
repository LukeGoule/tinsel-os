#ifndef ISR_H
#define ISR_H

#include <cdefs.h>

#define IDT_ENTRIES 256
#define KERNEL_CS 0x08
#define low_16(address)     (uint16_t)((address) & 0xFFFF)
#define high_16(address)    (uint16_t)(((address) >> 16) & 0xFFFF)

typedef struct registers
{
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} register_t;

typedef struct {
    uint16_t    low_offset;
    uint16_t    sel;
    uint8_t     always0;
    uint8_t     flags;
    uint16_t    high_offset;
} __attribute__((packed)) idt_gate_t ;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_register_t;

/* Functions implemented in idt.c */
void set_idt_gate(int n, uint32_t handler);
void set_idt();

void isr_init();

#endif
