#include "namespace_manager.cpp"
#include "cgroup_manager.cpp"

#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

class Container {
public:
    Container() = default;

    void runCommand(const string &cmd) {
        if (cmd == "exit") return;

        pid_t pid = fork();
        if (pid == 0) {
            // --- Step 1: Set up isolation ---
            NamespaceManager ns;
            if (!ns.setupNamespaces(true, true, false, false)) {
                exit(1);
            }
            ns.setHostname("mini-container");

            // --- Step 2: Fork again for actual container process ---
            pid_t inner_pid = fork();
            if (inner_pid == 0) {
                // Inside isolated child
                cout << "(["
                     << getpid()
                     << "] <-- Child PID inside container) RUNNING: "
                     << cmd << endl;

                // Execute command inside container
                vector<char*> args;
                args.push_back(const_cast<char*>(cmd.c_str()));
                if (cmd == "bash") args.push_back(const_cast<char*>("-i"));
                args.push_back(nullptr);

                execvp(cmd.c_str(), args.data());
                perror("execvp");
                exit(1);
            } else if (inner_pid > 0) {
                // --- Step 3: Set up cgroups *after* the inner process exists ---
                // (so we can assign its PID correctly)
                CgroupManager cg("container_" + to_string(getpid()));
                cg.limitCPU(50);                 // Limit CPU to 50%
                cg.limitMemory(200 * 1024 * 1024); // 200MB limit
                cg.addProcess(inner_pid);

                // Wait for the process to finish
                waitpid(inner_pid, nullptr, 0);

                // Clean up cgroup
                cg.cleanup();
                exit(0);
            } else {
                perror("fork (inner)");
                exit(1);
            }
        } else if (pid > 0) {
            // --- Step 4: Parent process ---
            cout << "[" << getpid() << "] <-- parent\n";
            waitpid(pid, nullptr, 0);
            cout << "Done!\n\n";
        } else {
            perror("fork");
        }
    }
};
