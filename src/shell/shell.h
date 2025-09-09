#ifndef SHELL_H
#define SHELL_H


void exec(char *command);
void enable_shell();
void time();
void rand();
void touch();
void panic();
void rm();
void cat();
void echo();

char** split_by_spaces_inplace(char* str);

#endif