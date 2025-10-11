# Senior Project Library

Docker Based Conainter Engine: Process, Resource, and Filesystem Isolation

## System Architecture

ContainerOS Engine
├── Process Isolation Manager
├── Resource Management (cgroups)
├── File System Layers
├── Network Namespace Controller
├── Image Manager
└── Security Manager

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
