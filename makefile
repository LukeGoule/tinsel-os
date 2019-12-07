COMPILER = g++
LINKER = ld
ASSEMBLER = nasm
CFLAGS = -m32 -c -ffreestanding -w -I include -g -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T src/linker.ld
EMULATOR = qemu-system-i386
EFLAGS = -m 512 -vga std -display gtk -s -netdev user,id=vmnic -device rtl8139,netdev=vmnic -machine type=q35,accel=kvm -cpu host,check -smp 12 -enable-kvm
KVERSION = tinsel_0
OBJS = obj/kmainasm.o obj/kmainc.o obj/vga.o obj/stdio.o obj/input.o obj/acpi.o
OUTPUT = tinsel_grub/boot/$(KVERSION).bin

run: all
	@grub-mkrescue -o tinsel.iso tinsel_grub
	@$(EMULATOR) $(EFLAGS) -cdrom tinsel.iso -boot d
all:$(OBJS)
	@$(LINKER) $(LDFLAGS) -o $(OUTPUT) $(OBJS)

#git:
	#@git pull
	#@git add *
	#@git commit -m "Update $(KVERSION)"
	#@git push

obj/kmainasm.o:src/kmain.asm
	@$(ASSEMBLER) $(ASFLAGS) -o obj/kmainasm.o src/kmain.asm
	@echo "[ASSEMBLE]: kmain.asm"

obj/kmainc.o:src/kmain.c
	@$(COMPILER) $(CFLAGS) -o obj/kmainc.o src/kmain.c
	@echo "[COMPILE]: kmain.c"

obj/vga.o:src/vga.c
	@$(COMPILER) $(CFLAGS) -o obj/vga.o src/vga.c
	@echo "[COMPILE]: vga.c"

obj/stdio.o:src/stdio.c
	@$(COMPILER) $(CFLAGS) -o obj/stdio.o src/stdio.c
	@echo "[COMPILE]: stdio.c"

obj/input.o:src/input.c
	@$(COMPILER) $(CFLAGS) -o obj/input.o src/input.c
	@echo "[COMPILE]: input.c"

obj/acpi.o:src/acpi.c
	@$(COMPILER) $(CFLAGS) -o obj/acpi.o src/acpi.c
	@echo "[COMPILE]: acpi.c"
