#include "heap.h"
#include "../memory/memory.h"  
#include "../stdlib/stdio/stdio.h"
#include "../stdlib/memutil/memutil.h"

// heap configuration
#define HEAP_START          0xD0000000  
#define HEAP_MAX_SIZE       0x10000000  // 256MB max heap size
#define HEAP_MIN_SIZE       0x100000    // 1MB initial heap size
#define HEAP_BLOCK_SIZE     0x1000      // 4KB blocks (page size)

// block header flags
#define HEAP_BLOCK_FREE     0x1
#define HEAP_BLOCK_USED     0x2
#define HEAP_MAGIC          0xDEADBEEF

// private global heap state - only accessible within this file
static struct {
    heap_block_t* first_block;
    uint32_t current_size;          
    bool initialized;               
    heap_stats_t stats;             
} heap;


void heap_init(void) {
    if (heap.initialized) {
        return;
    }
    // map initial heap pages
    uint32_t pages_needed = HEAP_MIN_SIZE / HEAP_BLOCK_SIZE;
    
    for (uint32_t i = 0; i < pages_needed; i++) {
        uint32_t vaddr = HEAP_START + (i * HEAP_BLOCK_SIZE);
        uint32_t paddr = alloc_pframe();
        
        map_pframe(vaddr, paddr, PAGE_FLAG_WRITE);
    }

    // initialize the first free block
    heap.first_block = (heap_block_t*)HEAP_START;
    heap.first_block->magic = HEAP_MAGIC;
    heap.first_block->size = HEAP_MIN_SIZE - sizeof(heap_block_t);
    heap.first_block->flags = HEAP_BLOCK_FREE;
    heap.first_block->next = NULL;
    heap.first_block->prev = NULL;

    heap.current_size = HEAP_MIN_SIZE;
    heap.initialized = true;

    // initialize statistics
    heap.stats.total_size = HEAP_MIN_SIZE;
    heap.stats.used_size = 0;
    heap.stats.free_size = HEAP_MIN_SIZE - sizeof(heap_block_t);
    heap.stats.num_blocks = 1;
    heap.stats.num_allocs = 0;
    heap.stats.num_frees = 0;

    // printf("Heap initialized: start=0x%x, size=%dKB\n", HEAP_START, HEAP_MIN_SIZE / 1024);
}

