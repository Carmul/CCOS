#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Private heap block header structure
typedef struct heap_block {
    uint32_t magic;                 // Magic number for corruption detection
    uint32_t size;                  // Size of this block (including header)
    uint32_t flags;                 // Block flags (free/used)
    struct heap_block* next;        // Next block in the list
    struct heap_block* prev;        // Previous block in the list
} heap_block_t;

// Private heap statistics
typedef struct heap_stats {
    uint32_t total_size;            // Total heap size
    uint32_t used_size;             // Currently allocated size
    uint32_t free_size;             // Currently free size
    uint32_t num_blocks;            // Number of blocks
    uint32_t num_allocs;            // Total allocations made
    uint32_t num_frees;             // Total frees made
} heap_stats_t;

// Public heap allocator interface
void heap_init(void);
void* kmalloc(uint32_t size);
void* kcalloc(uint32_t num, uint32_t size);
void* krealloc(void* ptr, uint32_t new_size);
void kfree(void* ptr);

// Debug and statistics functions
void heap_print_stats(void);
void heap_print_blocks(void);
bool heap_check_integrity(void);

// used only internaly
bool expand_heap(uint32_t min_size);
heap_block_t* find_free_block(uint32_t size);
void split_block(heap_block_t* block, uint32_t size);
void merge_free_blocks(heap_block_t* block);

#endif // HEAP_H