#ifndef XSH_H
#define XSH_H
#define MAX_LINE 512

#include <stdio.h>

struct cmds {
    char lastcmd[4096];
} cmds;



void handle_sigint(int sig);
void prompt();
int cmdlogic(char *cmd);
int interpret(char *file);

#endif
