#include <stdint.h>
#include "memory.h"
#include "../stdlib/stdio/stdio.h"
#include "../multiboot.h"

void init_memory(){
    return;
}



void print_mmaps(struct multiboot_info* boot_info) {
    for (uint32_t i = 0; i < boot_info->mmap_length; i += sizeof(struct multiboot_mmap_entry)) {
        struct multiboot_mmap_entry *mmme = (struct multiboot_mmap_entry *) (boot_info->mmap_addr + i);
        printf("addr: %x | length: %x | size: %x | Type: %x\n", mmme->addr_low, mmme->len_low, mmme->size, mmme->type);
    }
}