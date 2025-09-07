#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gdt/gdt.h"
#include "idt/idt.h"
#include "vga/vga.h"
#include "timer/timer.h"
#include "memory/memory.h"
#include "heap/heap.h"
#include "keyboard/keyboard.h"

#include "stdlib/memutil/memutil.h"
#include "stdlib/string/string.h"
#include "stdlib/stdio/stdio.h"

#include "multiboot.h"

void exec(const char *command);

void kmain(uint32_t magic __attribute__((unused)), struct multiboot_info* boot_info) {
	
	clear_screan();
	gdt_init();
	idt_init();
	timer_init();
	memory_init(boot_info);
	heap_init();


	while(true) {
		puts("\n$ ");
		char *command = read_line();
		if (!(*command)) {
			kfree(command);
			continue;
		}
		exec(command);
		kfree(command);
	}



	for(;;);
}

void exec(const char *command) {

	if(strcmp("ls", command) == 0) {
		puts("ls");
	} else if(strcmp("hs", command) == 0) {
		heap_print_stats();
	} else if(strcmp("hb", command) == 0) {
		heap_print_blocks();
	} else if(strcmp("hi", command) == 0) {
		heap_check_integrity();
	} else {
		printf("UNKNOWN COMMAND : %s", command);
	}
}
