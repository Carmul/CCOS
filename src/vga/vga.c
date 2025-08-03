#include<stdint.h>
#include "vga.h"
#include "../stdlib/memutil/memutil.h"

#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const) VGA_MEMORY;
uint8_t color = VGA_COLOR_BLACK << 4 | VGA_COLOR_WHITE; // default color white on black


void clear_screan() {
    line = 0;
    column = 0;
    for(int y = 0; y < VGA_HEIGHT; y++) {
        for(int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = ' ' | (color << 8);
        }
    }
    set_cursor_pos(0);
}


void scroll_up() {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[(y - 1) * VGA_WIDTH + x] = vga[y * VGA_WIDTH + x];
        }
    }

    for (int x = 0; x < VGA_WIDTH; x++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ' ' | (color << 8);
    }
}


void new_line() {
    column = 0;
    if(line == VGA_HEIGHT - 1) {
        scroll_up();
    } else {
        line++;
    }
}


void print(const char* str) {
    while(*str) {
        printc(*str);
        str++;
    }
}

void printc(const char c) {

    switch (c) {
        case '\n':
            new_line();
            break;
        case '\r':
            column = 0;
            break;
        case '\b':
            if (column > 0) {
                column--;
                vga[line * VGA_WIDTH + column] = ' ' | color << 8;
            } else if (line > 0) {
                line--;
                column = VGA_WIDTH - 1;
                if((vga[line * VGA_WIDTH + column] & 0xFF) != ' '){ // when last line is full
                    vga[line * VGA_WIDTH + column] = ' ' | color << 8;
                    break;
                } else { // returns to last non space char
                    while(((vga[line * VGA_WIDTH + column - 1] & 0xFF) == ' ') && column > 0)
                        column--;
                }
            }
            break;
        default:
            if(column == VGA_WIDTH){
                new_line();
            }
            vga[line * VGA_WIDTH + column] = c | color << 8;
            column++;
            break;
    }
    set_cursor_pos(line * VGA_WIDTH + column);
}


void printd(uint32_t dec) {
    char buf[11];              // 10 digits + NUL
    int i = 10;
    buf[i] = '\0';

    if (dec == 0) {
        buf[--i] = '0';
    } else {
        while (dec && i > 0) {
            buf[--i] = (char)('0' + (dec % 10));
            dec /= 10;
        }
    }
    print(&buf[i]);
}


void set_cursor_pos(uint16_t pos) {
    outb(VGA_CRTC_INDEX, 0x0F);         // low byte of cursor
    outb(VGA_CRTC_DATA, pos & 0xFF);
    outb(VGA_CRTC_INDEX, 0x0E);         // high byte of cursor
    outb(VGA_CRTC_DATA, (pos >> 8) & 0xFF);
}

void rotate_font_color(){
    color = (color & 0xF0) | (((color & 0x0F) + 1) % 16);
    if((color >> 4) == (color & 0x0F))
        rotate_font_color();
}

void set_font_color(uint8_t fc) {
    color = (color & 0xF0) | (fc & 0x0F);
}

