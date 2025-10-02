#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
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
            // Child: run the command
            printf("[Child %d] Running: %s\n", getpid(), cmd);
            
            char *args[] = {cmd, NULL};
            execvp(cmd, args);
            
            // Only runs if exec fails
            printf("Command failed!\n");
            return 1;
        } else {
            // Parent: wait for child
            printf("[Parent %d] Waiting...\n", getpid());
            wait(NULL);
            printf("[Parent] Done!\n\n");
        }
    }
    
    return 0;
}