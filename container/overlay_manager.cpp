#include <string>
#include <iostream>
#include <cstdlib>
#include <sys/mount.h>
#include <sys/stat.h>

using namespace std;

// Layered Union Filesystem (LUFS) with Copy-on-Write using OverlayFS
class OverlayManager {
private:
    string lowerDir;     // Read-only base layer (shared)
    string upperDir;     // Writable layer (per-container changes)
    string workDir;      // Overlay working directory (metadata)
    string mergedDir;    // Final mounted view (what container sees)
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
        
        // Create directories for overlay layers
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
    
    // Mount the overlay filesystem
    int mountOverlay() {
        // Build overlay mount options
        // Format: lowerdir=<lower>,upperdir=<upper>,workdir=<work>
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
    
    // Unmount the overlay filesystem
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
    
    // Get the merged directory path (what container should chroot into)
    string getMergedDir() const {
        return mergedDir;
    }
    
    // Get the upper directory (where changes are stored)
    string getUpperDir() const {
        return upperDir;
    }
    
    // Check if overlay is mounted
    bool isOverlayMounted() const {
        return isMounted;
    }
    
    // Show overlay statistics
    void showStats() {
        cout << "\n[*] Overlay Filesystem Statistics:\n";
        
        // Count files in upper layer (changes made)
        string cmd = "find " + upperDir + " -type f 2>/dev/null | wc -l";
        cout << "    Files modified/created: ";
        system(cmd.c_str());
        
        // Show disk usage of upper layer
        cmd = "du -sh " + upperDir + " 2>/dev/null";
        cout << "    Upper layer size: ";
        system(cmd.c_str());
    }
    
    // Cleanup overlay directories
    void cleanup() {
        if (isMounted) {
            unmountOverlay();
        }
        
        // Remove overlay directories (but keep upper if you want to preserve changes)
        system(("rm -rf " + workDir + " 2>/dev/null").c_str());
        system(("rm -rf " + mergedDir + " 2>/dev/null").c_str());
        // Note: NOT removing upperDir - contains all container changes!
    }
    
    // Commit changes: merge upper layer into base (advanced feature)
    int commitChanges(const string& newBaseDir) {
        cout << "[+] Committing container changes to new image...\n";
        
        // Copy lower layer to new location
        string cmd = "cp -a " + lowerDir + "/. " + newBaseDir;
        if (system(cmd.c_str()) != 0) {
            cerr << "[!] Failed to copy base layer\n";
            return -1;
        }
        
        // Overlay upper layer changes on top
        cmd = "cp -a " + upperDir + "/. " + newBaseDir;
        if (system(cmd.c_str()) != 0) {
            cerr << "[!] Failed to merge changes\n";
            return -1;
        }
        
        cout << "[+] Changes committed to: " << newBaseDir << "\n";
        return 0;
    }
};