#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "keyboard.h"
#include "../vga/vga.h"
#include "../stdlib/memutil/memutil.h"

static const char base_map[128] = {
/*00*/ 0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b', // 27 = ESC
/*0F*/ '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
/*1E*/ 'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\','z','x',
/*2E*/ 'c','v','b','n','m',',','.','/', 0, '*', 0,' ',
/*3C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*4C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*5C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*6C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static const char shift_map[128] = {
/*00*/ 0, 27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
/*0F*/ '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
/*1E*/ 'A','S','D','F','G','H','J','K','L',':','"','~', 0,'|','Z','X',
/*2E*/ 'C','V','B','N','M','<','>','?', 0, '*', 0,' ', 
/*3C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*4C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*5C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*6C*/ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

bool holds_shift = false;
bool capsLock_on = false;

void on_irq1() {
    uint8_t code = inb(0x60);
    uint8_t scancode = code & 0x7F;
    bool press = (code & 0x80) == 0;

    char c = 0;
    switch(scancode) {
        case 42: // shift
            if(press) {
                holds_shift = true;
            } else {
                holds_shift = false;
            }
            break;
        case 58: // capsLock_on
            if (!capsLock_on && !press){
                capsLock_on = true;
            }else if (capsLock_on && !press){
                capsLock_on = false;
            }
            break;
        default:
            if(press){            
                if(capsLock_on || holds_shift){
                    c = shift_map[scancode];
                } else {
                    c = base_map[scancode];
                }
            }   
    }
    if(c == 0) // ignore unknown keys
        return;

    printc(c);
}

