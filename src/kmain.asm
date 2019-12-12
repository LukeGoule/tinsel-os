bits 32
section .text
%define FL_VIDEO (1 << 2)
%define FLAGS FL_VIDEO
%define SCREEN_WIDTH 1024
%define SCREEN_HEIGHT 768
align 4
	dd 0x1BADB002
	dd FLAGS
	dd - (0x1BADB002+FLAGS)
	dd 0,0,0,0,0,0,SCREEN_WIDTH,SCREEN_HEIGHT,32

global start
global gdt_install
extern kmain

start:
	mov esp, 0x7FFFF
	push esp

	push ebx ; multiboot info structure
	push eax ; magic number

	cli

	call kmain

    jmp $

gdt_install:
	lgdt [GDTP]

	mov eax, cr0
	or eax, 0x1
	mov cr0, eax

	; Do a far jump to flush the code register.
	jmp 0x08:switch_pmode

switch_pmode:
  	mov ax, 0x10
  	mov ds, ax
  	mov ss, ax
  	mov es, ax
  	mov fs, ax
  	mov gs, ax

  	ret

global tss_flush
tss_flush:
	mov ax, 0x2B
	ltr ax
	ret


; Thank you Indian man on YouTube who was kind enough to share with us his working code :)
GDT:
	dq 00000000000000000h    	; null 8-bytes entrie
  	dw 0FFFFh      				; limit low word
  	dw 00000h      				; base low word
  	db 000h          			; base middle byte
  	db 010011010b   			; access byte (code, readable, system, unconforming)
  	db 011001111b  	 			; 4 granularity (4Kb) bits and limit high 4 bits
  	db 000h             		; base high byte
  	dw 0FFFFh      				; limit low word
  	dw 00000h      				; base low word
  	db 000h          			; base middle byte
  	db 010010010b   			; access byte (data, writeable, system, unconforming)
  	db 011001111b  				; 4 granularity (4Kb) bits and limit high 4 bits
  	db 000h            			; base high byte
GDTP:              				; GDT pointer declaration
  	dw GDTP-GDT    				; GDT limit = GDT end - GDT init - 1
  	dd GDT             			; 32-bit offset of GDT

; BSS Section
SECTION .bss
	resb 8192 ; 8KB of memory reserved
