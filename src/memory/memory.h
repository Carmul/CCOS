#ifndef MEMORY_H
#define MEMORY_H

#include "../multiboot.h"

void init_memory();
void print_mmaps(struct multiboot_info* boot_info);

#endif