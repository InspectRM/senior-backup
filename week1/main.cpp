#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char *args[] = {"/bin/bash", NULL};
        execvp(args[0], args); // Replace child with a shell
        perror("execvp failed");
        exit(1);
    } else {
        // Parent process
        wait(NULL);
        printf("Child process finished\n");
    }
    return 0;
}
