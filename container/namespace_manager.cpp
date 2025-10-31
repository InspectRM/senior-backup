#define _GNU_SOURCE
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

using namespace std;

// Base class for namespace operations
class NamespaceManager {
private:
    int makePrivateMounts() {
        if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0) {
            perror("MS_PRIVATE");
            return -1;
        }
        return 0;
    }
    
public:
    int enterNewNamespaces() {
        if (makePrivateMounts() != 0) return -1;
        int flags = CLONE_NEWUTS | CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNET;
        if (unshare(flags) != 0) { perror("unshare"); return -1; }
        return 0;
    }

    int chrootInto(const char* new_root) {
        if (chdir(new_root) != 0) { perror("chdir new_root"); return -1; }
        if (chroot(".") != 0)     { perror("chroot");         return -1; }
        if (chdir("/") != 0)      { perror("chdir /");        return -1; }
        return 0;
    }

    int mountMinimal() {
        mkdir("/proc", 0555);
        if (mount("proc", "/proc", "proc", 0, nullptr) != 0) { perror("mount /proc"); return -1; }
        mkdir("/dev", 0755);
        if (mount("tmpfs", "/dev", "tmpfs", MS_NOSUID|MS_STRICTATIME, "mode=755") != 0) { perror("mount /dev"); return -1; }
        mkdir("/tmp", 01777);
        if (mount("tmpfs", "/tmp", "tmpfs", MS_STRICTATIME, "mode=1777") != 0) { perror("mount /tmp"); return -1; }
        return 0;
    }
    //in linux : 
    // 0555 permissions (octal):
    // Special flags: 0  -> none
    // Owner:        5  -> 4 (read) + 1 (execute) = r-x
    // Group:        5  -> r-x
    // Others:       5  -> r-x

    // 01777 permissions (octal):
    // Special flags: 1  -> sticky bit (users can't delete others' files)
    // Owner:        7  -> rwx
    // Group:        7  -> rwx
    // Others:       7  -> rwx

    int setHostname(const char* name) {
        if (sethostname(name, (size_t)strlen(name)) != 0) { perror("sethostname"); return -1; }
        return 0;
    }
};