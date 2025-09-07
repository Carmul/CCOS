#ifndef KEYBOARD_H
#define KEYBOARD_H

void on_irq1();

char* read_line();

char keyboard_buffer_get();
void keyboard_buffer_put(char c);

#endif

