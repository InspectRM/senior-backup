## How `unshare()` Works

- `unshare()` is a Linux system call that **detaches the calling process from the host’s namespaces** and creates new ones.

- **Header:**

```c
#include <sched.h>
int unshare(int flags);
```

- **Example:** create new UTS + PID namespaces

```c
unshare(CLONE_NEWUTS | CLONE_NEWPID);
```

- After calling this, the process is running in new isolated namespaces.

- You can then run:

```c
sethostname("container1", 10);
```

to change the hostname just for this namespace.
