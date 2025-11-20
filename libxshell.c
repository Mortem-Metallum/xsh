#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include "xsh.h"

// Information for the prompt (% if a regular user, or # if root)
void prompt(){
    uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);
	if(strcmp(pw->pw_name, "root") != 0){
		printf("%% ");
   		fflush(stdout);
	} else {
		printf("# ");
   		fflush(stdout);
	}
}
// When SIGINT occurs (Ctrl+C), just continue executing
void handle_sigint(int sig){
	printf("\n");
    prompt();
}