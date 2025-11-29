#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xsh.h"

char cwd[PATH_MAX];
char histfile[705];

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
        if(strcmp(finalwd_tail, "") == 0){
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
void handle_sigint(int sig){
    printf("\n");
    prompt();
}
int cmdlogic(char *cmd){
	uid_t uid = getuid();
	struct passwd *pw = getpwuid(uid);
	
	// Create builtin vars
	char HOME[PATH_MAX];
	char USER[PATH_MAX];
	char SHELL[PATH_MAX];

	strcpy(HOME, pw->pw_dir);
	strcpy(USER, pw->pw_name);
	strcpy(SHELL, pw->pw_shell);
	


	
	if(strcmp(cmd, "") == 0 || cmd[0] == '#'){
        	;
    	} else if(strcmp(cmd, "pwd") == 0){
        	if(getcwd(cwd, sizeof(cwd)) != NULL) {
            		printf("%s\n", cwd);
        	}
		return 0;
    	} else if(strcmp(cmd, "clear") == 0){
        	printf("\033c");
		return 0;
    	} else if(strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0){
		printf("exit\n");
        	exit(0);
    	} else if(strcmp(cmd, "whoami") == 0){
		uid_t uid = getuid();
		struct passwd *pw = getpwuid(uid);
		if(pw){
			printf("%s\n", pw->pw_name);
			return 0;
		} else {
			printf("who are you?\n");
			return 1;
		}
	} else if(strncmp(cmd, "cat", 3) == 0){
		char *file = cmd + 3;
		while(*file == ' ') file++;
		char err[PATH_MAX];
		snprintf(err, sizeof(err), "cat: cannot stat %s", file);
		while(*file == ' ') file++;
		FILE* fp = fopen(file, "r");
		if(fp == NULL){
			perror(err);
			return errno;
		}
		char ch;
		while((ch = fgetc(fp)) != EOF){
			putchar(ch);
		}
		fclose(fp);
		return 0;
	} else if(strncmp(cmd, "echo", 4) == 0){
		char *msg = cmd + 4;
		while(*msg == ' ') msg++;
		char *msg_head = strchr(msg, '"');
		if(msg_head == NULL){
			printf("%s\n", msg);
		} else {
    			char *msg_tail = strrchr(msg, '"');
    			if(msg_tail != NULL && msg_tail > msg_head) {
				msg_head++;
        			*msg_tail = '\0';
				printf("%s\n", msg_head);
    			} else {
        			printf("\n");
    			}
		}
		return 0;
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
		return 0;
	} else if(strncmp(cmd, "mkdir", 5) == 0){
		char *mk = cmd + 5;
		while(*mk == ' ') mk++;
		int trymk = mkdir(mk, 666);
		if(trymk != 0){
			char err[PATH_MAX];
			snprintf(err, sizeof(err), "mkdir: cannot create %s", mk);
			perror(err);
			return errno;
		}
	} else if(strncmp(cmd, "touch", 5) == 0) {
		char *mk = cmd + 6;
		while(*mk == ' ') mk++;
		int trymk = open(mk, O_CREAT);
		if(trymk == -1){
			char err[PATH_MAX];
			snprintf(err, sizeof(err), "touch: cannot create %s", mk);
			perror(err);
			return errno;
		}
	} else if(strcmp(cmd, "history") == 0){
		uid_t uid = getuid();
		struct passwd *pw = getpwuid(uid);
		snprintf(histfile, sizeof(histfile), "%s/.xshhistory", pw->pw_dir);
		FILE *fp = fopen(histfile, "r");
		char ch;
		while((ch = fgetc(fp)) != EOF){
			putchar(ch);
		}
		fclose(fp);
		return 0;
	} else if(strncmp(cmd, "rm", 2) == 0){
		char *del = cmd + 2;
		while(*del == ' ') del++;
		int trydel = remove(del);
		if(trydel != 0){
			char err[PATH_MAX];
			snprintf(err, sizeof(err), "rm: cannot remove %s", del);
			perror(err);
			return errno;
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
				if(WIFSIGNALED(status)){
					int signal = WTERMSIG(status);
					return 128 + signal;
				}
			
    			if(WIFEXITED(status) && WEXITSTATUS(status) == 127){
        			printf("xsh: %s: command not found\n", args[0]);
					return 127;
				}
		} else {
			return 1;
		}
    }
}
int interpret(char *file){
	FILE *fp = fopen(file, "r");
	if(fp != NULL){
		char line[MAX_LINE];
		while(fgets(line, sizeof(line), fp)){
			cmdlogic(line);
		}
	} else {
		return 1;
	}
	fclose(fp);
	return 0;
}

