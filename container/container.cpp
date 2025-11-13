#include "host_filesystem.cpp"
#include "network_manager.cpp"

using namespace std;

// Class for managing container operations
class Container {
private:
    FileSystemBase* hostFs;  // POLYMORPHIC POINTER!
    NamespaceManager nsManager;
    NetworkManager netManager;  // Network support!
    
    static const char* HOST_CONTAINERS_DIR;
    static const char* HOST_BASE_ROOTFS;
    
    int buildBaseRootfsOnce() {
        cout << "[+] Building base rootfs at " << HOST_BASE_ROOTFS << " ...\n";
        try {
            hostFs->mkdirP(HOST_BASE_ROOTFS);
            const char* dirs[] = { "/bin", "/lib", "/lib64", "/usr", "/etc" };
            for (const char* d : dirs) {
                cout << "    [+] copying " << d << "\n";
                hostFs->shell("cp -a '" + string(d) + "' '" + string(HOST_BASE_ROOTFS) + "'");
            }
            hostFs->mkdirP(string(HOST_BASE_ROOTFS) + "/dev");
            hostFs->mkdirP(string(HOST_BASE_ROOTFS) + "/proc");
            hostFs->mkdirP(string(HOST_BASE_ROOTFS) + "/sys");
            hostFs->mkdirP(string(HOST_BASE_ROOTFS) + "/tmp");
        } catch (const exception& e) {
            cerr << "[!] build_base_rootfs error: " << e.what() << "\n";
            return -1;
        }
        return 0;
    }

    int ensureBaseRootfs() {
        hostFs->mkdirP(HOST_CONTAINERS_DIR);
        if (!hostFs->pathExists(HOST_BASE_ROOTFS)) return buildBaseRootfsOnce();
        return 0;
    }

    bool createRunDirsAndSetupOverlay(const string& program_path, string& run_id, 
                                       string& run_dir, string& rootfs_dir) {
        string name = hostFs->fileName(program_path);
        run_id     = name + "_" + to_string(getpid());
        run_dir    = string(HOST_CONTAINERS_DIR) + "/" + run_id;

        cout << "[+] Creating container " << run_id << " with overlay filesystem...\n";
        hostFs->mkdirP(run_dir);

        // Setup overlay filesystem (Copy-on-Write!)
        if (overlayMgr.setupOverlay(HOST_BASE_ROOTFS, run_dir) != 0) {
            cerr << "[!] Failed to setup overlay filesystem\n";
            return false;
        }
        
        // Mount the overlay
        if (overlayMgr.mountOverlay() != 0) {
            cerr << "[!] Failed to mount overlay filesystem\n";
            return false;
        }
        
        // Use the merged directory as rootfs
        rootfs_dir = overlayMgr.getMergedDir();
        
        cout << "[+] Container filesystem ready (using overlay - instant creation!)\n";
        return true;
    }

    bool copyProgramIntoRootfs(const string& program_path, const string& rootfs_dir, 
                                string& program_inside) {
        string abs_prog = hostFs->absPath(program_path);
        if (!hostFs->pathExists(abs_prog)) {
            cerr << "[!] Program not found: " << abs_prog << "\n";
            return false;
        }
        string name = hostFs->fileName(abs_prog);
        try {
            hostFs->shell("cp -a '" + abs_prog + "' '" + rootfs_dir + "/" + name + "'");
        } catch (const exception& e) {
            cerr << "[!] copy program failed: " << e.what() << "\n";
            return false;
        }
        program_inside = "/" + name;
        return true;
    }

    bool checkIsRunnable(const string& host_abs_program) {
        if (!hostFs->isExecutable(host_abs_program)) {
            cerr << "[!] File is not executable (no script support yet).\n"
                 << "    chmod +x " << host_abs_program << "\n";
            return false;
        }
        return true;
    }

    void buildExecArgv(const string& program_inside, const vector<string>& program_args,
                        vector<string>& argv_strings, vector<char*>& argv_cstrings) {
        argv_strings.clear(); argv_cstrings.clear();
        argv_strings.push_back(program_inside);
        for (const auto& a : program_args) argv_strings.push_back(a);
        for (auto& s : argv_strings) argv_cstrings.push_back(const_cast<char*>(s.c_str()));
        argv_cstrings.push_back(nullptr);
    }

