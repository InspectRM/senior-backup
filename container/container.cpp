#define _GNU_SOURCE
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

// from namespace_manager.cpp
int nm_enter_new_namespaces();
int nm_chroot_into(const char* new_root);
int nm_mount_minimal();
int nm_set_hostname(const char* name);

static const char* HOST_CONTAINERS_DIR = "/containers";
static const char* HOST_BASE_ROOTFS    = "/containers/base_rootfs";

//returns true if a file or folder exists on the host.
static bool host_path_exists(const string& path) {
    struct stat st; return stat(path.c_str(), &st) == 0;
}

//runs the command line
static void host_shell(const string& cmd) {
    int rc = system(cmd.c_str());
    if (rc != 0) throw runtime_error("Command failed: " + cmd);
}

//make a folder if it doesn't exist.
static void host_mkdir_p(const string& path) {
    host_shell("mkdir -p '" + path + "'");
}

//remove a file or folder function
static void host_rm_rf(const string& path) {
    if (!host_path_exists(path)) return;
    host_shell("rm -rf '" + path + "'");
}

//get just the file name from a full path.("/a/b/app" -> "app")
static string host_file_name(const string& path) {
    size_t slash = path.find_last_of('/');
    return (slash == string::npos) ? path : path.substr(slash + 1);
}

//turn a relative path into an absolute path.
static string host_abs_path(const string& path) {
    if (!path.empty() && path[0] == '/') return path;
    char cwd[4096]; getcwd(cwd, sizeof(cwd)); return string(cwd) + "/" + path;
}

//check if a file can be run (has execute permission).
static bool host_is_executable(const string& path) {
    return access(path.c_str(), X_OK) == 0;
}

// Unmount a mount point if it is mounted. Do nothing if it's not.
// Uses lazy unmount so it doesn't hang if busy.
static void host_try_umount(const string& mount_path) {
    host_shell("mountpoint -q '" + mount_path + "' && umount -l '" + mount_path + "' || true");
}


static int build_base_rootfs_once() {
    cout << "[+] Building base rootfs at " << HOST_BASE_ROOTFS << " ...\n";
    try {
        host_mkdir_p(HOST_BASE_ROOTFS);
        const char* dirs[] = { "/bin", "/lib", "/lib64", "/usr", "/etc" };
        for (const char* d : dirs) {
            cout << "    [+] copying " << d << "\n";
            host_shell("cp -a '" + string(d) + "' '" + string(HOST_BASE_ROOTFS) + "'");
        }
        host_mkdir_p(string(HOST_BASE_ROOTFS) + "/dev");
        host_mkdir_p(string(HOST_BASE_ROOTFS) + "/proc");
        host_mkdir_p(string(HOST_BASE_ROOTFS) + "/sys");
        host_mkdir_p(string(HOST_BASE_ROOTFS) + "/tmp");
    } catch (const exception& e) {
        cerr << "[!] build_base_rootfs error: " << e.what() << "\n";
        return -1;
    }
    return 0;
}

static int ensure_base_rootfs() {
    host_mkdir_p(HOST_CONTAINERS_DIR);
    if (!host_path_exists(HOST_BASE_ROOTFS)) return build_base_rootfs_once();
    return 0;
}

static bool create_run_dirs_and_copy_base(const string& program_path,
                                          string& run_id,
                                          string& run_dir,
                                          string& rootfs_dir) {
    string name = host_file_name(program_path);
    run_id     = name + "_" + to_string(getpid());
    run_dir    = string(HOST_CONTAINERS_DIR) + "/" + run_id;
    rootfs_dir = run_dir + "/rootfs";

    cout << "[+] Creating container " << run_id << " ...\n";
    host_mkdir_p(rootfs_dir);

    cout << "[+] Copying base rootfs (may take a moment)...\n";
    try {
        host_shell("cp -a '" + string(HOST_BASE_ROOTFS) + "/.' '" + rootfs_dir + "'");
    } catch (const exception& e) {
        cerr << "[!] copy base rootfs failed: " << e.what() << "\n";
        return false;
    }
    return true;
}

