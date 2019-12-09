## tinsel OS, a c++ operating system built for fun.

COMPILER = g++
LINKER = ld
ASSEMBLER = nasm
CFLAGS = -m32 -c -ffreestanding -w -I include -g -fpermissive
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T src/linker.ld
EMULATOR = qemu-system-i386
EFLAGS = -m 512 -vga std -display gtk -s -netdev user,id=vmnic -device rtl8139,netdev=vmnic -machine type=q35,accel=kvm -cpu host,check -smp 12 -enable-kvm
KVERSION = tinsel_0
OBJS = obj/kmainasm.o obj/kmainc.o obj/vga.o obj/stdio.o obj/input.o obj/acpi.o obj/cpu.o obj/memory.o obj/shell_cmd.o obj/rtc.o
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

obj/kmainc.o:src/kmain.cpp
	@$(COMPILER) $(CFLAGS) -o obj/kmainc.o src/kmain.cpp
	@echo "[COMPILE]: kmain.cpp"

obj/vga.o:src/vga.cpp
	@$(COMPILER) $(CFLAGS) -o obj/vga.o src/vga.cpp
	@echo "[COMPILE]: vga.cpp"

obj/stdio.o:src/stdio.cpp
	@$(COMPILER) $(CFLAGS) -o obj/stdio.o src/stdio.cpp
	@echo "[COMPILE]: stdio.cpp"

obj/input.o:src/input.cpp
	@$(COMPILER) $(CFLAGS) -o obj/input.o src/input.cpp
	@echo "[COMPILE]: input.cpp"

obj/acpi.o:src/acpi.cpp
	@$(COMPILER) $(CFLAGS) -o obj/acpi.o src/acpi.cpp
	@echo "[COMPILE]: acpi.cpp"

obj/cpu.o:src/cpu.cpp
	@$(COMPILER) $(CFLAGS) -o obj/cpu.o src/cpu.cpp
	@echo "[COMPILE]: cpu.cpp"

obj/memory.o:src/memory.cpp
	@$(COMPILER) $(CFLAGS) -o obj/memory.o src/memory.cpp
	@echo "[COMPILE]: memory.cpp"

obj/shell_cmd.o:src/shell_cmd.cpp
	@$(COMPILER) $(CFLAGS) -o obj/shell_cmd.o src/shell_cmd.cpp
	@echo "[COMPILE]: shell_cmd.cpp"

obj/rtc.o:src/rtc.cpp
	@$(COMPILER) $(CFLAGS) -o obj/rtc.o src/rtc.cpp
	@echo "[COMPILE]: rtc.cpp"
