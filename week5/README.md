## How it works (short flow)

1. On first run, copy host dirs (`/bin`, `/lib`, `/lib64`, `/usr`, `/etc`) into **`/containers/base_rootfs`**.
2. For each launch:
   - Create **`/containers/<prog>_<pid>/rootfs`**.
   - Copy **base_rootfs** into it.
   - Copy your **program** into the new rootfs (it appears as `/<name>` inside).
3. Enter new namespaces (PID, Mount, Hostname, IPC, NET).
4. `chroot` into the run rootfs, mount **/proc**, **/dev** (tmpfs), **/tmp** (tmpfs).
5. `exec` your program inside the container.
6. When it finishes, unmount and delete the run directory.

---

## `container.cpp` (orchestrator)

### Key constants
- `HOST_CONTAINERS_DIR = "/containers"`
- `HOST_BASE_ROOTFS    = "/containers/base_rootfs"`

### Important helpers (host-side)
- `host_shell(cmd)` — run shell command (throws on error).
- `host_mkdir_p(path)` — `mkdir -p`.
- `host_rm_rf(path)` — `rm -rf` if exists.
- `host_path_exists(path)` — check existence.
- `host_file_name(path)` — get last path segment.
- `host_abs_path(path)` — make absolute if needed.
- `host_is_executable(path)` — check execute bit.
- `host_try_umount(path)` — unmount if mounted (lazy).

### Main steps (functions)
- **Base rootfs**
  - `build_base_rootfs_once()` → copy `/bin /lib /lib64 /usr /etc`, create `/dev /proc /sys /tmp`.
  - `ensure_base_rootfs()` → build if missing.
- **Per run**
  - `create_run_dirs_and_copy_base(...)` → make run dir + copy base.
  - `copy_program_into_rootfs(...)` → put program at `/<name>` inside rootfs.
  - `check_is_runnable(...)` → only direct executables (no scripts yet).
  - `build_exec_argv(...)` → argv for `execv`.
  - `run_and_wait(...)` → fork → namespaces → fork → chroot → mounts → exec → wait.
  - `cleanup_run_dir(...)` → umount `/proc /dev /tmp`, remove run dir.
- **Entry point:** `int run_program(const string& program_path, const vector<string>& program_args)`

---

## `namespace_manager.cpp` (namespaces + mounts)

- `nm_make_mounts_private()` → `mount(NULL, "/", NULL, MS_REC|MS_PRIVATE, NULL)` so mount changes don’t leak to host.
- `nm_enter_new_namespaces()` → `unshare(CLONE_NEWUTS|CLONE_NEWNS|CLONE_NEWIPC|CLONE_NEWPID|CLONE_NEWNET)`.
- `nm_chroot_into(new_root)` → `chdir(new_root)`, `chroot(".")`, `chdir("/")`.
- `nm_mount_minimal()` → create + mount essentials:

```c
// 0555 = r-x r-x r-x
mkdir("/proc", 0555);
mount("proc", "/proc", "proc", 0, NULL);

// 0755 = rwx r-x r-x
mkdir("/dev", 0755);
mount("tmpfs", "/dev", "tmpfs", MS_NOSUID|MS_STRICTATIME, "mode=755");

// 01777 = rwx rwx rwx + sticky
mkdir("/tmp", 01777);
mount("tmpfs", "/tmp", "tmpfs", MS_STRICTATIME, "mode=1777");
```

- `nm_set_hostname("mini-container")` → set container hostname.

### Permission quick-notes (octal)
- **0555**
  - Special: `0` none
  - Owner: `5` = r-x
  - Group: `5` = r-x
  - Others: `5` = r-x
- **0755**
  - Special: `0` none
  - Owner: `7` = rwx
  - Group: `5` = r-x
  - Others: `5` = r-x
- **01777**
  - Special: `1` sticky
  - Owner/Group/Others: `7` = rwx