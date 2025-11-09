#include <string>
using namespace std;

// Abstract base class for filesystem operations (POLYMORPHISM!)
class FileSystemBase {
public:
    virtual ~FileSystemBase() {}
    
    //returns true if a file or folder exists on the host.
    virtual bool pathExists(const string& path) = 0;
    
    //runs the command line
    virtual void shell(const string& cmd) = 0;
    
    //make a folder if it doesn't exist.
    virtual void mkdirP(const string& path) = 0;
    
    //remove a file or folder function
    virtual void rmRf(const string& path) = 0;
    
    //get just the file name from a full path.("/a/b/app" -> "app")
    virtual string fileName(const string& path) = 0;
    
    //turn a relative path into an absolute path.
    virtual string absPath(const string& path) = 0;
    
    //check if a file can be run (has execute permission).
    virtual bool isExecutable(const string& path) = 0;
    
    // Unmount a mount point if it is mounted. Do nothing if it's not.
    // Uses lazy unmount so it doesn't hang if busy.
    virtual void tryUmount(const string& mount_path) = 0;
};