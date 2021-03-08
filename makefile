## tinsel OS, a c++ operating system built for fun.

COMPILER = g++
LINKER = ld
ASSEMBLER = nasm
CFLAGS = -m32 -c -ffreestanding -w -I include -g -fpermissive -fno-rtti -fno-exceptions
ASFLAGS = -f elf32
LDFLAGS = -m elf_i386 -T src/linker.ld
EMULATOR = qemu-system-i386
EFLAGS = -m 512 -vga std -display gtk -s -netdev user,id=vmnic -device rtl8139,netdev=vmnic -machine type=q35,accel=kvm -cpu host,check -smp 12 -enable-kvm drive.img\
 #-drive id=disk,file="/root/VirtualBox VMs/TINSEL1/ext2.img",if=none -device ide-drive,drive=disk

KVERSION = tinsel_0
OBJS = obj/kmainasm.o obj/intasm.o obj/kmainc.o obj/vga.o obj/stdio.o obj/input.o obj/acpi.o obj/cpu.o obj/memory.o obj/shell_cmd.o obj/rtc.o obj/icxxabi.o obj/idt.o obj/isr.o obj/ata.o obj/pci.o obj/mouse.o obj/windows.o obj/ext2.o obj/bitmap.o
OUTPUT = tinsel_grub/boot/$(KVERSION).bin

#setup:
#	apt update
#	apt install mtools
#	apt install grub
#	apt install 

run: all
	@grub-mkrescue -o tinsel.iso tinsel_grub
	@$(EMULATOR) $(EFLAGS) -cdrom tinsel.iso #-boot d ### switched to virtualbox for it's easy hard drive support.
all:$(OBJS)
	@$(LINKER) $(LDFLAGS) -o $(OUTPUT) $(OBJS)

#git:
	@#git pull
	@#git add *
	@#git commit -m "Update $(KVERSION)"
	@#git push

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

obj/icxxabi.o:src/icxxabi.cpp
	@$(COMPILER) $(CFLAGS) -o obj/icxxabi.o src/icxxabi.cpp
	@echo "[COMPILE]: icxxabi.cpp"

## INTERRUPTS
obj/intasm.o:src/ints/ints.asm
	@$(ASSEMBLER) $(ASFLAGS) -o  obj/intasm.o src/ints/ints.asm

obj/isr.o:src/ints/isr.cpp
	@$(COMPILER) $(CFLAGS) -o obj/isr.o src/ints/isr.cpp
	@echo "[COMPILE]: ints/isr.cpp"

obj/idt.o:src/ints/idt.cpp
	@$(COMPILER) $(CFLAGS) -o obj/idt.o src/ints/idt.cpp
	@echo "[COMPILE]: ints/idt.cpp"

obj/pci.o:src/pci.cpp
	@$(COMPILER) $(CFLAGS) -o obj/pci.o src/pci.cpp
	@echo "[COMPILE]: pci.cpp"

obj/windows.o:src/windows.cpp
	@$(COMPILER) $(CFLAGS) -o obj/windows.o src/windows.cpp
	@echo "[COMPILE]: windows.cpp"

## FILES
obj/ext2.o:src/fs/ext2.cpp
	@$(COMPILER) $(CFLAGS) -o obj/ext2.o src/fs/ext2.cpp
	@echo "[COMPILE]: fs/ext2.cpp"

obj/bitmap.o:src/files/bitmap.cpp
	@$(COMPILER) $(CFLAGS) -o obj/bitmap.o src/files/bitmap.cpp
	@echo "[COMPILE]: files/bitmap.cpp"


## HARDWARE
obj/ata.o:src/hardware/ata.cpp
	@$(COMPILER) $(CFLAGS) -o obj/ata.o src/hardware/ata.cpp
	@echo "[COMPILE]: hardware/ata.cpp"

obj/mouse.o:src/hardware/mouse.cpp
	@$(COMPILER) $(CFLAGS) -o obj/mouse.o src/hardware/mouse.cpp
	@echo "[COMPILE]: hardware/mouse.cpp"
