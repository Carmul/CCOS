#include <stdint.h>
#include "string.h"

uint16_t strlen(const char* str) {
	uint16_t len = 0;
	while (str[len])
		len++;
	return len;
}

uint16_t strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}