#include "shell.h"
#include "../vga/vga.h"
#include "../heap/heap.h"
#include "../stdlib/stdio/stdio.h"
#include "../stdlib/memutil/memutil.h"
#include "../keyboard/keyboard.h"
#include "../stdlib/string/string.h"
#include "../timer/timer.h"
#include "../fs/ramfs/ramfs.h"
#include <stdint.h>

extern uint64_t ticks;

char** args;
int argc;

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

    args = split_by_spaces_inplace(command);

	if(strcmp("ls", args[0]) == 0) {
        ls();

	} else if(strcmp("clear", args[0]) == 0 || strcmp("cls", args[0]) == 0) {
		clear_screan();

	} else if(strcmp("hs", args[0]) == 0) { // heap status
		heap_print_stats();

	} else if(strcmp("hb", args[0]) == 0) { // heap blocks
		heap_print_blocks();

	} else if(strcmp("hi", args[0]) == 0) { // heap integriry
		heap_check_integrity();

	} else if (strcmp("time", args[0]) == 0) {
        time();

    } else if (strcmp("rand", args[0]) == 0) {
        rand();

    } else if(strcmp("panic", args[0]) == 0) {
        panic();

    } else if(strcmp("touch", args[0]) == 0) {
        touch();

    } else if(strcmp("rm", args[0]) == 0) {
        rm();

    } else if(strcmp("cat", args[0]) == 0) {
        cat();

    } else if(strcmp("echo", args[0]) == 0) {
        echo();

    } else {
		printf("UNKNOWN COMMAND : %s\n", args[0]);

	}

    kfree(args);
}

// This modifies the input string
// Returns array of pointers into 'str' (no extra mallocs for words)
char** split_by_spaces_inplace(char* str) {
    if (!str) return NULL;

    size_t count = 0;
    // First pass: count words
    for (char* p = str; *p; p++) {
        if (*p != ' ' && (p == str || *(p-1) == ' '))
            count++;
    }
    argc = count;
    char** result = kmalloc((count + 1) * sizeof(char*));
    if (!result) return NULL;

    size_t idx = 0;
    char* p = str;
    while (*p) {
        while (*p == ' ') { *p = '\0'; p++; } // skip spaces, terminate them
        if (*p) {
            result[idx++] = p;
            while (*p && *p != ' ') p++;
        }
    }
    result[idx] = NULL;
    return result;
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

void panic() {
    asm volatile("int3"); // trigger breakpoint exception
}

void touch() {
    if (argc != 2){
        printf("touch accepts 2 arguments not %d\n", argc);
        return;
    }
    ramfs_close(ramfs_create(args[1]));
    return;
}

void rm() {
    if (argc != 2){
        printf("rm accepts 2 arguments not %d\n", argc);
        return;
    }

    int status = ramfs_delete(args[1]);
    if (status == -1) {
        printf("Error: file \"%s\" not found\n", args[1]);
    }
}

void cat() {
    if (argc != 2){
        printf("cat accepts 2 arguments not %d\n", argc);
        return;
    }

    int fd = ramfs_open(args[1]);
    if (fd == -1) {
        printf("Error: file \"%s\" not found\n", args[1]);
        return;
    }
    char *buffer = kmalloc(RAMDISK_BLOCK_SIZE);
    int read = ramfs_read(fd, buffer, RAMDISK_BLOCK_SIZE);
    ramfs_close(fd);

    if (read == 0) {
        kfree(buffer);
        return;
    }

    char *temp = buffer;
    for (int i = 0; i < read; i++) {
        putc(*temp++);
    }
    putc('\n');
    
    kfree(buffer);
}

void echo() {
    if (argc == 2) {
        puts(args[1]);
        putc('\n');
        return;
    }
    else if (argc == 3) {
        int fd = ramfs_open(args[2]);
        if (fd == -1)
            fd = ramfs_create(args[2]);

        ramfs_write(fd, args[1], strlen(args[1]));
        ramfs_close(fd);
    }
    else {
        printf("echo accepts 2/3 arguments <ascii string> [file to echo into]\n");
    }

}