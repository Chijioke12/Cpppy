#!/bin/bash
set -e

# Setup Emscripten path if present in /tmp/emsdk
export PATH="/tmp/bin:/tmp/emsdk/upstream/emscripten:/tmp/emsdk/upstream/bin:$PATH"

echo "=== Building C++ Physics Game with Emscripten ==="

mkdir -p dist
mkdir -p public

em++ -O3 -std=c++17 \
  -s USE_SDL=2 \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s NO_EXIT_RUNTIME=1 \
  --shell-file shell_minimal.html \
  -I./cpp/include \
  ./cpp/src/main.cpp \
  -o ./dist/index.html

# Also copy to public/ and root index.html for live preview
cp ./dist/index.html ./index.html 2>/dev/null || true
cp -r ./dist/* ./public/ 2>/dev/null || true

echo "=== Build Complete! Output in ./dist/ (index.html, index.js, index.wasm) ==="
