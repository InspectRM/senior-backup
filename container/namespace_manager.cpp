#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std; 


class NamespaceManager {
public:
    NamespaceManager() = default;

    bool setupNamespaces(bool pid = true, bool uts = false, bool mount = false, bool net = false) {
        int flags = 0;
        if (pid)   flags |= CLONE_NEWPID;
        if (uts)   flags |= CLONE_NEWUTS;
        if (mount) flags |= CLONE_NEWNS;
        if (net)   flags |= CLONE_NEWNET;

        if (unshare(flags) == -1) {
            perror("unshare");
            return false;
        }

        cout << "[NamespaceManager] Initialized:";
        if (pid) cout << " PID";
        if (uts) cout << " UTS";
        if (mount) cout << " MOUNT";
        if (net) cout << " NET";
        cout << endl;

        return true;
    }

    bool setHostname(const string &name) {
        if (sethostname(name.c_str(), name.size()) == -1) {
            perror("sethostname");
            return false;
        }
        cout << "[NamespaceManager] Hostname set to: " << name << endl;
        return true;
    }
};
