#include <iostream>
#include <string>
#include <vector>
using namespace std;

int run_program(const string& program_path, const vector<string>& program_args, bool interactive);

static void usage(const char* exe) {
    cerr << "Usage:\n"
         << "  " << exe << " run <program> [-- <args...>]\n"
         << "  " << exe << " run -i     (interactive shell)\n\n"
         << "Examples:\n"
         << "  sudo " << exe << " run ./my_app\n"
         << "  sudo " << exe << " run -i\n";
}

int main(int argc, char** argv) {
    if (argc < 3) { usage(argv[0]); return 1; }

    string cmd = argv[1];
    if (cmd != "run") { usage(argv[0]); return 1; }

    bool interactive = false;
    string program_path;
    vector<string> program_args;

    // detect -i / --interactive
    if (string(argv[2]) == "-i" || string(argv[2]) == "--interactive") {
        interactive = true;
        program_path = "/bin/bash"; // default shell inside container
    } else {
        int sep = -1;
        for (int i = 2; i < argc; i++)
            if (string(argv[i]) == "--") { sep = i; break; }

        if (sep == -1) {
            program_path = argv[2];
            for (int i = 3; i < argc; i++) program_args.push_back(argv[i]);
        } else {
            program_path = argv[2];
            for (int i = sep + 1; i < argc; i++) program_args.push_back(argv[i]);
        }
    }

    return run_program(program_path, program_args, interactive);
}
