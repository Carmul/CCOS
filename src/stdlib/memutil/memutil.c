# include <stdint.h>
#include "memutil.h"

void memset(void *dest, int val, uint32_t count) {
    char *tmp = (char *) dest;
    while(count){
        *tmp = val;
        tmp++;
        count--;
    }
}

