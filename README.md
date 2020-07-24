# tinsel-os
A re-write of v-os, just done to higher standards.

24/7/2020: updated the makefile to add some notes for getting it up and running, removed some errors for when you aren't running this on a VM with a hard disk, looked at the "window" system a bit more.

## Notes:
- Getting the ATA "driver" working was pretty cool. It can now read files from an ext2 formatted hard disk image. I moved to VirtualBox over Qemu for this since I couldn't get Qemu to emulate an IDE device. (Here's a link to the hard drive image I created: https://anonfile.com/j7F1daF0nd/ext2-4_vdi). Currently this OS has no practical application and I don't think it ever will since I don't want to waste time creating some SCSI driver. IDE is fine for now. Code samples from other GitHub projects have been used to create the ext2 and ide drivers since I didn't know what I was doing properly.

- The mouse cursor, which is a single pixel, is not very complicated and just a proof of concept. Currently it can't do anything besides be seen on the screen, which unfortunately isn't very interesting. I added IRQs based on some old code I have and what's on osdev wiki - these work flawlessly as far as I can see.

- Use `help` to view a list of the commands. Feel free to add your own commands! It's not hard as I've written a basic command handler system to simplify the process of processing commands. (see `src/shell_cmd.c` and `include/shell_cmd.h`)

- The memory manager that I've written is not good. I've made a best guess as to where nothing on the system will be using memory, but that's really not good enough. The functions `kmalloc` and `kfree` do work, however, so for this purpose it works as intended. It also tracks "blocks" of memory which really should just be called structures to what's being used where, since the size of a "block" is not predetermined.

## Images:
Most recent:
![](https://i.imgur.com/9i0w7rG.png)
![](https://i.imgur.com/Yv2uWph.png)
![](https://i.imgur.com/zVE4Bmr.png)
![](https://i.imgur.com/xBxOUOC.png)

First screenshot of it working:
![](https://i.imgur.com/iHYrxlH.png)

## PS:
This is meant to be a personal project for understanding modern computer systems, not a completely functioning OS that does everything a kernel like Linux or NT or OSX can, so please don't judge me for practising extremely bad OS programming or bad methods of doing things.
