#!/bin/bash
set -e

# Setup Emscripten path if present in /tmp/emsdk
export PATH="/tmp/bin:/tmp/emsdk/upstream/emscripten:/tmp/emsdk/upstream/bin:$PATH"

echo "=== Building C++ Box2D Physics Game with Emscripten ==="

mkdir -p dist
mkdir -p public

em++ -O3 -std=c++17 \
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

# Fix any <!doctypehtml> into valid <!DOCTYPE html>
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

echo "=== Build Complete! Output in ./dist/ (index.html, index.js, index.wasm) ==="
