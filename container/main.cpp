#include "container.cpp"
#include <iostream>
#include <string>

using namespace std;

class ContainerShell {
public:
    void start() {
        cout << "=== Mini Container Shell ===\nType 'exit' to quit.\n";
        string cmd;
        while (true) {
            cout << "shell> ";
            cin >> cmd;
            if (cmd == "exit") break;
            container.runCommand(cmd);
        }
    }

private:
    Container container;
};

int main() {
    ContainerShell shell;
    shell.start();
    return 0;
}
