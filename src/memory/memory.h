#ifndef MEMORY_H
#define MEMORY_H

#include "../multiboot.h"

#define KERNEL_START 0xC0000000
#define REC_PAGEDIR ((uint32_t*)0xFFFFF000)
#define REC_PAGETABLE(i) ((uint32_t*) (0xFFC00000 + ((i) << 12)))

#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE (1 << 1)
#define PAGE_FLAG_OWNER (1 << 9)

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

extern uint32_t initial_page_dir[1024];

void memory_init(struct multiboot_info* boot_info);
void print_mmaps(struct multiboot_info* boot_info);
uint32_t get_physical_alloc_start(struct multiboot_info* bootInfo);
void pmm_init(uint32_t mem_low, uint32_t mem_high);
uint32_t alloc_pframe();

uint32_t* get_current_page_dir();
void change_page_dir(uint32_t* pd);
void sync_page_dirs();
void map_pframe(uint32_t virutalAddr, uint32_t physAddr, uint32_t flags);

void invalidate(uint32_t vaddr);

#endif