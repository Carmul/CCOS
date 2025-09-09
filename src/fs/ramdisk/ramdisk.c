#include "ramdisk.h"
#include "../../memory/memory.h"
#include "../../stdlib/memutil/memutil.h"

static uint8_t block_bitmap[RAMDISK_MAX_BLOCKS / 8];
static uint32_t blocks_allocated;

void ramdisk_init(void) {
    // Clear bitmap (all blocks free)
    memset(block_bitmap, 0, sizeof(block_bitmap));
    blocks_allocated = 0;    
}

static bool is_block_used(uint32_t block_num) {
    if (block_num >= RAMDISK_MAX_BLOCKS) return true;
    uint32_t byte_idx = block_num / 8;
    uint32_t bit_idx = block_num % 8;
    return (block_bitmap[byte_idx] & (1 << bit_idx)) != 0;
}

static void set_block_used(uint32_t block_num, bool used) {
    if (block_num >= RAMDISK_MAX_BLOCKS) return;
    uint32_t byte_idx = block_num / 8;
    uint32_t bit_idx = block_num % 8;
    
    if (used) {
        block_bitmap[byte_idx] |= (1 << bit_idx);
    } else {
        block_bitmap[byte_idx] &= ~(1 << bit_idx);
    }
}

int ramdisk_alloc_block(void) {
    for (uint32_t i = 0; i < RAMDISK_MAX_BLOCKS; i++) {
        if (!is_block_used(i)) {
            set_block_used(i, true);

            uint32_t phy = alloc_pframe();
            map_pframe(RAMDISK_BASE_ADDR + blocks_allocated * RAMDISK_BLOCK_SIZE, phy, PAGE_FLAG_WRITE);
            blocks_allocated++;

            return i;
        }
    }
    return -1; // No free blocks
}

void ramdisk_free_block(uint32_t block_num) {
    set_block_used(block_num, false);
}

void* ramdisk_get_block_ptr(uint32_t block_num) {
    if (block_num >= RAMDISK_MAX_BLOCKS) return NULL;
    return (void*)(RAMDISK_BASE_ADDR + (block_num * RAMDISK_BLOCK_SIZE));
}
