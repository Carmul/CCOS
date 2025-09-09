#ifndef RAMDISK_H
#define RAMDISK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RAMDISK_BASE_ADDR 0xE0000000
#define RAMDISK_BLOCK_SIZE 0x1000 // page size
#define RAMDISK_MAX_SIZE 0x10000000
#define RAMDISK_MAX_BLOCKS (RAMDISK_MAX_SIZE / RAMDISK_BLOCK_SIZE)

// Function prototypes
void ramdisk_init(void);
int ramdisk_alloc_block(void);
void ramdisk_free_block(uint32_t block_num);
void* ramdisk_get_block_ptr(uint32_t block_num);

#endif