#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "vga/vga.h"
#include "timer/timer.h"
#include "memory/memory.h"
#include "heap/heap.h"

#include "stdlib/memutil/memutil.h"
#include "stdlib/string/string.h"
#include "stdlib/stdio/stdio.h"

#include "multiboot.h"

void kmain(uint32_t magic __attribute__((unused)), struct multiboot_info* boot_info) {
	
	clear_screan();
	gdt_init();
	idt_init();
	timer_init();
	memory_init(boot_info);
	heap_init();


	for(;;);
}
