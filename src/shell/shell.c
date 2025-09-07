#include "shell.h"
#include "../vga/vga.h"
#include "../heap/heap.h"
#include "../stdlib/stdio/stdio.h"
#include "../keyboard/keyboard.h"
#include "../stdlib/string/string.h"
#include "../timer/timer.h"
#include <stdint.h>

extern uint64_t ticks;

void enable_shell() {
    while(true) {
		puts("$ ");
		char *command = read_line();
		if (!(*command)) {
			kfree(command);
			continue;
		}
		exec(command);
		kfree(command);
	}
}


void exec(char *command) {
    // remove trailing spaces
    for (size_t i = strlen(command) - 1; i > 0; i--) {
        if (command[i] == ' ')
            command[i] = '\0';
        else
            break;
    }
    // revome leading spaces
    while (*command == ' ') { command++; }

	if(strcmp("ls", command) == 0) {
		puts("ls\n");
	} else if(strcmp("clear", command) == 0 || strcmp("cls", command) == 0) {
		clear_screan();
	} else if(strcmp("hs", command) == 0) { // heap status
		heap_print_stats();
	} else if(strcmp("hb", command) == 0) { // heap blocks
		heap_print_blocks();
	} else if(strcmp("hi", command) == 0) { // heap integriry
		heap_check_integrity();
	} else if (strcmp("time", command) == 0) {
        time();
    } else if (strcmp("rand", command) == 0) {
        rand();
    } else if(strcmp("panic", command) == 0) {
        asm volatile("int3"); // trigger breakpoint exception
    } else {
		printf("UNKNOWN COMMAND : %s\n", command);
	}
}


void time() {
    uint32_t t = (uint32_t)ticks; // 64bit division not natively supported
    uint32_t d = t / 86400000;
    uint32_t h = t / 3600000 % 24; 
    uint32_t m = t / 60000  % 60;
    uint32_t s = t / 1000 % 60;
    uint32_t ms = t % 1000;
    printf("Machine booted %d days %d hours %d minuits and %d.%d seconds ago\n",d, h, m, s, ms);
}

void rand() {
    uint32_t x = (uint32_t)((ticks >> 32) ^ ticks);
    // LCG step
    x = x * 1664525u + 1013904223u;
    printf("%u\n", x);
}
