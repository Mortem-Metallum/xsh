#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <limits.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "xsh.h"

char cwd[PATH_MAX];

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
int interpret(char *file){
    FILE *fp = fopen(file, "r");
	char cmd[512];
	if(fp != NULL){
		while(fgets(cmd, sizeof(cmd), fp)){
			cmd[strcspn(cmd, "\n")] = '\0';
    	    if(strcmp(cmd, "") == 0 || strchr(cmd, '#') != NULL){
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

		    } else if(strncmp(cmd, "cd", 3) == 0){
			    uid_t uid = getuid();
			    struct passwd *pw = getpwuid(uid);
			    char *dir = cmd + 3;
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
	    fclose(fp);
        return 0;
	} else {
		return 1;
	}
}