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

void strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

void itoa(int num, char* str) {
    int i = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    char temp[16];
    int j = 0;
    while (num > 0) {
        temp[j++] = '0' + (num % 10);
        num /= 10;
    }
    // Reverse the string
    for (int k = j - 1; k >= 0; k--) {
        str[i++] = temp[k];
    }
    str[i] = '\0';
}