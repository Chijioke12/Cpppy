#!/bin/bash
set -e

echo "=== Installing Python 2 (Required for legacy Emscripten) ==="
apt-get update || true
apt-get install -y python2 python-is-python2 xz-utils || true

echo "=== Downloading Emscripten SDK ==="
rm -rf /tmp/emsdk
git clone https://github.com/emscripten-core/emsdk.git /tmp/emsdk
cd /tmp/emsdk

echo "=== Installing Emscripten 1.38.48 (fastcomp for KaiOS) ==="
./emsdk install 1.38.48
./emsdk activate 1.38.48

echo "=== Done! ==="
echo "To build your KaiOS asm.js game, run:"
echo "npm run build:kaios"
