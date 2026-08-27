CXX ?= g++
EMCXX ?= em++
CXXFLAGS = -O3 -std=c++17 -I./cpp/include
SDL_FLAGS = `sdl2-config --cflags --libs`

EMFLAGS = -O3 -std=c++17 -I./cpp/include \
	-s USE_SDL=2 \
	-s WASM=1 \
	-s ALLOW_MEMORY_GROWTH=1 \
	-s NO_EXIT_RUNTIME=1 \
	--shell-file shell_minimal.html

SOURCES = cpp/src/main.cpp

.PHONY: all web native clean

all: web

# Emscripten WebAssembly Build (Outputs to dist/index.html, dist/index.js, dist/index.wasm)
web:
	mkdir -p dist
	$(EMCXX) $(EMFLAGS) $(SOURCES) -o dist/index.html
	cp dist/index.html index.html 2>/dev/null || true
	mkdir -p public && cp -r dist/* public/ 2>/dev/null || true
	@echo "Build successful! Open dist/index.html or deploy to GitHub Pages."

# Native Desktop Executable Build (Requires libsdl2-dev)
native:
	$(CXX) $(CXXFLAGS) $(SOURCES) $(SDL_FLAGS) -o physics_game
	@echo "Native desktop build successful! Run ./physics_game"

clean:
	rm -rf dist physics_game *.wasm *.js
