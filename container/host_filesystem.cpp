#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include "namespace_manager.cpp"
#include "filesystem_base.cpp"

using namespace std;

class HostFileSystem : public FileSystemBase {
public:
    //returns true if a file or folder exists on the host.
    bool pathExists(const string& path) override {
        struct stat st; return stat(path.c_str(), &st) == 0;
    }
    
    //runs the command line
    void shell(const string& cmd) override {
        int rc = system(cmd.c_str());
        if (rc != 0) throw runtime_error("Command failed: " + cmd);
    }
    
    //make a folder if it doesn't exist.
    void mkdirP(const string& path) override {
        shell("mkdir -p '" + path + "'");
    }
    
    //remove a file or folder function
    void rmRf(const string& path) override {
        if (!pathExists(path)) return;
        shell("rm -rf '" + path + "'");
    }
    
    //get just the file name from a full path.("/a/b/app" -> "app")
    string fileName(const string& path) override {
        size_t slash = path.find_last_of('/');
        return (slash == string::npos) ? path : path.substr(slash + 1);
    }
    
    //turn a relative path into an absolute path.
    string absPath(const string& path) override {
        if (!path.empty() && path[0] == '/') return path;
        char cwd[4096]; getcwd(cwd, sizeof(cwd)); return string(cwd) + "/" + path;
    }
    
    //check if a file can be run (has execute permission).
    bool isExecutable(const string& path) override {
        return access(path.c_str(), X_OK) == 0;
    }
    
    // Unmount a mount point if it is mounted. Do nothing if it's not.
    // Uses lazy unmount so it doesn't hang if busy.
    void tryUmount(const string& mount_path) override {
        shell("mountpoint -q '" + mount_path + "' && umount -l '" + mount_path + "' || true");
    }
};