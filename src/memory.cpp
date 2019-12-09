#include <memory.h>
#include <stdio.h>

/*
This is my "simple" memory allocator for usage in this nanokernel.
It's very dirty and I can't recommend using it for anything other than maybe research? or fun? or programming???
Basically it's not perfect and it probably allocates to very important memory, mmap is needed for this tbh.
*/

uint32_t mempos     = 0x0;
size_t   next_block = 0;

memblock_t Blocks[MAX_MEMORY_BLOCKS];

void memory_init(uint32_t start_offset) {
    mempos = ((uint32_t)&end_sym) + start_offset;
}

size_t find_free_block(size_t min_size) {
    if (next_block == 0) {
        // nothing has been allocated just yet.
        return MAX_MEMORY_BLOCKS + 1;
    }

    for (size_t i = 0; i < next_block + 1; i++) {
        if ((Blocks[i].free) && (Blocks[i].bytes >= min_size)) {
            // found a free block that meets the criteria, return it's index in the block array.
            return i;
        }
    }

    // no free blocks could be found here.
    return MAX_MEMORY_BLOCKS + 1;
}

// Allocate some memory and track it "safely".
void* kmalloc(size_t bytes) {
    void* ret = (void*)mempos;

    size_t iter = find_free_block(bytes);
    if (iter != MAX_MEMORY_BLOCKS + 1) {
        Blocks[iter].free = false;
        return Blocks[iter].location;
    }
    else {
        // if a previously freed block that fits our number of bytes has not been found,
        // allocate a new location and block and set that block to not be free so it isn't overwritten
        // until the user is done with it.
        memblock_t new_block;
        new_block.location  = ret;
        new_block.free      = false;
        new_block.bytes     = bytes;

        Blocks[next_block] = new_block;
        next_block += 1;

        mempos = mempos + bytes;
}
    return ret;
}

void kfree(void* allocated_position) {
    size_t i;
    bool found = false;

    if (!allocated_position) {
        return;
    }

    for (i = 0; i < next_block + 1; i++) {
        if ((Blocks[i].location == allocated_position) && (!Blocks[i].free)) {
            found = true;
            break;
        }
    }

    if (!found) return;

    Blocks[i].free = true;
}

size_t kmalloc_used_bytes() {
    size_t output = 0x0;

    for (size_t i = 0; i < next_block + 1; i++) {
        if (!Blocks[i].free) output += Blocks[i].bytes;
    }

    return output;
}

size_t kmalloc_used_blocks() {
    size_t output = 0x0;
    for (size_t i = 0; i < MAX_MEMORY_BLOCKS; i++) { if (!(Blocks[i].free)) output ++; }
    return output;
}

size_t kmalloc_unused_blocks() {
    size_t output = 0x0;
    for (size_t i = 0; i < MAX_MEMORY_BLOCKS; i++) { if ((Blocks[i].free)) output ++; }
    return output;
}

// C++ class operators
void *operator new(size_t size) {
    return kmalloc(size);
}

void *operator new[](size_t size) {
    return kmalloc(size);
}

void operator delete(void *p) {
    kfree(p);
}

void operator delete[](void *p) {
    kfree(p);
}


const char* bruh = "Hello!";
bool CMD_kmalloc(int argc, char** argv) {
    // Each time you run this, the memory address (theoretically) should not change because of the kfree implementation
    // If it does, something may have gone wrong in the memory allocation process.

    // Allocate us some bytes, Mr. Kernel.
    char* test = (char*)kmalloc(strlen(bruh));

    // Copy over the string into the newly allocated bytes
    strcpy(test, bruh);

    // Output the new string and it's memory location.
    printf("[%5Test%0] %s (0x%x)\n", test, (uint32_t)test);

    // Free the previously allocated memory.
    kfree((void*)test);

    // Output the current allocator usage.
    printf("[%5Info%0] malloc usage: %uB\n", kmalloc_used_bytes());

    return true;
}
