
///  TERMORARELY OUT OF ORDER , NOT NEEDED AT THE MOMENT! ///
///  TERMORARELY OUT OF ORDER , NOT NEEDED AT THE MOMENT! ///
///  TERMORARELY OUT OF ORDER , NOT NEEDED AT THE MOMENT! ///



// #include <iostream>
// #include <fstream>
// #include <string>
// #include <sys/stat.h>
// #include <unistd.h>

// using namespace std;

// class CgroupManager {
// public:
//     string path;

//     // Constructor: create a cgroup directory for this container
//     CgroupManager(const string &name) {
//         path = "/sys/fs/cgroup/" + name;

//         if (mkdir(path.c_str(), 0755) == -1 && access(path.c_str(), F_OK) != 0) {
//             perror("[CgroupManager] mkdir");
//         } else {
//             cout << "[CgroupManager] Created: " << path << endl;
//         }
//     }

//     // Add process (by PID) to the cgroup
//     bool addProcess(pid_t pid) {
//         string filePath = path + "/cgroup.procs";
//         ofstream file(filePath);
//         if (!file.is_open()) {
//             perror("[CgroupManager] Error opening cgroup.procs");
//             return false;
//         }
//         file << pid;
//         file.close();
//         cout << "[CgroupManager] Added PID " << pid << " to " << filePath << endl;
//         return true;
//     }

//     // Limit CPU (percentage-based control)
//     bool limitCPU(int percent) {
//         string filePath = path + "/cpu.max";
//         ofstream file(filePath);
//         if (!file.is_open()) {
//             perror("[CgroupManager] Error opening cpu.max");
//             return false;
//         }

//         // Format: <max> <period>
//         // e.g., 50% = 50ms out of every 100ms
//         int quota = percent * 1000;   // convert percent to microseconds
//         file << quota << " 100000";
//         file.close();

//         cout << "[CgroupManager] CPU limit set to " << percent << "% for " << path << endl;
//         return true;
//     }

//     // Limit Memory (in bytes)
//     bool limitMemory(size_t bytes) {
//         string filePath = path + "/memory.max";
//         ofstream file(filePath);
//         if (!file.is_open()) {
//             perror("[CgroupManager] Error opening memory.max");
//             return false;
//         }

//         file << bytes;
//         file.close();

//         cout << "[CgroupManager] Memory limit set to " << bytes / (1024 * 1024) << " MB for " << path << endl;
//         return true;
//     }

//     // Remove the cgroup after container exits
//     void cleanup() {
//         if (rmdir(path.c_str()) == -1) {
//             perror("[CgroupManager] Error removing cgroup directory");
//         } else {
//             cout << "[CgroupManager] Cleaned up " << path << endl;
//         }
//     }
// };
