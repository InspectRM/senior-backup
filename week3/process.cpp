#define _GNU_SOURCE
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>

int main() {
    while (1) {
        printf("shell> ");
        
        char cmd[100];
        scanf("%s", cmd);
        
        if (strcmp(cmd, "exit") == 0) {
            break;
        }
        
        pid_t pid = fork();
        
        if (pid == 0) {
            unshare(CLONE_NEWPID);

            pid_t inner_pid = fork();

            if (inner_pid == 0){
                printf("([%d] <-- Child PID inside container) RUNNING: %s\n", getpid(), cmd);
                if (strcmp(cmd, "bash") == 0) {
                        char *args[] = {cmd, "-i", NULL};  // -i = interactive
                        execvp(cmd, args);
                    } else {
                        char *args[] = {cmd, NULL};
                        execvp(cmd, args);
                    }
                printf("Error, couldn't execute!\n");
                return 1;
            } else{
                wait(NULL);
                return 0;
            }
        } else{
            printf("[%d] <-- parent\n", getpid());
            wait(NULL);
            printf("Done!\n\n");  
        } 
    }
    
    return 0;
}