void* kmalloc(uint32_t size) {
    if (!heap.initialized) {
        heap_init();
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // align size to 8-byte boundary for better performance
    size = ALIGN(size, 8);
    
    heap_block_t* block = find_free_block(size);
    
    // if no suitable block found, try to expand the heap
    if (!block) {
        if (!expand_heap(size + sizeof(heap_block_t))) {
            printf("kmalloc: Out of memory (requested %d bytes)\n", size);
            return NULL;
        }
        block = find_free_block(size);
        
        if (!block) {
            printf("kmalloc: Still no suitable block after expansion\n");
            return NULL;
        }
    }
    
    // split the block if it's much larger than needed
    split_block(block, size);
    
    // mark the block as used
    block->flags = HEAP_BLOCK_USED;
    
    // update statistics
    heap.stats.used_size += block->size;
    heap.stats.free_size -= block->size;
    heap.stats.num_allocs++;
    
    // return pointer to the data area (after the header)
    return (void*)((uint8_t*)block + sizeof(heap_block_t));
}

void* kcalloc(uint32_t num, uint32_t size) {
    uint32_t total_size = num * size;
    void* ptr = kmalloc(total_size);
    
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

void* krealloc(void* ptr, uint32_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    // get the block header
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    
    // verify magic number
    if (block->magic != HEAP_MAGIC) {
        printf("krealloc: Invalid pointer or corrupted block\n");
        return NULL;
    }
    
    new_size = ALIGN(new_size, 8);
    
    // if the new size fits in the current block, just return the same pointer
    if (new_size <= block->size) {
        return ptr;
    }
    
    // allocate a new block
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    // copy the old data
    memcpy(new_ptr, ptr, block->size < new_size ? block->size : new_size);
    
    // free the old block
    kfree(ptr);
    
    return new_ptr;
}

void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // get the block header
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    
    // verify magic number
    if (block->magic != HEAP_MAGIC) {
        printf("kfree: Invalid pointer or corrupted block at %p\n", ptr);
        return;
    }
    
    // verify the block is actually used
    if (block->flags & HEAP_BLOCK_FREE) {
        printf("kfree: Double free detected at %p\n", ptr);
        return;
    }
    
    // mark as free
    block->flags = HEAP_BLOCK_FREE;
    
    // update statistics
    heap.stats.used_size -= block->size;
    heap.stats.free_size += block->size;
    heap.stats.num_frees++;
    
    // merge with adjacent free blocks
    merge_free_blocks(block);
}

void heap_print_stats(void) {
    printf("\n=== Heap Statistics ===\n");
    printf("Total heap size:    %d KB\n", heap.stats.total_size / 1024);
    printf("Used memory:        %d KB\n", heap.stats.used_size / 1024);
    printf("Free memory:        %d KB\n", heap.stats.free_size / 1024);
    printf("Fragmentation:      %.1f%%\n", 
           (float)(heap.stats.total_size - heap.stats.used_size - heap.stats.free_size) * 100.0f / heap.stats.total_size);
    printf("Number of blocks:   %d\n", heap.stats.num_blocks);
    printf("Total allocations:  %d\n", heap.stats.num_allocs);
    printf("Total frees:        %d\n", heap.stats.num_frees);
    printf("=======================\n\n");
}

void heap_print_blocks(void) {
    printf("\n=== Heap Block List ===\n");
    heap_block_t* current = heap.first_block;
    int block_num = 0;
    
    while (current) {
        printf("Block %d: addr=0x%x, size=%d, %s\n", 
               block_num++, 
               (uint32_t)current,
               current->size,
               (current->flags & HEAP_BLOCK_FREE) ? "FREE" : "USED");
        current = current->next;
    }
    printf("=======================\n\n");
}

bool heap_check_integrity(void) {
    heap_block_t* current = heap.first_block;
    uint32_t total_size = 0;
    uint32_t block_count = 0;
    
    while (current) {
        // check magic number
        if (current->magic != HEAP_MAGIC) {
            printf("Heap corruption: Invalid magic in block at 0x%x\n", (uint32_t)current);
            return false;
        }
        
        // check if next/prev pointers are consistent
        if (current->next && current->next->prev != current) {
            printf("Heap corruption: Inconsistent linked list at 0x%x\n", (uint32_t)current);
            return false;
        }
        
        total_size += current->size + sizeof(heap_block_t);
        block_count++;
        current = current->next;
    }
    
    // verify statistics
    if (block_count != heap.stats.num_blocks) {
        printf("Heap corruption: Block count mismatch (found %d, expected %d)\n", 
               block_count, heap.stats.num_blocks);
        return false;
    }
    
    printf("Heap integrity check passed (%d blocks, %d KB total)\n", 
           block_count, total_size / 1024);
    return true;
}

// functions used only internaly

bool expand_heap(uint32_t min_size) {
    // calculate how much need to expand
    uint32_t expansion_size = ALIGN(min_size, HEAP_BLOCK_SIZE);
    
    // check if would exceed maximum heap size
    if (heap.current_size + expansion_size > HEAP_MAX_SIZE) {
        printf("heap: Cannot expand beyond maximum size\n");
        return false;
    }

    // map new pages
    uint32_t pages_needed = expansion_size / HEAP_BLOCK_SIZE;
    uint32_t start_vaddr = HEAP_START + heap.current_size;

    // check if there is enough phisical memory
    extern uint32_t pframes_left;
    if(pframes_left < pages_needed){
        printf("heap: Cannot expand beyond maximum size\n");
        return false;
    }

    for (uint32_t i = 0; i < pages_needed; i++) {
        uint32_t vaddr = start_vaddr + (i * HEAP_BLOCK_SIZE);
        uint32_t paddr = alloc_pframe();
        
        if (paddr == 0) {
            printf("heap: Failed to allocate physical memory for expansion\n");
            return false;
        }
        
        map_pframe(vaddr, paddr, PAGE_FLAG_WRITE);
    }

    // create a new free block for the expanded area
    heap_block_t* new_block = (heap_block_t*)start_vaddr;
    new_block->magic = HEAP_MAGIC;
    new_block->size = expansion_size - sizeof(heap_block_t);
    new_block->flags = HEAP_BLOCK_FREE;
    new_block->next = NULL;
    new_block->prev = NULL;

    // find the last block and link the new block
    heap_block_t* current = heap.first_block;
    while (current->next) {
        current = current->next;
    }
    
    current->next = new_block;
    new_block->prev = current;

    // try to merge with the previous block if it's free
    if (current->flags & HEAP_BLOCK_FREE) {
        current->size += sizeof(heap_block_t) + new_block->size;
        current->next = new_block->next;
        if (new_block->next) {
            new_block->next->prev = current;
        }
        heap.stats.num_blocks--;
    } else {
        heap.stats.num_blocks++;
    }

    heap.current_size += expansion_size;
    heap.stats.total_size += expansion_size;
    heap.stats.free_size += expansion_size - sizeof(heap_block_t);

    // printf("Heap expanded by %dKB (total: %dKB)\n", expansion_size / 1024, heap.current_size / 1024);
    
    return true;
}

heap_block_t* find_free_block(uint32_t size) {
    heap_block_t* current = heap.first_block;
    heap_block_t* best_fit = NULL;
    
    // find best-fit
    while (current) {
        if ((current->flags & HEAP_BLOCK_FREE) && current->size >= size) {
            if (!best_fit || current->size < best_fit->size) {
                best_fit = current;
            }
            // if found an exact match, use it immediately
            if (current->size == size) {
                break;
            }
        }
        current = current->next;
    }
    
    return best_fit;
}

void split_block(heap_block_t* block, uint32_t size) {
    // only split if the remaining space is large enough for a new block
    if (block->size > size + sizeof(heap_block_t) + 16) {
        heap_block_t* new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
        
        new_block->magic = HEAP_MAGIC;
        new_block->size = block->size - size - sizeof(heap_block_t);
        new_block->flags = HEAP_BLOCK_FREE;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
        
        heap.stats.num_blocks++;
        heap.stats.free_size += sizeof(heap_block_t); // account for new header
    }
}

void merge_free_blocks(heap_block_t* block) {
    // merge with next block if it's free
    while (block->next && (block->next->flags & HEAP_BLOCK_FREE)) {
        heap_block_t* next_block = block->next;
        
        block->size += sizeof(heap_block_t) + next_block->size;
        block->next = next_block->next;
        
        if (next_block->next) {
            next_block->next->prev = block;
        }
        
        heap.stats.num_blocks--;
        heap.stats.free_size += sizeof(heap_block_t); // reclaimed header space
    }
    
    // merge with previous block if it's free
    if (block->prev && (block->prev->flags & HEAP_BLOCK_FREE)) {
        heap_block_t* prev_block = block->prev;
        
        prev_block->size += sizeof(heap_block_t) + block->size;
        prev_block->next = block->next;
        
        if (block->next) {
            block->next->prev = prev_block;
        }
        
        heap.stats.num_blocks--;
        heap.stats.free_size += sizeof(heap_block_t); // reclaimed header space
    }
}
