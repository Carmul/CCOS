#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "vga/vga.h"
#include "timer/timer.h"
#include "memory/memory.h"

#include "stdlib/memutil/memutil.h"
#include "stdlib/string/string.h"
#include "stdlib/stdio/stdio.h"

#include "multiboot.h"

void kmain(uint32_t magic, struct multiboot_info* boot_info) 
{
	clear_screan();
	gdt_init();
	idt_init();
	timer_init();

	
	set_font_color(VGA_COLOR_LIGHT_RED);
	printf("GRUB magic value: 0x%X\n", magic);
	print_mmaps(boot_info);
	set_font_color(VGA_COLOR_WHITE);
	
	for(;;);
}
