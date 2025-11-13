#include <string>
#include <iostream>
#include <cstdlib>
#include <sys/mount.h>
#include <sys/stat.h>

using namespace std;

// Layered Union Filesystem (LUFS) with Copy-on-Write using OverlayFS
class OverlayManager {
private:
    string lowerDir;   
    string upperDir;   
    string workDir;      
    string mergedDir;  
    bool isMounted;
    
public:
    OverlayManager() : isMounted(false) {}
    
    // Initialize overlay filesystem for a container
    int setupOverlay(const string& baseRootfs, const string& containerDir) {
        // Define layer directories
        lowerDir = baseRootfs;                    // Base template (read-only)
        upperDir = containerDir + "/upper";       // Container-specific changes
        workDir = containerDir + "/work";         // Overlay metadata
        mergedDir = containerDir + "/merged";     // Final view
        
        if (mkdir(upperDir.c_str(), 0755) != 0 && errno != EEXIST) {
            perror("mkdir upper");
            return -1;
        }
        
        if (mkdir(workDir.c_str(), 0755) != 0 && errno != EEXIST) {
            perror("mkdir work");
            return -1;
        }
        
        if (mkdir(mergedDir.c_str(), 0755) != 0 && errno != EEXIST) {
            perror("mkdir merged");
            return -1;
        }
        
        cout << "[+] Created overlay filesystem layers\n";
        cout << "    Lower (read-only): " << lowerDir << "\n";
        cout << "    Upper (writable):  " << upperDir << "\n";
        cout << "    Merged (final):    " << mergedDir << "\n";
        
        return 0;
    }
    
    int mountOverlay() {
        // lowerdir=<lower>,upperdir=<upper>,workdir=<work>
        string options = "lowerdir=" + lowerDir + 
                        ",upperdir=" + upperDir + 
                        ",workdir=" + workDir;
        
        // Mount overlay filesystem
        if (mount("overlay", mergedDir.c_str(), "overlay", 0, options.c_str()) != 0) {
            perror("mount overlay");
            cerr << "[!] Failed to mount overlay filesystem\n";
            cerr << "[!] Options: " << options << "\n";
            return -1;
        }
        
        isMounted = true;
        cout << "[+] Mounted overlay filesystem at " << mergedDir << "\n";
        cout << "[+] Copy-on-Write enabled: changes saved to upper layer\n";
        
        return 0;
    }
    
    int unmountOverlay() {
        if (!isMounted) return 0;
        
        if (umount2(mergedDir.c_str(), MNT_DETACH) != 0) {
            perror("umount overlay");
            return -1;
        }
        
        isMounted = false;
        cout << "[+] Unmounted overlay filesystem\n";
        return 0;
    }
    
    string getMergedDir() const {
        return mergedDir;
    }
    
    string getUpperDir() const {
        return upperDir;
    }
    
    bool isOverlayMounted() const {
        return isMounted;
    }
    
    void showStats() {
        cout << "\n[*] Overlay Filesystem Statistics:\n";
        
        string cmd = "find " + upperDir + " -type f 2>/dev/null | wc -l";
        cout << "    Files modified/created: ";
        system(cmd.c_str());
        
        cmd = "du -sh " + upperDir + " 2>/dev/null";
        cout << "    Upper layer size: ";
        system(cmd.c_str());
    }
    
    void cleanup() {
        if (isMounted) {
            unmountOverlay();
        }
        
        system(("rm -rf " + workDir + " 2>/dev/null").c_str());
        system(("rm -rf " + mergedDir + " 2>/dev/null").c_str());
        // Note: NOT removing upperDir - contains all container changes!
    }
    
    int commitChanges(const string& newBaseDir) {
        cout << "[+] Committing container changes to new image...\n";
        
        string cmd = "cp -a " + lowerDir + "/. " + newBaseDir;
        if (system(cmd.c_str()) != 0) {
            cerr << "[!] Failed to copy base layer\n";
            return -1;
        }
        
        cmd = "cp -a " + upperDir + "/. " + newBaseDir;
        if (system(cmd.c_str()) != 0) {
            cerr << "[!] Failed to merge changes\n";
            return -1;
        }
        
        cout << "[+] Changes committed to: " << newBaseDir << "\n";
        return 0;
    }
};