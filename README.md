# tinsel-os
A re-write of v-os, just done to higher standards.

## Notes:
- The mouse cursor, which is a single pixel, is not very complicated and just a proof of concept. Currently it can't do anything besides be seen on the screen, which unfortunately isn't very interesting. I added IRQs based on some old code I have and what's on osdev wiki - these work flawlessly as far as I can see. Only the mouse & keyboard IRQs are currently in use.
- Use `help` to view a list of the commands. Feel free to add your own commands! It's not hard as I've written a basic command handler system to simplify the process of processing commands. (see `src/shell_cmd.c` and `include/shell_cmd.h`)
- The memory manager that I've written is not good. I've made a best guess as to where nothing on the system will be using memory, but that's really not good enough. The functions `kmalloc` and `kfree` do work, however, so for this purpose it works as intended. It also tracks "blocks" of memory which really should just be called structures to what's being used where, since the size of a "block" is not predetermined.

## Images:
Most recent:
![](https://i.imgur.com/Yv2uWph.png)
![](https://i.imgur.com/zVE4Bmr.png)
![](https://i.imgur.com/xBxOUOC.png)

First screenshot of it working:
![](https://i.imgur.com/iHYrxlH.png)

## PS:
This is meant to be a personal project for understanding modern computer systems, not a completely functioning OS that does everything a kernel like Linux or NT or OSX can, so please don't judge me for practising extremely bad OS programming or bad methods of doing things.
