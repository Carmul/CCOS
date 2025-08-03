#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000

/* Hardware text mode color constants. */
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA  13
#define VGA_COLOR_LIGHT_BROWN 14
#define VGA_COLOR_WHITE 15

void printc(const char c);
void print(const char* s);
void printd(uint32_t dec);
void scroll_up();
void new_line();
void clear_screan();
void set_cursor_pos(uint16_t pos);
void rotate_font_color();
void set_font_color(uint8_t fc);

#endif