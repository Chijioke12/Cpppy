# C++ 2D Physics Engine & Game (Emscripten / WebAssembly)

A 2D rigid-body physics engine and game written in **modern C++17** and compiled directly to **WebAssembly** via the **Emscripten** SDK, using SDL2 for hardware-accelerated rendering, particle effects, and interactive input handling.

---

## 🚀 Features

- **Physics Engine Core (C++17)**:
  - Sequential impulse solver with sub-stepping for penetration resolution.
  - SAT (Separating Axis Theorem) convex polygon collision detection and circle manifolds.
  - Rotational inertia, Coulomb friction, restitution, and mass dynamics.
  - Multi-joint constraint solver: Distance springs, structural rods, suspension ropes, and stress-analyzed weld joints (with heat-map color coding and tension snapping).
  - Radial gravity wells, orbital mechanics, accretion disks, and black holes.
  - Dynamic particle emitter for sparks, fiery shockwaves, smoke puffs, and debris.

- **Interactive Levels**:
  1. **Level 1: Castle Siege** – Demolition physics, stone pillars, king targets, and explosive TNT crates.
  2. **Level 2: Bridge Load Test** – Truss bridge stress analysis with a heavy 6-wheel articulated truck.
  3. **Level 3: Orbital Slingshot** – Zero-G orbital mechanics, planetary gravity wells, and black hole slingshots.
  4. **Level 4: Ragdoll Pinball** – Multi-joint skeletal ragdoll physics with bumpers and bouncers.
  5. **Level 5: Physics Sandbox Lab** – Newton's cradle, domino chains, wrecking ball, and interactive entity spawner.

- **In-Engine C++ Tools & HUD**:
  - **Slingshot**: Parabolic trajectory prediction arc with 4 ammo types (Boulder, TNT, Rubber Ball, Cluster).
  - **Grab & Fling**: Real-time cursor joint for picking up and flinging rigid bodies.
  - **Entity Spawner**: Dynamic spawning of crates, stone columns, glass panes, rubber spheres, TNT, ragdolls, and vehicles.
  - **Explosion Tool**: Mouse-click shockwave detonations.
  - **Telemetry HUD**: Real-time FPS, entity count, active constraints, target counter, gravity toggles, and slow-motion.

---

## 🤖 GitHub Actions & GitHub Pages Deployment

The repository includes a ready-to-run GitHub Actions workflow (`.github/workflows/deploy.yml`) that automatically compiles the C++ codebase to WebAssembly and deploys it to **GitHub Pages** on every push to `main` or `master`.

### How to Enable GitHub Pages:
1. Push the code to your GitHub repository.
2. Navigate to **Settings** > **Pages** in your repository.
3. Under **Build and deployment** > **Source**, select **GitHub Actions**.
4. The workflow will automatically build the C++ project with the Emscripten SDK and deploy the interactive game to `https://<username>.github.io/<repo-name>/`.

---

## 🛠️ Building Locally

### 1. Build with Emscripten (WebAssembly)

Make sure the Emscripten SDK (`emsdk`) is installed and active in your shell:

```bash
# Using the build script:
./build.sh

# Or using Make:
make web

# Or using CMake:
mkdir build && cd build
emcmake cmake ..
emmake make
```

The compiled output will be generated in `dist/`:
- `index.html` (Custom Emscripten shell)
- `index.js` (JavaScript glue code)
- `index.wasm` (Compiled WebAssembly binary)

### 2. Build Natively (Desktop Linux / macOS / Windows)

You can also compile and run the engine natively as a standard C++ desktop application using your system's C++ compiler and SDL2:

```bash
# Install SDL2 development libraries:
# Ubuntu/Debian: sudo apt-get install libsdl2-dev
# macOS: brew install sdl2

# Compile native binary:
make native

# Run the game:
./physics_game
```

---

## 🎮 Controls & Hotkeys

| Key / Action | Description |
| :--- | :--- |
| `1` – `5` | Switch between Levels 1 to 5 |
| `R` | Reset current level |
| `T` | Toggle Slow-Motion (0.25x / 1.0x) |
| `G` | Cycle Gravity (Earth 9.8m/s² &bull; Zero-G &bull; Moon 1.6m/s²) |
| `TAB` | Cycle Active Tool (Slingshot &bull; Drag &bull; Spawn &bull; Explode) |
| `Space` | Detonate first active TNT block |
| **Left Click + Drag** | Aim Slingshot / Drag Rigid Body / Spawn Entities |
| **Fullscreen Button** | Toggle full browser canvas expansion |

---

## 📁 Project Structure

```
├── .github/
│   └── workflows/
│       └── deploy.yml        # CI/CD pipeline for Emscripten & GitHub Pages
├── cpp/
│   ├── include/
│   │   ├── Vector2.h         # 2D Vector math & geometric operations
│   │   ├── RigidBody.h       # Rigid body dynamics, shapes, materials, inertia
│   │   ├── Collision.h       # SAT polygon collision & circle manifolds
│   │   ├── Constraint.h      # Springs, rods, ropes, and weld joints
│   │   ├── ParticleSystem.h  # Sparks, smoke, and fiery explosions
│   │   ├── PhysicsWorld.h    # World solver, sub-stepping, gravity wells
│   │   ├── GameLevels.h      # Level setups & procedural ragdolls/vehicles
│   │   └── GameRenderer.h    # SDL2 rasterization & procedural font engine
│   └── src/
│       └── main.cpp          # Game loop, input handling, and SDL2 entry point
├── build.sh                  # Emscripten compilation shell script
├── CMakeLists.txt            # Cross-platform CMake configuration
├── Makefile                  # Simple build targets (make web / make native)
├── shell_minimal.html        # Clean, responsive Emscripten HTML template
└── package.json              # Workspace configuration
```
