#include <stdint.h>
#include "memory.h"
#include "../stdlib/stdio/stdio.h"
#include "../stdlib/memutil/memutil.h"
#include "../multiboot.h"

#define NUM_PAGES_DIRS 256
#define NUM_PAGE_FRAMES (0x100000000 / 0x1000 / 8)

static uint32_t page_frame_min;
static uint32_t page_frame_max;
static uint32_t totl_alloc;

uint32_t pframes_left;

uint8_t physical_memory_bitmap[NUM_PAGE_FRAMES / 8];

static uint32_t page_dirs[NUM_PAGES_DIRS][1024] __attribute__((aligned(4096)));
static uint8_t page_dirs_used[NUM_PAGES_DIRS];

void pmm_init(uint32_t mem_low, uint32_t mem_high) {
    page_frame_min = ALIGN(mem_low, 0x1000) / 0x1000;
    page_frame_max = mem_high / 0x1000;
    totl_alloc = 0;
    pframes_left = page_frame_max - page_frame_min;

    memset(physical_memory_bitmap, 0, sizeof(physical_memory_bitmap));
}


void memory_init(struct multiboot_info *boot_info) {
    uint32_t physical_alloc_start = get_physical_alloc_start(boot_info);
    uint32_t physical_alloc_end = boot_info->mem_upper * 1024;

    initial_page_dir[0] = 0; // remove idetity mapping
    invalidate(0);
    initial_page_dir[1023] = ((uint32_t) initial_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE; // recursive mapping
    invalidate(0xFFFFF000);

    pmm_init(physical_alloc_start, physical_alloc_end);

    memset(page_dirs, 0, sizeof(page_dirs));
    memset(page_dirs_used, 0, sizeof(page_dirs_used));
}


void map_pframe(uint32_t virutalAddr, uint32_t physAddr, uint32_t flags){
    uint32_t *prevPageDir = 0;

    if (virutalAddr >= KERNEL_START){
        prevPageDir = get_current_page_dir();
        if (prevPageDir != initial_page_dir){
            change_page_dir(initial_page_dir);
        }
    }

    uint32_t pdIndex = virutalAddr >> 22;
    uint32_t ptIndex = virutalAddr >> 12 & 0x3FF;
    
    uint32_t* pageDir = REC_PAGEDIR;
    uint32_t* pt = REC_PAGETABLE(pdIndex);

    if (!(pageDir[pdIndex] & PAGE_FLAG_PRESENT)){
        uint32_t ptPAddr = alloc_pframe();
        pageDir[pdIndex] = ptPAddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
        invalidate(virutalAddr);

        for (uint32_t i = 0; i < 1024; i++){
            pt[i] = 0;
        }
    }

    pt[ptIndex] = physAddr | PAGE_FLAG_PRESENT | flags;
    invalidate(virutalAddr);

    if (prevPageDir != 0){
        sync_page_dirs();

        if (prevPageDir != initial_page_dir){
            change_page_dir(prevPageDir);
        }
    }

}


uint32_t alloc_pframe() {
    uint32_t start = page_frame_min / 8 + ((page_frame_min & 7) != 0 ? 1 : 0);
    uint32_t end = page_frame_max / 8 - ((page_frame_max & 7) != 0 ? 1 : 0);

    for (uint32_t b = start; b < end; b++) {
        uint8_t byte = physical_memory_bitmap[b];
        if (byte == 0xFF){
            continue;
        }

        for (uint32_t i = 0; i < 8; i++) {
            bool used = byte >> i & 1;

            if (!used) {
                pframes_left--;

                byte |= (1 << i);; // set the bit
                physical_memory_bitmap[b] = byte;
                totl_alloc++;

                uint32_t addr = (b*8 + i) * 0x1000;
                return addr;
            }
        }
    }
    printf("ERROR: cant allocate physical page frames\n");
    asm volatile("int $0x0E");  // 0x0E = Page Fault
    return 0;
}


uint32_t* get_current_page_dir() {
    uint32_t pd;
    asm volatile("mov %%cr3, %0": "=r"(pd));
    pd += KERNEL_START;

    return (uint32_t*) pd;
}


void change_page_dir(uint32_t* pd) {
    pd = (uint32_t*) (((uint32_t)pd)-KERNEL_START);
    asm volatile("mov %0, %%eax \n mov %%eax, %%cr3 \n" :: "m"(pd));
}


void sync_page_dirs(){
    for (int i = 0; i < NUM_PAGES_DIRS; i++){
        if (page_dirs_used[i]){
            uint32_t* pageDir = page_dirs[i];

            for (int i = 768; i < 1023; i++){
                pageDir[i] = initial_page_dir[i] & ~PAGE_FLAG_OWNER;
            }
        }
    }
}


uint32_t get_physical_alloc_start(struct multiboot_info* bootInfo) {
    struct multiboot_mmap_entry* entry = (struct multiboot_mmap_entry*) bootInfo->mmap_addr;
    uint32_t end_entry = bootInfo->mmap_addr + bootInfo->mmap_length;

    extern uint32_t _kernel_end; // virtual address in higher-half 
    uint32_t kernel_end_phys = (uint32_t)&_kernel_end - 0xC0000000;

    while ((uint32_t)entry < end_entry) {
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // Start allocation just after the kernel
            if (entry->addr_low <= kernel_end_phys && entry->addr_low + entry->len_low > kernel_end_phys) {
                return kernel_end_phys;
            }
        }
        entry = (struct multiboot_mmap_entry*) ((uint32_t)entry + entry->size + sizeof(entry->size));
    }
	asm volatile("int $0x0E");  // 0x0E = Page Fault
	return 0;
}


void print_mmaps(struct multiboot_info* boot_info) {
    for (uint32_t i = 0; i < boot_info->mmap_length; i += sizeof(struct multiboot_mmap_entry)) {
        struct multiboot_mmap_entry *mmme = (struct multiboot_mmap_entry *) (boot_info->mmap_addr + i);
        printf("addr: %x | length: %x | size: %x | Type: %x\n", mmme->addr_low, mmme->len_low, mmme->size, mmme->type);
    }
}


void invalidate(uint32_t vaddr){
    asm volatile("invlpg %0" :: "m"(vaddr));
}
