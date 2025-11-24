// The X shell
// Copyright (C) John Doe, 2025.
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <curses.h>
#include <stdlib.h>
#include <pwd.h>
#include "xsh.h"

// Define the CPU arch
// x86_64/AMD64
#if defined(__x86_64__) || defined(_M_X64)
	char arch[15] = "i86pc, amd64";
#elif defined(__i386) || defined(_M_IX86)
// i386-i686 or IA-32
	char arch[15] = "i86pc, ia32";
// ARM64
#elif defined(__aarch64__)
	char arch[15] = "arm, aarch64";
// 32-bit ARM
#elif defined(__arm__)
	char arch[15] = "arm, arm";
// Any other architecture (eg. Itanium, PowerPC, RISC-V, SPARC, SPARC64)
#else
	char arch[15] = "other architecture";
#endif

char version[26] = "xsh version v1.2.0-stable";
char cwd[PATH_MAX];

char disclaimer[1596] = "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
char histfile[512];
char xshrc[PATH_MAX];


char cmd[705];

int main(int argc, char *argv[]){
    signal(SIGINT, handle_sigint);
	int i = 1;
	while(i < argc){
		if(strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0){
			printf("%s (%s)\n", version, arch);
			return 0;
		} else if(strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0){
			printf("%s (%s)\n", version, arch);
			printf("Usage: xsh [option]\n");
			printf("Args:\n\n");
			printf("<no arg>: execute xsh normally\n");
			printf("--version or -v: print version and exit\n");
			printf("--license or -L: print license information and exit\n");
			printf("--help or -h: display this help and exit\n");
			return 0;
		} else if(strcmp(argv[i], "--license") == 0 || strcmp(argv[i], "-L") == 0){
			printf("Copyright (C) 2025, John Doe. License BSD 3-clause or later\n\nThis is free software. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n\n1. Redistributions of source code must retain the above copyright notice, this lists of conditions, and the following disclaimer.\n\n2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n\n3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\n\n");
			printf("%s\n", disclaimer);
			return 0;
		} else {
			int tryinter = interpret(argv[i]);
			if(tryinter != 0){
				printf("xsh: %s was unexpected at this time\n", argv[i]);
				return 1;
			} 
			return 0;
		}
	i++;
	}
	uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);
	snprintf(xshrc, sizeof(xshrc), "%s/.xshrc", pw->pw_dir);
	FILE *openrc = fopen(xshrc, "r");
	if(openrc != NULL){
		interpret(xshrc);
	} else {
		;
	}

    while(1){

		prompt();


		
		char finalcmd[PATH_MAX];
		snprintf(finalcmd, sizeof(finalcmd), "%s\n", cmd);
		strncpy(cmds.lastcmd, finalcmd, sizeof(cmds.lastcmd));
		
		if(!pw->pw_dir){
			printf("who are you?\n");
			printf("1|");
		} else {
			snprintf(histfile, sizeof(histfile), "%s/.xshhistory", pw->pw_dir);
			FILE *openhist = fopen(histfile, "a");
			if(strcmp(cmds.lastcmd, "history") == 0){ 
				continue;
			} else {
				fprintf(openhist, cmds.lastcmd);
			}
			fclose(openhist);
		}

    	if(fgets(cmd, sizeof(cmd), stdin) == NULL) {
        	break;
    	}

    	cmd[strcspn(cmd, "\n")] = '\0';



    	if(strcmp(cmd, "") == 0){
        	continue;
    	} else if(strcmp(cmd, "pwd") == 0){
        	if(getcwd(cwd, sizeof(cwd)) != NULL) {
            		printf("%s\n", cwd);
        	}
    	} else if(strcmp(cmd, "clear") == 0){
        	printf("\033c");
    	} else if(strcmp(cmd, "exit") == 0){
			printf("exit\n");
        	return 0;
    	} else if(strcmp(cmd, "whoami") == 0){
			uid_t uid = getuid();
			struct passwd *pw = getpwuid(uid);
			if(pw){
				printf("%s\n", pw->pw_name);
			} else {
				printf("who are you?\n");
				printf("1|");
			}
		} else if(strncmp(cmd, "cat", 4) == 0){
			char *file = cmd + 4;
			char err[PATH_MAX];
			snprintf(err, sizeof(err), "cat: cannot stat %s", file);
			while(*file == ' ') file++;
			FILE* fp = fopen(file, "r");
			if(fp == NULL){
				perror(err);
				printf("1|");
				continue;
			}
			char ch;
			while((ch = fgetc(fp)) != EOF){
				putchar(ch);
			}
			fclose(fp);


		} else if(strncmp(cmd, "echo", 5) == 0){
			char *msg = cmd + 5;
			char *msg_head = strchr(msg, '"');
			if(msg_head == NULL) {
				printf("%s\n", msg);
			} else {
    			char *msg_tail = strrchr(msg_head, '"');
    			if(msg_tail != NULL && msg_tail != msg_head) {
        			msg_head++;
        			*msg_tail = '\0';
			        printf("%s\n", msg_head);
    			} else {
        			printf("echo: I say NO!\n1|");
    			}
			}

		} else if(strncmp(cmd, "cd", 2) == 0){
			uid_t uid = getuid();
			struct passwd *pw = getpwuid(uid);
			char *dir = cmd + 2;
			while(*dir == ' ') dir++;
			if(strcmp(dir, "~") == 0 || strcmp(dir, "") == 0){
				chdir(pw->pw_dir);
			} else {
				chdir(dir);
			}
		} else if(strncmp(cmd, "mkdir", 5) == 0){
			char *mk = cmd + 5;
			int trymk = mkdir(mk, 666);
			if(trymk != 0){
				char err[PATH_MAX];
				snprintf(err, sizeof(err), "mkdir: cannot create%s", mk);
				perror(err);
				printf("1|");
			}
		} else if(strncmp(cmd, "touch", 6) == 0) {
			char *mk = cmd + 6;
			printf("%s\n", mk);
			int trymk = open(mk, O_CREAT);
			if(trymk == 1){
				char err[PATH_MAX];
				snprintf(err, sizeof(err), "touch: cannot create%s", mk);
				perror(err);
				printf("1|");
			}
		} else if(strcmp(cmd, "history") == 0){
			FILE *fp = fopen(histfile, "r");
			char ch;
			while((ch = fgetc(fp)) != EOF){
				putchar(ch);
			}
			fclose(fp);
		} else if(strncmp(cmd, "rm", 3) == 0){
			char *del = cmd + 3;
			int trydel = remove(del);
			if(trydel != 0){
				char err[PATH_MAX];
				snprintf(err, sizeof(err), "rm: cannot remove %s", del);
				perror(err);
				printf("1|");
			}
		} else {
        	char *args[64];
        	int argc_local = 0;
        	char *token = strtok(cmd, " ");
        	while(token && argc_local < 63){
            	args[argc_local++] = token;
            	token = strtok(NULL, " ");
        	}
        	args[argc_local] = NULL;

        	pid_t pid = fork();
			if(pid == 0){
    			execvp(args[0], args);
    			_exit(127);
			} else if(pid > 0){
    			int status;
    			waitpid(pid, &status, 0);
    			if(WIFEXITED(status) && WEXITSTATUS(status) == 127){
        			printf("-xsh: %s: command not found\n127|", args[0]);
    			}
			} else {
    			perror("-xsh: could not execute");
			}
    	}

    }
	// When EOF occurs (Ctrl+D), print "exit" before returning code 0
	printf("\nexit\n");
	return 0;
}
