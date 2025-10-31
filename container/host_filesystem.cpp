#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include "namespace_manager.cpp"

using namespace std;

// Class for host file system operations
class HostFileSystem {
public:
    //returns true if a file or folder exists on the host.
    bool pathExists(const string& path) {
        struct stat st; return stat(path.c_str(), &st) == 0;
    }
    
    //runs the command line
    void shell(const string& cmd) {
        int rc = system(cmd.c_str());
        if (rc != 0) throw runtime_error("Command failed: " + cmd);
    }
    
    //make a folder if it doesn't exist.
    void mkdirP(const string& path) {
        shell("mkdir -p '" + path + "'");
    }
    
    //remove a file or folder function
    void rmRf(const string& path) {
        if (!pathExists(path)) return;
        shell("rm -rf '" + path + "'");
    }
    
    //get just the file name from a full path.("/a/b/app" -> "app")
    string fileName(const string& path) {
        size_t slash = path.find_last_of('/');
        return (slash == string::npos) ? path : path.substr(slash + 1);
    }
    
    //turn a relative path into an absolute path.
    string absPath(const string& path) {
        if (!path.empty() && path[0] == '/') return path;
        char cwd[4096]; getcwd(cwd, sizeof(cwd)); return string(cwd) + "/" + path;
    }
    
    //check if a file can be run (has execute permission).
    bool isExecutable(const string& path) {
        return access(path.c_str(), X_OK) == 0;
    }
    
    // Unmount a mount point if it is mounted. Do nothing if it's not.
    // Uses lazy unmount so it doesn't hang if busy.
    void tryUmount(const string& mount_path) {
        shell("mountpoint -q '" + mount_path + "' && umount -l '" + mount_path + "' || true");
    }
};