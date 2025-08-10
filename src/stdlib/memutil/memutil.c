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

void memcpy(void *destptr, void *srcptr, uint32_t size) {
	char* dst = (char*) destptr;
	const char* src = (char*) srcptr;
	for (uint32_t i = 0; i < size; i++)
		dst[i] = src[i];
}

