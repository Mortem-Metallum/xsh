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

char version[26] = "xsh version v1.4.5-stable";
char cwd[PATH_MAX];

char disclaimer[1596] = "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";
char histfile[512];
char xshrc[PATH_MAX];


char cmd[705];

int main(int argc, char *argv[]){
    signal(SIGINT, handle_sigint);
	int i = 1;
	while(i < argc){
		if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0){
			printf("%s (%s)\n", version, arch);
			return 0;
		} else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
			printf("%s (%s)\n", version, arch);
			printf("Usage: xsh [option]\n");
			printf("Args:\n\n");
			printf("<no arg>: execute xsh normally\n");
			printf("--version or -v: print version and exit\n");
			printf("--license or -L: print license information and exit\n");
			printf("--help or -h: display this help and exit\n");
			printf("--command or -c: execute a single command\n");
			return 0;
		} else if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--command") == 0){
			if(i + 1 < argc){
				char *x = argv[i+1];
				cmdlogic(x, argv[0]);
				return 0;
			} else {
				printf("%s: %s requires an argument\n", argv[0], argv[i]);
				return 1;
			}
		} else {
			int tryinter = interpret(argv[i], argv[0]);
			if(tryinter != 0){
				printf("%s: %s was unexpected at this time\n", argv[0], argv[i]);
				return 1;
			} 
			return 0;
		}
	i++;
	}
	uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);
	if(argv[0] == NULL){
		return 1;
	}
	
	if(argv[0][0] == '-'){
		snprintf(xshrc, sizeof(xshrc), "%s/.xsh_profile", pw->pw_dir);
	} else {
		snprintf(xshrc, sizeof(xshrc), "%s/.xshrc", pw->pw_dir);
	}
	FILE *openrc = fopen(xshrc, "r");
	if(openrc != NULL){
		interpret(xshrc, progname);
	} else {
		FILE *createrc = fopen(xshrc, "w");
		fprintf(createrc, "#!/usr/bin/env xsh\n# Autorun script for the X shell\n");
		fclose(createrc);
	} while(1){
		prompt();


		
		char finalcmd[PATH_MAX];
		snprintf(finalcmd, sizeof(finalcmd), "%s\n", cmd);
		strncpy(cmds.lastcmd, finalcmd, sizeof(cmds.lastcmd));
		snprintf(progname, sizeof(progname), "%s", argv[0]);
	
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

		
		int ret = cmdlogic(cmd, progname);
		if(ret == 0){
			;
		} else {
			printf("%d|", ret);
		}

		

	}
	// When EOF occurs (Ctrl+D), print "exit" before returning code 0
	printf("\nexit\n");
	return 0;
}
