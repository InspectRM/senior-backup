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
    
    Create virtual ethernet pair
    void createVethPair() {
        string cmd = "ip link add " + vethHost + " type veth peer name " + vethContainer;
        system(cmd.c_str());
        cout << "[+] Created network pair\n";
    }
    
    void moveToContainer(pid_t containerPid) {
        string cmd = "ip link set " + vethContainer + " netns " + to_string(containerPid);
        system(cmd.c_str());
        cout << "[+] Moved interface to container\n";
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