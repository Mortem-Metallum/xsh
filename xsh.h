#ifndef XSH_H
#define XSH_H

struct cmds {
    char lastcmd[4096];
} cmds;

void handle_sigint(int sig);
void prompt();
int xsh_exec_script(char *file);

#endif