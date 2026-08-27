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

### 3. Build for KaiOS 2.5 (Pure asm.js / Gecko 48)

KaiOS 2.5 devices (such as JioPhone, Nokia 8110 4G, Nokia 2720 Flip) run Gecko 48 without WebAssembly support. To build for KaiOS using Emscripten (e.g. `1.38.x` or with `-s WASM=0`):

```bash
# Build pure asm.js and bundle manifest.webapp
./build.sh kaios
```

The output in `dist/` will contain:
- `index.html` (KaiOS-compatible shell)
- `index.js` (Pure `"use asm";` JavaScript engine)
- `index.js.mem` (Memory initialization heap)
- `manifest.webapp` (Packaged app manifest for KaiOS app store / OmniSD)

---

## 🎮 Controls & Hotkeys (Desktop & KaiOS D-Pad)

| Action | Desktop Key | KaiOS Physical Keypad |
| :--- | :--- | :--- |
| **Aim Slingshot** | Mouse Drag | `Up` / `Down` / `Left` / `Right` (D-Pad) |
| **Fire Slingshot** | Release Click | `Enter` / `OK` / `5` (Center Key) |
| **In-Flight Ability** | `Space` / Click | `Enter` / `OK` / `5` |
| **Select Level (1-5)**| `1` – `5` | `1` – `5` Number Keys |
| **Reset Level** | `R` | `Backspace` / `SoftRight` |
| **Next Level** | `N` | `F2` / `SoftRight` |
| **Toggle Slow-Mo** | `T` | `F1` / `SoftLeft` |

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
