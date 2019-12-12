bits 32
section .text

global kbd_int
extern cpp_kbd_handler
kbd_int:
	cli
	pusha                          ; make sure you don't damage current state
	call cpp_kbd_handler
	mov al,20h
	out 20h,al                     ; acknowledge the interrupt to the PIC
	popa                           ; restore state
	sti
	iret                           ; return to code executed before.

global mouse_int
extern cpp_mouse_handler
mouse_int:
	cli
	pusha    	                   ; make sure you don't damage current state
	call cpp_mouse_handler
	popa                           ; restore state
	sti
	iret                           ; return to code executed before.

global rtc_int
extern cpp_rtc_handler
rtc_int:
	cli
	pusha
	call cpp_rtc_handler
	popa
	sti
	iret

global ide_primary_int
extern cpp_ide_primary
ide_primary_int:
    cli
    pusha
    call cpp_ide_primary
    popa
    sti
    iret

global ide_secondary_int
extern cpp_ide_secondary
ide_secondary_int:
    cli
    pusha
    call cpp_ide_secondary
    popa
    sti
    iret

global null_handlr
null_handlr:
	mov al,20h
	out 20h,al ; acknowledge the interrupt to the PIC
	iret
