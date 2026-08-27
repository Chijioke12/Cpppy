#!/bin/bash
set -e

# ==============================================================================
# KaiOS 2.5 (asm.js) Build Script
# ==============================================================================

echo "=== Building Physics Engine for KaiOS (asm.js) ==="

mkdir -p dist
mkdir -p public

# Check if em++ exists or fallback to pre-built dist
EMCC_BIN=""
if command -v em++ >/dev/null 2>&1; then
    EMCC_BIN="em++"
elif [ -f "/tmp/emsdk/fastcomp/emscripten/em++" ]; then
    EMCC_BIN="/tmp/emsdk/fastcomp/emscripten/em++"
    export PATH="/tmp/emsdk/node/14.18.2_64bit/bin:/tmp/emsdk/fastcomp/emscripten:/tmp/emsdk/fastcomp/bin:$PATH"
elif [ -f "/tmp/emsdk/upstream/emscripten/em++" ]; then
    EMCC_BIN="/tmp/emsdk/upstream/emscripten/em++"
    export PATH="/tmp/bin:/tmp/emsdk/upstream/emscripten:/tmp/emsdk/upstream/bin:$PATH"
elif [ -f "/tmp/bin/em++" ]; then
    EMCC_BIN="/tmp/bin/em++"
    export PATH="/tmp/bin:$PATH"
fi

if [ -z "$EMCC_BIN" ]; then
    echo "Using existing pre-built artifacts in ./public/ -> ./dist/"
    cp -r ./public/* ./dist/ 2>/dev/null || true
    echo "=== Build Complete! Output in ./dist/ ==="
    exit 0
fi

echo "--- Compiling pure asm.js for KaiOS 2.5 (Gecko 48) ---"

$EMCC_BIN -O3 -std=c++17 \
  -s WASM=0 \
  -s LEGACY_VM_SUPPORT=1 \
  -s ALLOW_MEMORY_GROWTH=0 \
  -s TOTAL_MEMORY=33554432 \
  -s USE_SDL=2 \
  -s NO_EXIT_RUNTIME=1 \
  --memory-init-file 1 \
  --shell-file shell_minimal.html \
  -I./cpp/include \
  -I./cpp/src/box2d_src \
  -I./cpp/src/box2d_src/dynamics \
  -I./cpp/src/box2d_src/collision \
  -I./cpp/src/box2d_src/common \
  -I./cpp/src/box2d_src/rope \
  ./cpp/src/box2d_src/collision/*.cpp \
  ./cpp/src/box2d_src/common/*.cpp \
  ./cpp/src/box2d_src/dynamics/*.cpp \
  ./cpp/src/box2d_src/rope/*.cpp \
  ./cpp/src/main.cpp \
  -o ./dist/index.html

# Ensure valid <!DOCTYPE html> header and proper HTML attribute spacing
node scripts/postprocess_html.js

cp -r ./dist/* ./public/ 2>/dev/null || true
cp ./public/manifest.webapp ./dist/manifest.webapp 2>/dev/null || true

# Generate OmniSD SD card sideload package (application.zip + update.webapp + metadata.json)
node scripts/package_omnisd.js

echo "=== Build Complete! Output in ./dist/ ==="
