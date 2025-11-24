#ifndef XSH_H
#define XSH_H
#include <stdio.h>

struct cmds {
    char lastcmd[4096];
} cmds;

void handle_sigint(int sig);
void prompt();
int interpret(char *file);

#endif