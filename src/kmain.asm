bits 32
section .text
%define FL_VIDEO (1 << 2)
%define FLAGS FL_VIDEO
align 4
	dd 0x1BADB002
	dd FLAGS
	dd - (0x1BADB002+FLAGS)
	dd 0,0,0,0,0,0,1280,720,32

global start
extern kmain

start:
	mov esp, 0x7FFFF
	push esp

	push ebx ; multiboot info structure
	push eax ; magic number

	cli

	call kmain

    jmp $

global tss_flush
tss_flush:
	mov ax, 0x2B
	ltr ax
	ret

; BSS Section
SECTION .bss
	resb 8192 ; 8KB of memory reserved