    int runAndWait(const string& rootfs_dir, const vector<char*>& argv_cstrings, bool interactive, bool enableNetwork) {
        pid_t child = fork();
        if (child < 0) { perror("fork"); return 1; }

        if (child == 0) {
            if (nsManager.enterNewNamespaces() != 0) { cerr << "[!] unshare failed\n"; _exit(1); }

            pid_t init_pid = fork();
            if (init_pid < 0) { perror("fork"); _exit(1); }

            if (init_pid == 0) {
                // Give parent time to set up networking
                if (enableNetwork) {
                    sleep(1);
                }
                
                if (nsManager.chrootInto(rootfs_dir.c_str()) != 0) { cerr << "[!] chroot failed\n"; _exit(1); }
                if (nsManager.mountMinimal() != 0)                 { cerr << "[!] mount failed\n"; _exit(1); }
                nsManager.setHostname("mini-container");

                // Configure container network if enabled
                if (enableNetwork) {
                    netManager.setupContainerSide();
                }

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

        // Parent process: Set up networking for child
        if (enableNetwork) {
            cout << "[+] Setting up container networking...\n";
            
            // Create veth pair
            if (netManager.createVethPair() != 0) {
                cerr << "[!] Failed to create network, continuing without networking\n";
            }
            else {
                // Move one end into container
                if (netManager.moveVethToContainer(child) != 0) {
                    cerr << "[!] Failed to move veth to container\n";
                }
                else {
                    // Configure host side
                    if (netManager.setupHostSide() != 0) {
                        cerr << "[!] Failed to configure host network\n";
                    }
                    else {
                        // Enable internet access (NAT)
                        netManager.enableInternetAccess();
                    }
                }
            }
        }

        int st = 0; waitpid(child, &st, 0);
        return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : 1;
    }

    void cleanupRunDir(const string& rootfs_dir, const string& run_dir) {
        cout << "[+] Cleaning up...\n";
        hostFs->tryUmount(rootfs_dir + "/proc");
        hostFs->tryUmount(rootfs_dir + "/dev");
        hostFs->tryUmount(rootfs_dir + "/tmp");
        hostFs->rmRf(run_dir);
        
        // Cleanup network
        netManager.cleanup();
        
        cout << "[✓] Done.\n";
    }

public:
    // Constructor with polymorphism support
    Container() {
        hostFs = new HostFileSystem();  // Create concrete implementation
    }
    
    // Destructor to clean up
    ~Container() {
        delete hostFs;
    }
    
    int runProgram(const string& program_path, const vector<string>& program_args, bool interactive, bool enableNetwork = false) {
        if (geteuid() != 0) { cerr << "[!] run as root (sudo)\n"; return 1; }
        if (ensureBaseRootfs() != 0) return 1;

        string run_id, run_dir, rootfs_dir;
        if (!createRunDirsAndCopyBase(program_path, run_id, run_dir, rootfs_dir)) return 1;

        string program_inside;
        if (!copyProgramIntoRootfs(program_path, rootfs_dir, program_inside)) {
            hostFs->rmRf(run_dir); return 1;
        }

        string host_abs = hostFs->absPath(program_path);
        if (!checkIsRunnable(host_abs)) {
            hostFs->rmRf(run_dir); return 1;
        }

        vector<string> argv_strings;
        vector<char*>  argv_cstrings;
        buildExecArgv(program_inside, program_args, argv_strings, argv_cstrings);

        int rc = runAndWait(rootfs_dir, argv_cstrings, interactive, enableNetwork);
        //comign back later experimenting /bin/bash
        cleanupRunDir(rootfs_dir, run_dir);
        return rc;
    }
};

// Initialize static members
const char* Container::HOST_CONTAINERS_DIR = "/containers";
const char* Container::HOST_BASE_ROOTFS    = "/containers/base_rootfs";