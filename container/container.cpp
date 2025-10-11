#include "namespace_manager.cpp"
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
            NamespaceManager ns;
            if (!ns.setupNamespaces(true, true, false, false)) {
                exit(1);
            }
            ns.setHostname("mini-container");

            pid_t inner_pid = fork();
            if (inner_pid == 0) {
                cout << "(["
                          << getpid()
                          << "] <-- Child PID inside container) RUNNING: "
                          << cmd << endl;

                vector<char*> args;
                args.push_back(const_cast<char*>(cmd.c_str()));
                if (cmd == "bash") args.push_back(const_cast<char*>("-i"));
                args.push_back(nullptr);

                execvp(cmd.c_str(), args.data());
                perror("execvp");
                exit(1);
            } else {
                waitpid(inner_pid, nullptr, 0);
                exit(0);
            }
        } else {
            cout << "[" << getpid() << "] <-- parent\n";
            waitpid(pid, nullptr, 0);
            cout << "Done!\n\n";
        }
    }
};
