#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include "xsh.h"

char cwd[PATH_MAX];


// Information for the prompt (% if a regular user, or # if root)
void prompt(){
	char finalwd[PATH_MAX] = "";
	uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);
	getcwd(cwd, sizeof(cwd));
	if(strcmp(cwd, pw->pw_dir) == 0) {
		char finalwd_home[PATH_MAX] = "~";
		strcpy(finalwd, finalwd_home);
	} else {
		strcpy(finalwd, cwd);
		char *finalwd_tail = strrchr(cwd, '/'); finalwd_tail++;
		if(strcmp(finalwd_tail, "\0") == 0){
			;
		} else {
			strcpy(finalwd, finalwd_tail);
		}
	}
	if(strcmp(pw->pw_name, "root") != 0){
		printf("%s %% ", finalwd);
   		fflush(stdout);
	} else {
		printf("%s # ", finalwd);
   		fflush(stdout);
	}
}
// When SIGINT occurs (Ctrl+C), just continue executing
void handle_sigint(int sig){
	printf("\n");
    prompt();
}