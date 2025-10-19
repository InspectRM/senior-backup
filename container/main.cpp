#include <iostream>
#include <string>
#include <vector>
using namespace std;

// implemented in container.cpp
int run_program(const string& program_path, const vector<string>& program_args);

static void usage(const char* exe) {
    cerr << "Usage: " << exe << " run <program> [-- <args...>]\n"
         << "Example:\n"
         << "  sudo " << exe << " run ./my_app -- --fullscreen --level 3\n";
}

int main(int argc, char** argv) {
    if (argc < 3) { usage(argv[0]); return 1; }
    string command = argv[1];
    if (command != "run") { usage(argv[0]); return 1; }

    int sep = -1;
    for (int i = 2; i < argc; i++) if (string(argv[i]) == "--") { sep = i; break; }

    string program_path;
    vector<string> program_args;

    if (sep == -1) {
        program_path = argv[2];
        for (int i = 3; i < argc; i++) program_args.push_back(argv[i]);
    } else {
        program_path = argv[2];
        for (int i = sep + 1; i < argc; i++) program_args.push_back(argv[i]);
    }

    return run_program(program_path, program_args);
}
