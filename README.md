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
│ ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌─────────────────┐ │
│ │Namespaces│   │ Cgroups  │   │ UnionFS  │   │ Netfilter       │ │
│ │          │   │          │   │          │   │                 │ │
│ │• PID     │   │• CPU     │   │• Overlay │   │• iptables       │ │
│ │• Mount   │   │• Memory  │   │• AuFS    │   │• bridge         │ │
│ │• Network │   │• I/O     │   │• Snapshot│   │• veth pairs     │ │
│ │• UTS     │   │• Devices │   │          │   │                 │ │
│ │• IPC     │   │          │   │          │   │                 │ │
│ │• User    │   │          │   │          │   │                 │ │
│ └──────────┘   └──────────┘   └──────────┘   └─────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```
### Installation
git clone https://github.com/your-username/containeros-engine.git
cd containeros-engine
mkdir build && cd build
cmake ..
make

### Basic Usage
# Run a simple container
./containeros run ubuntu:latest /bin/bash

# Run with resource limits
./containeros run --memory=100m --cpus=0.5 alpine /bin/sh