static bool copy_program_into_rootfs(const string& program_path,
                                     const string& rootfs_dir,
                                     string& program_inside) {
    string abs_prog = host_abs_path(program_path);
    if (!host_path_exists(abs_prog)) {
        cerr << "[!] Program not found: " << abs_prog << "\n";
        return false;
    }
    string name = host_file_name(abs_prog);
    try {
        host_shell("cp -a '" + abs_prog + "' '" + rootfs_dir + "/" + name + "'");
    } catch (const exception& e) {
        cerr << "[!] copy program failed: " << e.what() << "\n";
        return false;
    }
    program_inside = "/" + name;
    return true;
}

static bool check_is_runnable(const string& host_abs_program) {
    if (!host_is_executable(host_abs_program)) {
        cerr << "[!] File is not executable (no script support yet).\n"
             << "    chmod +x " << host_abs_program << "\n";
        return false;
    }
    return true;
}

static void build_exec_argv(const string& program_inside,
                            const vector<string>& program_args,
                            vector<string>& argv_strings,
                            vector<char*>& argv_cstrings) {
    argv_strings.clear(); argv_cstrings.clear();
    argv_strings.push_back(program_inside);
    for (const auto& a : program_args) argv_strings.push_back(a);
    for (auto& s : argv_strings) argv_cstrings.push_back(const_cast<char*>(s.c_str()));
    argv_cstrings.push_back(nullptr);
}

static int run_and_wait(const string& rootfs_dir,const vector<char*>& argv_cstrings,bool interactive){
    pid_t child = fork();
    if (child < 0) { perror("fork"); return 1; }

    if (child == 0) {
        if (nm_enter_new_namespaces() != 0) { cerr << "[!] unshare failed\n"; _exit(1); }

        pid_t init_pid = fork();
        if (init_pid < 0) { perror("fork"); _exit(1); }

        if (init_pid == 0) {
            if (nm_chroot_into(rootfs_dir.c_str()) != 0) { cerr << "[!] chroot failed\n"; _exit(1); }
            if (nm_mount_minimal() != 0)                 { cerr << "[!] mount failed\n"; _exit(1); }
            nm_set_hostname("mini-container");

            if (interactive) {
                cout << "[+] Entering interactive container shell...\n";
                execl("/bin/bash", "/bin/bash", nullptr);
            } else {
                execv(argv_cstrings[0], (char* const*)argv_cstrings.data());
            }

            perror("execv");
            _exit(127);
        }
        else {
            int st = 0; waitpid(init_pid, &st, 0);
            _exit((WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : 1);
        }
    }

    int st = 0; waitpid(child, &st, 0);
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : 1;
}

static void cleanup_run_dir(const string& rootfs_dir, const string& run_dir) {
    cout << "[+] Cleaning up...\n";
    host_try_umount(rootfs_dir + "/proc");
    host_try_umount(rootfs_dir + "/dev");
    host_try_umount(rootfs_dir + "/tmp");
    host_rm_rf(run_dir);
    cout << "[✓] Done.\n";
}

int run_program(const string& program_path, const vector<string>& program_args, bool interactive){
    if (geteuid() != 0) { cerr << "[!] run as root (sudo)\n"; return 1; }
    if (ensure_base_rootfs() != 0) return 1;

    string run_id, run_dir, rootfs_dir;
    if (!create_run_dirs_and_copy_base(program_path, run_id, run_dir, rootfs_dir)) return 1;

    string program_inside;
    if (!copy_program_into_rootfs(program_path, rootfs_dir, program_inside)) {
        host_rm_rf(run_dir); return 1;
    }

    string host_abs = host_abs_path(program_path);
    if (!check_is_runnable(host_abs)) {
        host_rm_rf(run_dir); return 1;
    }

    vector<string> argv_strings;
    vector<char*>  argv_cstrings;
    build_exec_argv(program_inside, program_args, argv_strings, argv_cstrings);

    int rc = run_and_wait(rootfs_dir, argv_cstrings, interactive);
    //comign back later experimenting /bin/bash
    cleanup_run_dir(rootfs_dir, run_dir);
    return rc;
}
