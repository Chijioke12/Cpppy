#!/bin/bash
set -e

# ==============================================================================
# KaiOS 2.5 (asm.js) & Modern Web (WebAssembly) Dual Build Script
# ==============================================================================

MODE="${1:-kaios}"
echo "=== Building Physics Engine in mode: $MODE ==="

mkdir -p dist
mkdir -p public

# Check if em++ exists or fallback to pre-built dist
EMCC_BIN=""
if command -v em++ >/dev/null 2>&1; then
    EMCC_BIN="em++"
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

if [ "$MODE" = "asmjs" ] || [ "$MODE" = "kaios" ]; then
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
else
    echo "--- Compiling WebAssembly for Modern Browsers ---"
    
    $EMCC_BIN -O3 -std=c++17 \
      -s USE_SDL=2 \
      -s WASM=1 \
      -s ALLOW_MEMORY_GROWTH=1 \
      -s NO_EXIT_RUNTIME=1 \
      --minify 0 \
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
fi

# Ensure valid <!DOCTYPE html> header
node -e "
const fs = require('fs');
let html = fs.readFileSync('./dist/index.html', 'utf8');
html = html.replace(/<!doctypehtml>/gi, '<!DOCTYPE html>\n');
if (!html.startsWith('<!DOCTYPE html>')) {
  html = '<!DOCTYPE html>\n' + html.replace(/^<!DOCTYPE[^>]*>/i, '');
}
fs.writeFileSync('./dist/index.html', html);
fs.writeFileSync('./index.html', html);
"

cp -r ./dist/* ./public/ 2>/dev/null || true
cp ./public/manifest.webapp ./dist/manifest.webapp 2>/dev/null || true

echo "=== Build Complete! Output in ./dist/ ==="
