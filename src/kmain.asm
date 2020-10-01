;; kmain.asm :  Responsible for the header of the kernel.
;				Tells the multiboot compliant bootloader what to do
;				before the bootloader hands over control to the kernel. 

bits 32			; 32 bit code
section .text	; code not data.

; This flag tells the multiboot compliant bootloader that we want VGA graphics enabled.
%define FL_VIDEO 		(1 << 2)

; These are the flags we want the bootloader to see
%define FLAGS 			FL_VIDEO

; These sort of speak for themselves
%define SCREEN_WIDTH 	1024
%define SCREEN_HEIGHT 	768
%define SCREEN_DEPTH    32

align 4
	; This number is the magic number which must be the first 4 bytes of the kernel
	; It tells the bootloader which version of the multiboot spec we want.
	dd 0x1BADB002
	
	; The next 4 bytes are the flags which the bootloader reads to setup & give the kernel data.
	dd FLAGS

	; These 4 bytes sign the header, ensuring it is correct.
	dd - (0x1BADB002+FLAGS)

	; These are the options for the enabled flags. For now I have only enabled VGA video so
	; the only options I have set is the screen dimensions and pixel depth.
	dd 0, 0, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH

global start			; first function in the kernel, called by the bootloader by start(magic number, multiboot structure.)
global gdt_install		; unused iirc.
extern kmain			; the C++ kernel entry point. 

start:
	; Setup the stack at 0x7FFFF
	mov esp, 0x7FFFF
	push esp

	push ebx ; multiboot info structure
	push eax ; magic number

	; Clear interrupts
	cli
	
	; kmain(int magic number, void* multiboot ptr)
	; see kmain.cpp (really bad file naming huh)
	call kmain

	; while (1); - really the operating system should never reach this point as there are already while loops within kmain.
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
	dq 00000000000000000h    	; Null 8-bytes entry
  	dw 0FFFFh      				; Limit low word
  	dw 00000h      				; Base low word
  	db 000h          			; Base middle byte
  	db 010011010b   			; Access byte (code, readable, system, unconforming)
  	db 011001111b  	 			; 4 Granularity (4Kb) bits and limit high 4 bits
  	db 000h             		; Base high byte
  	dw 0FFFFh      				; Limit low word
  	dw 00000h      				; Base low word
  	db 000h          			; Base middle byte
  	db 010010010b   			; Access byte (data, writeable, system, unconforming)
  	db 011001111b  				; 4 Granularity (4Kb) bits and limit high 4 bits
  	db 000h            			; Base high byte
GDTP:              				; GDT Pointer declaration
  	dw GDTP-GDT    				; GDT Limit = GDT end - GDT init - 1
  	dd GDT             			; 32-bit offset of GDT

; BSS Section
SECTION .bss
	resb 8192 ; 8KB of memory reserved
