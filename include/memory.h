#ifndef MEMORY_H
#define MEMORY_H

#include <cdefs.h>

#define MAX_MEMORY_BLOCKS 512 // gotta be a constant for now i'm afraid.

extern uint32_t end_sym;

typedef struct memblock_t {
    void*   location    = 0x0;
    bool    free        = true;
    size_t  bytes       = 0x0;
};

void    memory_init         (uint32_t start_offset);
size_t  find_free_block     (size_t min_size);
void*   kmalloc             (size_t bytes);
void    kfree               (void* allocated_position);
size_t  kmalloc_used_bytes  ();
size_t  kmalloc_used_blocks ();
size_t  kmalloc_unused_blocks();
bool    CMD_kmalloc         (char* inp);

#endif
