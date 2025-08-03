#include "gdt.h"
#include "../stdlib/memutil/memutil.h"

extern void gdt_load(struct gdt_ptr*);

static struct gdt_entry gdt[6];
static struct gdt_ptr gdt_descriptor;
struct tss_entry tss;


static void set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[i].limit_low = (limit & 0xFFFF);
    gdt[i].base_low = (base & 0xFFFF);
    gdt[i].base_middle = (base >> 16) & 0xFF;
    gdt[i].access = access;
    gdt[i].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[i].base_high = (base >> 24) & 0xFF;
}


void write_tss(uint32_t num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    memset(&tss, 0, sizeof(tss)); 

    tss.ss0 = ss0;
    tss.esp0 = esp0;

    tss.cs = 0x08 | 0x3;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10 | 0x3;

    set_entry(num, base, limit, 0xE9, 0x00); 
}


void gdt_init() {
    gdt_descriptor.limit = sizeof(gdt) - 1;
    gdt_descriptor.base = (uint32_t)&gdt;

    set_entry(0, 0, 0, 0, 0);                // Null segment
    set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel Code segment
    set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel Data segment
    set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
    set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment
    write_tss(5, 0x10, 0x0);


    gdt_load(&gdt_descriptor);
    asm volatile ("ltr %%ax" : : "a"(5 << 3));
}
