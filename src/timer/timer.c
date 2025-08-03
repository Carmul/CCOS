#include <stdint.h>
#include "timer.h"
#include "../stdlib/memutil/memutil.h"
#include "../vga/vga.h"

uint64_t ticks;
const uint32_t Hz = 1000; // Hz must be at least 19

void timer_init() {
    ticks = 0;
    uint32_t divisor = 1193180 / Hz;

    outb(0x43, 0x34);             // Send command word
    outb(0x40, divisor & 0xFF);   // Send low byte
    outb(0x40, divisor >> 8);     // Send high byte
}


void on_irq0() {
    ticks++;
}


void sleep_ms(uint32_t ms) {
    uint64_t target = ticks + ms;
    while (ticks < target) {
        __asm__ __volatile__("hlt");
    }
}
