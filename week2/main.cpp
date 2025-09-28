#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main() {
    // Unshare into a new UTS namespace
    if (unshare(CLONE_NEWUTS) == -1) {
        perror("unshare");
        return 1;
    }

    // Change hostname inside namespace
    if (sethostname("container1", 10) == -1) {
        perror("sethostname");
        return 1;
    }

    // Now run a shell inside the namespace
    execlp("/bin/bash", "/bin/bash", NULL);

    return 0;
}
