#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

class NetworkManager {
private:
    string vethHost;
    string vethContainer;
    
public:
    NetworkManager() {
        vethHost = "veth_h" + to_string(getpid());
        vethContainer = "veth_c" + to_string(getpid());
    }
    
    int createVethPair() {
        string cmd = "ip link add " + vethHost + " type veth peer name " + vethContainer;
        if (system(cmd.c_str()) != 0) {
            cerr << "[!] Failed to create veth pair\n";
            return -1;
        }
        cout << "[+] Created veth pair: " << vethHost << " <-> " << vethContainer << "\n";
        return 0;
    }
    
    int moveVethToContainer(pid_t containerPid) {
        string cmd = "ip link set " + vethContainer + " netns " + to_string(containerPid);
        if (system(cmd.c_str()) != 0) {
            cerr << "[!] Failed to move veth to container\n";
            return -1;
        }
        cout << "[+] Moved " << vethContainer << " into container namespace\n";
        return 0;
    }
    
    int setupHostSide() {
        string cmd1 = "ip addr add " + hostIP + "/24 dev " + vethHost;
        if (system(cmd1.c_str()) != 0) {
            cerr << "[!] Failed to set host IP\n";
            return -1;
        }
        
        string cmd2 = "ip link set " + vethHost + " up";
        if (system(cmd2.c_str()) != 0) {
            cerr << "[!] Failed to bring up host interface\n";
            return -1;
        }
        
        cout << "[+] Configured host interface " << vethHost << " (" << hostIP << ")\n";
        return 0;
    }
    
    void setupHostSide() {
        //create a localhost
    }
    

    void setupContainerSide() {
//call from inside container
    }
    
    void enableInternet() {
    // Step 5: Enable internet (one-time setup on host)
    }
    
    
    void cleanup() {
        system(("ip link del " + vethHost).c_str());
    }
    
    string getContainerVeth() { return vethContainer; }
};
