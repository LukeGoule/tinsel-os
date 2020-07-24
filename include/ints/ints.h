#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define _ENABLE_INTS  __asm__("cli");
#define _DISABLE_INTS __asm__("sti");

// also known as IRQ ack
#define _END_OF_IRQ(x)  if (x >= 8) { \
                            outportb(0x20,0x20); \
                            outportb(0xa0,0x20); \
                        } else { \
                            outportb(0x20,0x20); \
                        }

#define IDT_DEF(x, y) void idt ## x () {printf("Interrupt(%d): %s\n", x, exception_messages[x]);if (y) {__asm__("hlt");}}
#define GATE_DEF(x) set_idt_gate(x, idt ## x);
#define NULL_IRQ(x) set_idt_gate(x, null_handlr);


#endif
