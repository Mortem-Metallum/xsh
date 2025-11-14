#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>

#if defined(__x86_64__) || defined(_M_X64)
	char arch[15] = "i86pc, amd64";
#elif defined(__i386) || defined(_M_IX86)
	char arch[15] = "i86pc, ia32";
#elif defined(__aarch64__)
	char arch[15] = "arm, aarch64";
#elif defined(__arm__)
	char arch[15] = "arm, arm";
#else
	char arch[15] = "other";
#endif

char version[600] = "xsh version v1.0.0-alpha_release";
char cwd[PATH_MAX];
char cmd[705];
char disclaimer[1596] = "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
void handle_sigint(int sig){
    printf("\n%% ");
    fflush(stdout);
}

int main(int argc, char *argv[]){
    signal(SIGINT, handle_sigint);
	int i = 1;
	while(i < argc){
		if(strcmp(argv[i], "--version") == 0){
			printf("%s (%s)\n", version, arch);
			printf("Copyright (C) 2025, John Doe. License BSD 3-clause or later\n\nThis is free software. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n\n1. Redistributions of source code must retain the above copyright notice, this lists of conditions, and the following disclaimer.\n\n2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n\n3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.\n\n");
			printf("%s\n", disclaimer);
			return 0;
		} else if(strcmp(argv[i], "--help") == 0){
			printf("%s (%s)\n", version, arch);
			printf("Usage: xsh [ OPERAND ]\n");
			printf("Args:\n\n");
			printf("<no arg>: execute xsh normally\n");
			printf("--version: print version and license and exit\n");
			printf("--help: display this help and exit\n");
			return 0;
		} else {
			;
		}
	}
	i++;

    while(1){
		uid_t uid = getuid();
		struct passwd *pw = getpwuid(uid);
		if(strcmp(pw->pw_name, "root") != 0){
			printf("%% ");
   			fflush(stdout);
		} else {
			printf("# ");
   			fflush(stdout);
		}
   		

    	if(fgets(cmd, sizeof(cmd), stdin) == NULL) {
        	break;
    	}

    	cmd[strcspn(cmd, "\n")] = '\0';

    	if(strcmp(cmd, "") == 0){
        	continue;
    	}

    	if(strcmp(cmd, "pwd") == 0){
        	if(getcwd(cwd, sizeof(cwd)) != NULL) {
            		printf("%s\n", cwd);
        	}
    	} else if(strcmp(cmd, "clear") == 0){
        	printf("\033c");
    	} else if(strcmp(cmd, "exit") == 0){
			printf("exit\n");
        	return 0;
    	} else if(strcmp(cmd, "whoami") == 0){
			if(pw){
				printf("%s\n", pw->pw_name);
			} else {
				printf("who are you?\n");
			}
		} else if(strncmp(cmd, "echo", 4) == 0){
			char *msg = cmd + 4;
			while(*msg == ' ') msg++;
			printf("%s\n", msg);
		} else if(strcmp(cmd, "cd") == 0){
			struct passwd *pw = getpwuid(getuid());
			chdir(pw->pw_dir);
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
        			printf("xsh: %s: command not found\n", args[0]);
    			}
			} else {
    			perror("xsh: unable to execute command");
			}
    	}
    }
}
