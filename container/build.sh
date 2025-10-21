#!/usr/bin/env bash
set -e

# === CONFIG ===
CXX="g++"
CXXFLAGS="-std=c++20 -O2 -Wall -Wextra"
BIN_DIR="bin"
TARGET="$BIN_DIR/container"
SRC="main.cpp container.cpp namespace_manager.cpp"

# === BUILD ===
echo "[+] Building Mini Container Runtime..."

# Create bin folder if missing
mkdir -p "$BIN_DIR"

# Compile source files into a single executable
$CXX $CXXFLAGS -o "$TARGET" $SRC

echo "[✓] Build complete: $TARGET"

# === USAGE GUIDE ===
echo ""
echo "Usage:"
echo "  sudo $TARGET run ./my_app -- --foo bar"
echo ""
echo "Example:"
echo "  sudo $TARGET run ./test_program"
echo ""

# === OPTIONAL: RUN IMMEDIATELY ===
# Uncomment this if you want to auto-run after build:
# sudo $TARGET run ./my_app
