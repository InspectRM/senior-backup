# Senior Project Library

Docker Based Conainter Engine: Process, Resource, and Filesystem Isolation

## 🏗️ System Architecture (possible future implementations)
```
┌─────────────────────────────────────────────────────────────┐
│                     ContainerOS Engine                      │
├─────────────────────────────────────────────────────────────┤
│ ┌─────────────┐   ┌─────────────┐   ┌─────────────────────┐ │
│ │ Process     │   │ Resource    │   │ Filesystem          │ │
│ │ Isolation   │   │ Controller  │   │ Manager             │ │
│ │             │   │             │   │                     │ │
│ │ • PID NS    │   │ • CPU limits│   │ • Union FS          │ │
│ │ • Mount NS  │   │ • Memory    │   │ • Copy-on-Write     │ │
│ │ • UTS NS    │   │ • I/O BW    │   │ • Layer management  │ │
│ │ • IPC NS    │   │ • Cgroups v2│   │ • Snapshotting      │ │
│ └─────────────┘   └─────────────┘   └─────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│ ┌─────────────┐   ┌─────────────┐   ┌─────────────────────┐ │
│ │ Network     │   │ Image       │   │ Security            │ │
│ │ Manager     │   │ Registry    │   │ Manager             │ │
│ │             │   │             │   │                     │ │
│ │ • Net NS    │   │ • Layer pull│   │ • Capabilities      │ │
│ │ • Virtual   │   │ • Cache mgmt│   │ • Seccomp profiles  │ │
│ │   interfaces│   │ • Versioning│   │ • AppArmor          │ │
│ │ • NAT       │   │ • Storage   │   │ • Namespace opts    │ │
│ └─────────────┘   └─────────────┘   └─────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                     Linux Kernel APIs                       │
├─────────────────────────────────────────────────────────────┤
│ ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌────────────┐ │
│ │Namespaces│   │ Cgroups  │   │ UnionFS  │   │ Netfilter  │ │
│ │          │   │          │   │          │   │            │ │
│ │• PID     │   │• CPU     │   │• Overlay │   │• iptables  │ │
│ │• Mount   │   │• Memory  │   │• AuFS    │   │• bridge    │ │
│ │• Network │   │• I/O     │   │• Snapshot│   │• veth pairs│ │
│ │• UTS     │   │• Devices │   │          │   │            │ │
│ │• IPC     │   │          │   │          │   │            │ │
│ │• User    │   │          │   │          │   │            │ │
│ └──────────┘   └──────────┘   └──────────┘   └────────────┘ │
└─────────────────────────────────────────────────────────────┘
```


### Basic Usage
# Run a simple container
~coming soon~

# Run with resource limits
~coming soon~
<img width="523" height="193" alt="image" src="https://github.com/user-attachments/assets/7e92ecd4-e4da-451a-9f44-aa1eb0fe4de6" />

<img width="645" height="106" alt="image" src="https://github.com/user-attachments/assets/23466874-5f5d-40f5-a3a3-737a50783340" />
