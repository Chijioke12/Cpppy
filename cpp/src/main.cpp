#include <SDL2/SDL.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "../include/Vector2.h"
#include "../include/RigidBody.h"
#include "../include/Constraint.h"
#include "../include/Collision.h"
#include "../include/ParticleSystem.h"
#include "../include/PhysicsWorld.h"
#include "../include/GameLevels.h"
#include "../include/GameRenderer.h"

// -------------------------------------------------------------
// Global Game State
// -------------------------------------------------------------
static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;

static const int SCREEN_WIDTH = 1060;
static const int SCREEN_HEIGHT = 680;

static PhysicsWorld g_world;
static int g_currentLevel = 1;
static bool g_isRunning = true;

// Gameplay Stats
static int g_score = 0;
static int g_targetsTotal = 0;
static int g_targetsDestroyed = 0;
static int g_shotsFired = 0;
static bool g_levelComplete = false;

// Tool & Interaction Modes
enum InteractionTool {
    TOOL_SLINGSHOT = 0,
    TOOL_DRAG = 1,
    TOOL_SPAWN = 2,
    TOOL_EXPLODE = 3
};
static InteractionTool g_currentTool = TOOL_SLINGSHOT;

enum AmmoType {
    AMMO_BOULDER = 0,
    AMMO_TNT = 1,
    AMMO_RUBBER = 2,
    AMMO_CLUSTER = 3
};
static AmmoType g_selectedAmmo = AMMO_BOULDER;

enum SpawnItem {
    SPAWN_WOOD_BOX = 0,
    SPAWN_STONE_COL = 1,
    SPAWN_GLASS_PANE = 2,
    SPAWN_RUBBER_BALL = 3,
    SPAWN_TNT = 4,
    SPAWN_RAGDOLL = 5,
    SPAWN_TRUCK = 6
};
static SpawnItem g_selectedSpawnItem = SPAWN_WOOD_BOX;

// Slingshot State
static bool g_isAiming = false;
static Vector2 g_aimStart(160, 560);
static Vector2 g_aimCurrent(160, 560);

// FPS calculation
static uint32_t g_lastTime = 0;
static int g_fps = 60;
static int g_frameCount = 0;
static uint32_t g_fpsTimer = 0;

// -------------------------------------------------------------
// Level Management
// -------------------------------------------------------------
static void countTargets() {
    g_targetsTotal = 0;
    g_targetsDestroyed = 0;
    for (const auto& b : g_world.bodies) {
        if (b->isTarget) {
            g_targetsTotal++;
        }
    }
}

static void loadLevel(int levelId) {
    g_currentLevel = levelId;
    g_levelComplete = false;
    g_isAiming = false;

    if (levelId == 1) {
        GameLevels::buildLevel1(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
        g_world.gravity = Vector2(0, 980.0f);
        g_aimStart = Vector2(160, 560);
    } else if (levelId == 2) {
        GameLevels::buildLevel2(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
        g_world.gravity = Vector2(0, 980.0f);
        g_aimStart = Vector2(100, 480);
    } else if (levelId == 3) {
        GameLevels::buildLevel3(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
        g_world.gravity = Vector2(0, 0.0f); // Zero-G
        g_aimStart = Vector2(120, (float)SCREEN_HEIGHT * 0.5f);
    } else if (levelId == 4) {
        GameLevels::buildLevel4(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
        g_world.gravity = Vector2(0, 980.0f);
        g_aimStart = Vector2((float)SCREEN_WIDTH * 0.5f, 600);
    } else {
        GameLevels::buildLevel5(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
        g_world.gravity = Vector2(0, 980.0f);
        g_aimStart = Vector2(160, 560);
    }

    countTargets();
}

// -------------------------------------------------------------
// Slingshot Projectile Launcher
// -------------------------------------------------------------
static void fireSlingshot(Vector2 start, Vector2 target, AmmoType ammo) {
    Vector2 dir = start - target;
    float power = dir.length() * 5.0f;
    if (power < 15.0f) return;
    if (power > 3200.0f) power = 3200.0f;

    Vector2 normDir = dir.normalized();
    Vector2 launchVel = normDir * power;
    g_shotsFired++;

    if (ammo == AMMO_BOULDER) {
        auto boulder = g_world.createCircle(BODY_DYNAMIC, MAT_STONE, start, 24.0f);
        boulder->velocity = launchVel;
        boulder->mass = 28.0f;
        boulder->invMass = 1.0f / 28.0f;
        boulder->color = 0x607D8B;
    } else if (ammo == AMMO_TNT) {
        auto bomb = g_world.createCircle(BODY_DYNAMIC, MAT_TNT, start, 22.0f);
        bomb->velocity = launchVel;
        bomb->color = 0xE53935;
    } else if (ammo == AMMO_RUBBER) {
        auto rubber = g_world.createCircle(BODY_DYNAMIC, MAT_RUBBER, start, 20.0f);
        rubber->velocity = launchVel;
        rubber->restitution = 0.95f;
        rubber->color = 0xFF7043;
    } else if (ammo == AMMO_CLUSTER) {
        for (int i = -1; i <= 1; ++i) {
            Vector2 spreadVel = launchVel.rotate((float)i * 0.14f);
            auto pellet = g_world.createCircle(BODY_DYNAMIC, MAT_METAL, start + Vector2((float)i * 10.0f, 0.0f), 14.0f);
            pellet->velocity = spreadVel;
            pellet->mass = 12.0f;
            pellet->invMass = 1.0f / 12.0f;
            pellet->color = 0xFFCA28;
        }
    }
}

// -------------------------------------------------------------
// Mouse & Input Handling
// -------------------------------------------------------------
struct RectButton {
    int id;
    int x, y, w, h;
    std::string text;
    bool isSelected;
};

static std::vector<RectButton> g_levelButtons;
static std::vector<RectButton> g_toolButtons;
static std::vector<RectButton> g_ammoButtons;
static std::vector<RectButton> g_spawnButtons;

static void initUIButtons() {
    g_levelButtons.clear();
    g_levelButtons.push_back({1, 10, 10, 100, 28, "1: Castle", true});
    g_levelButtons.push_back({2, 115, 10, 100, 28, "2: Bridge", false});
    g_levelButtons.push_back({3, 220, 10, 100, 28, "3: Orbit", false});
    g_levelButtons.push_back({4, 325, 10, 100, 28, "4: Ragdoll", false});
    g_levelButtons.push_back({5, 430, 10, 100, 28, "5: Sandbox", false});
    g_levelButtons.push_back({99, 535, 10, 70, 28, "Reset", false});

    g_toolButtons.clear();
    g_toolButtons.push_back({0, 10, 44, 90, 24, "Slingshot", true});
    g_toolButtons.push_back({1, 105, 44, 75, 24, "Grab/Drag", false});
    g_toolButtons.push_back({2, 185, 44, 60, 24, "Spawn", false});
    g_toolButtons.push_back({3, 250, 44, 65, 24, "Explode", false});

    g_ammoButtons.clear();
    g_ammoButtons.push_back({0, 330, 44, 60, 24, "Boulder", true});
    g_ammoButtons.push_back({1, 395, 44, 50, 24, "TNT", false});
    g_ammoButtons.push_back({2, 450, 44, 60, 24, "Rubber", false});
    g_ammoButtons.push_back({3, 515, 44, 60, 24, "Cluster", false});

    g_spawnButtons.clear();
    g_spawnButtons.push_back({0, 330, 44, 55, 24, "Box", true});
    g_spawnButtons.push_back({1, 390, 44, 55, 24, "Stone", false});
    g_spawnButtons.push_back({2, 450, 44, 55, 24, "Glass", false});
    g_spawnButtons.push_back({3, 510, 44, 55, 24, "Ball", false});
    g_spawnButtons.push_back({4, 570, 44, 50, 24, "TNT", false});
    g_spawnButtons.push_back({5, 625, 44, 65, 24, "Ragdoll", false});
    g_spawnButtons.push_back({6, 695, 44, 55, 24, "Truck", false});
}

static bool checkButtonClick(const std::vector<RectButton>& buttons, int mx, int my, int& outId) {
    for (const auto& btn : buttons) {
        if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
            outId = btn.id;
            return true;
        }
    }
    return false;
}

static void handleMouseDown(int mx, int my, int button) {
    int clickedId = -1;

    // Check Level Buttons
    if (checkButtonClick(g_levelButtons, mx, my, clickedId)) {
        if (clickedId == 99) {
            loadLevel(g_currentLevel);
        } else {
            loadLevel(clickedId);
            for (auto& btn : g_levelButtons) {
                btn.isSelected = (btn.id == clickedId);
            }
        }
        return;
    }

    // Check Tool Buttons
    if (checkButtonClick(g_toolButtons, mx, my, clickedId)) {
        g_currentTool = (InteractionTool)clickedId;
        for (auto& btn : g_toolButtons) {
            btn.isSelected = (btn.id == clickedId);
        }
        return;
    }

    // Check Ammo Buttons (when in slingshot mode)
    if (g_currentTool == TOOL_SLINGSHOT) {
        if (checkButtonClick(g_ammoButtons, mx, my, clickedId)) {
            g_selectedAmmo = (AmmoType)clickedId;
            for (auto& btn : g_ammoButtons) {
                btn.isSelected = (btn.id == clickedId);
            }
            return;
        }
    }

    // Check Spawn Buttons (when in spawn mode)
    if (g_currentTool == TOOL_SPAWN) {
        if (checkButtonClick(g_spawnButtons, mx, my, clickedId)) {
            g_selectedSpawnItem = (SpawnItem)clickedId;
            for (auto& btn : g_spawnButtons) {
                btn.isSelected = (btn.id == clickedId);
            }
            return;
        }
    }

    // Ignore clicks in UI header bar
    if (my < 75) return;

    Vector2 mousePos((float)mx, (float)my);

    if (g_currentTool == TOOL_SLINGSHOT) {
        g_isAiming = true;
        g_aimCurrent = mousePos;
    } else if (g_currentTool == TOOL_DRAG) {
        g_world.startDrag((float)mx, (float)my);
    } else if (g_currentTool == TOOL_EXPLODE) {
        g_world.applyExplosion(mousePos, 200.0f, 2400.0f, 200.0f);
    } else if (g_currentTool == TOOL_SPAWN) {
        if (g_selectedSpawnItem == SPAWN_WOOD_BOX) {
            g_world.createBox(BODY_DYNAMIC, MAT_WOOD, mousePos, 40.0f, 40.0f);
        } else if (g_selectedSpawnItem == SPAWN_STONE_COL) {
            g_world.createBox(BODY_DYNAMIC, MAT_STONE, mousePos, 35.0f, 75.0f);
        } else if (g_selectedSpawnItem == SPAWN_GLASS_PANE) {
            g_world.createBox(BODY_DYNAMIC, MAT_GLASS, mousePos, 20.0f, 80.0f);
        } else if (g_selectedSpawnItem == SPAWN_RUBBER_BALL) {
            g_world.createCircle(BODY_DYNAMIC, MAT_RUBBER, mousePos, 20.0f);
        } else if (g_selectedSpawnItem == SPAWN_TNT) {
            g_world.createBox(BODY_DYNAMIC, MAT_TNT, mousePos, 38.0f, 38.0f);
        } else if (g_selectedSpawnItem == SPAWN_RAGDOLL) {
            GameLevels::spawnRagdoll(g_world, mousePos.x, mousePos.y, 0.9f);
        } else if (g_selectedSpawnItem == SPAWN_TRUCK) {
            GameLevels::spawnVehicle(g_world, mousePos.x, mousePos.y, 10.0f);
        }
    }
}

static void handleMouseMove(int mx, int my) {
    Vector2 mousePos((float)mx, (float)my);
    if (g_isAiming) {
        g_aimCurrent = mousePos;
    }
    if (g_world.isDragging) {
        g_world.updateDrag((float)mx, (float)my);
    }
}

static void handleMouseUp(int mx, int my, int button) {
    if (g_isAiming && g_currentTool == TOOL_SLINGSHOT) {
        g_isAiming = false;
        Vector2 mousePos((float)mx, (float)my);
        fireSlingshot(g_aimStart, mousePos, g_selectedAmmo);
    }
    if (g_world.isDragging) {
        g_world.endDrag();
    }
}

static void handleKeyDown(SDL_Keycode key) {
    if (key >= SDLK_1 && key <= SDLK_5) {
        int lvl = key - SDLK_1 + 1;
        loadLevel(lvl);
        for (auto& btn : g_levelButtons) {
            btn.isSelected = (btn.id == lvl);
        }
    } else if (key == SDLK_r) {
        loadLevel(g_currentLevel);
    } else if (key == SDLK_t) {
        // Toggle slow motion
        g_world.timeScale = (g_world.timeScale == 1.0f) ? 0.25f : 1.0f;
    } else if (key == SDLK_g) {
        // Cycle gravity
        if (g_world.gravity.y == 980.0f) g_world.gravity.y = 0.0f;
        else if (g_world.gravity.y == 0.0f) g_world.gravity.y = 160.0f;
        else g_world.gravity.y = 980.0f;
    } else if (key == SDLK_TAB) {
        int nextTool = (g_currentTool + 1) % 4;
        g_currentTool = (InteractionTool)nextTool;
        for (auto& btn : g_toolButtons) {
            btn.isSelected = (btn.id == nextTool);
        }
    } else if (key == SDLK_SPACE) {
        // Detonate first TNT
        for (auto& b : g_world.bodies) {
            if (b->material == MAT_TNT && !b->isDead) {
                b->takeDamage(1000.0f);
                break;
            }
        }
    }
}

// -------------------------------------------------------------
// Rendering Loop
// -------------------------------------------------------------
static void renderFrame() {
    // 1. Clear background
    if (g_currentLevel == 3) {
        SDL_SetRenderDrawColor(g_renderer, 8, 10, 18, 255); // Deep space
    } else {
        SDL_SetRenderDrawColor(g_renderer, 15, 23, 42, 255); // Slate 900
    }
    SDL_RenderClear(g_renderer);

    // 2. Subtle grid lines
    SDL_SetRenderDrawColor(g_renderer, 30, 41, 59, 255);
    const int gridSize = 40;
    for (int x = 0; x < SCREEN_WIDTH; x += gridSize) {
        SDL_RenderDrawLine(g_renderer, x, 75, x, SCREEN_HEIGHT);
    }
    for (int y = 80; y < SCREEN_HEIGHT; y += gridSize) {
        SDL_RenderDrawLine(g_renderer, 0, y, SCREEN_WIDTH, y);
    }

    // 3. Gravity Wells / Orbits (Level 3)
    for (const auto& gw : g_world.gravityWells) {
        SDL_Color auraColor = gw.isBlackHole ? SDL_Color{168, 85, 247, 80} : SDL_Color{56, 189, 248, 80};
        GameRenderer::drawCircleOutline(g_renderer, (int)gw.position.x, (int)gw.position.y, (int)gw.radius, auraColor);
        GameRenderer::drawCircleOutline(g_renderer, (int)gw.position.x, (int)gw.position.y, (int)(gw.radius * 0.5f), auraColor);
    }

    // 4. Constraints / Springs
    for (const auto& c : g_world.constraints) {
        if (c->isBroken) continue;
        Vector2 pA = c->getWorldAnchorA();
        Vector2 pB = c->getWorldAnchorB();

        // Stress heat color: Green -> Yellow -> Red
        SDL_Color lineColor = {148, 163, 184, 255};
        if (c->currentStress > 0.05f) {
            uint8_t r = (uint8_t)std::min(255.0f, c->currentStress * 255.0f);
            uint8_t g = (uint8_t)std::max(0.0f, (1.0f - c->currentStress) * 220.0f);
            lineColor = {r, g, 80, 255};
        }

        SDL_SetRenderDrawColor(g_renderer, lineColor.r, lineColor.g, lineColor.b, 255);
        SDL_RenderDrawLine(g_renderer, (int)pA.x, (int)pA.y, (int)pB.x, (int)pB.y);

        // Joints
        GameRenderer::drawFilledCircle(g_renderer, (int)pA.x, (int)pA.y, 3, {203, 213, 225, 255});
        GameRenderer::drawFilledCircle(g_renderer, (int)pB.x, (int)pB.y, 3, {203, 213, 225, 255});
    }

    // 5. Mouse Drag Line
    if (g_world.isDragging && g_world.draggedBodyId != -1) {
        for (const auto& b : g_world.bodies) {
            if (b->id == g_world.draggedBodyId) {
                Vector2 anchor = b->position + g_world.dragLocalAnchor.rotate(b->angle);
                SDL_SetRenderDrawColor(g_renderer, 6, 182, 212, 255); // Cyan
                SDL_RenderDrawLine(g_renderer, (int)anchor.x, (int)anchor.y, (int)g_world.dragTargetPos.x, (int)g_world.dragTargetPos.y);
                GameRenderer::drawFilledCircle(g_renderer, (int)anchor.x, (int)anchor.y, 4, {6, 182, 212, 255});
                break;
            }
        }
    }

    // 6. Rigid Bodies
    for (const auto& b : g_world.bodies) {
        if (b->isDead) continue;
        SDL_Color bodyColor = GameRenderer::hexToSDL(b->color);
        SDL_Color outlineColor = {255, 255, 255, 120};

        if (b->shapeType == SHAPE_CIRCLE) {
            GameRenderer::drawFilledCircle(g_renderer, (int)b->position.x, (int)b->position.y, (int)b->radius, bodyColor);
            GameRenderer::drawCircleOutline(g_renderer, (int)b->position.x, (int)b->position.y, (int)b->radius, outlineColor);

            // Orientation line
            Vector2 orient = b->position + Vector2(std::cos(b->angle), std::sin(b->angle)) * (b->radius * 0.8f);
            SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 150);
            SDL_RenderDrawLine(g_renderer, (int)b->position.x, (int)b->position.y, (int)orient.x, (int)orient.y);

            // Target Bulls-eye pattern
            if (b->isTarget) {
                GameRenderer::drawFilledCircle(g_renderer, (int)b->position.x, (int)b->position.y, (int)(b->radius * 0.6f), {255, 255, 255, 255});
                GameRenderer::drawFilledCircle(g_renderer, (int)b->position.x, (int)b->position.y, (int)(b->radius * 0.3f), {239, 68, 68, 255});
            }
        } else {
            // Polygons & Boxes
            GameRenderer::drawPolygon(g_renderer, b->worldVertices, bodyColor, outlineColor);

            // TNT Box Label
            if (b->material == MAT_TNT) {
                GameRenderer::drawText(g_renderer, "TNT", (int)b->position.x - 9, (int)b->position.y - 4, 1, {255, 255, 255, 255});
            }
        }
    }

    // 7. Particles
    for (const auto& p : g_world.particleSystem.particles) {
        if (p.life <= 0) continue;
        SDL_Color pColor = GameRenderer::hexToSDL(p.color);
        SDL_SetRenderDrawColor(g_renderer, pColor.r, pColor.g, pColor.b, 255);
        SDL_Rect pRect = { (int)(p.position.x - p.size * 0.5f), (int)(p.position.y - p.size * 0.5f), (int)p.size, (int)p.size };
        SDL_RenderFillRect(g_renderer, &pRect);
    }

    // 8. Slingshot Rubber Band & Trajectory Arc
    if (g_isAiming && g_currentTool == TOOL_SLINGSHOT) {
        // Elastic bands
        SDL_SetRenderDrawColor(g_renderer, 249, 115, 22, 255); // Orange
        SDL_RenderDrawLine(g_renderer, (int)g_aimStart.x - 12, (int)g_aimStart.y - 15, (int)g_aimCurrent.x, (int)g_aimCurrent.y);
        SDL_RenderDrawLine(g_renderer, (int)g_aimStart.x + 12, (int)g_aimStart.y - 15, (int)g_aimCurrent.x, (int)g_aimCurrent.y);

        // Loaded Projectile
        SDL_Color ammoColors[4] = {{96, 125, 139, 255}, {229, 57, 53, 255}, {255, 112, 67, 255}, {255, 202, 40, 255}};
        GameRenderer::drawFilledCircle(g_renderer, (int)g_aimCurrent.x, (int)g_aimCurrent.y, 14, ammoColors[g_selectedAmmo]);
        GameRenderer::drawCircleOutline(g_renderer, (int)g_aimCurrent.x, (int)g_aimCurrent.y, 14, {255, 255, 255, 255});

        // Parabolic trajectory preview
        Vector2 launchVec = (g_aimStart - g_aimCurrent) * 5.0f;
        float gravY = (g_currentLevel == 3) ? 0.0f : g_world.gravity.y;

        for (int i = 1; i <= 24; ++i) {
            float t = (float)i * 0.045f;
            float px = g_aimStart.x + launchVec.x * t;
            float py = g_aimStart.y + launchVec.y * t + 0.5f * gravY * t * t;
            if (px > SCREEN_WIDTH || py > SCREEN_HEIGHT || px < 0) break;
            GameRenderer::drawFilledCircle(g_renderer, (int)px, (int)py, std::max(2, 5 - i / 5), {249, 115, 22, 180});
        }
    }

    // ---------------------------------------------------------
    // 9. In-Game GUI Header & Toolbar
    // ---------------------------------------------------------
    // Top Bar Background
    SDL_SetRenderDrawColor(g_renderer, 15, 23, 42, 255);
    SDL_Rect headerRect = {0, 0, SCREEN_WIDTH, 72};
    SDL_RenderFillRect(g_renderer, &headerRect);
    SDL_SetRenderDrawColor(g_renderer, 51, 65, 85, 255);
    SDL_RenderDrawLine(g_renderer, 0, 72, SCREEN_WIDTH, 72);

    // Draw Level Buttons
    for (const auto& btn : g_levelButtons) {
        SDL_Rect r = { btn.x, btn.y, btn.w, btn.h };
        if (btn.isSelected) {
            SDL_SetRenderDrawColor(g_renderer, 37, 99, 235, 255); // Blue 600
        } else {
            SDL_SetRenderDrawColor(g_renderer, 30, 41, 59, 255); // Slate 800
        }
        SDL_RenderFillRect(g_renderer, &r);
        SDL_SetRenderDrawColor(g_renderer, 71, 85, 105, 255);
        SDL_RenderDrawRect(g_renderer, &r);

        GameRenderer::drawText(g_renderer, btn.text, btn.x + 8, btn.y + 10, 1, {255, 255, 255, 255});
    }

    // Draw Tool Buttons
    for (const auto& btn : g_toolButtons) {
        SDL_Rect r = { btn.x, btn.y, btn.w, btn.h };
        if (btn.isSelected) {
            SDL_SetRenderDrawColor(g_renderer, 234, 88, 12, 255); // Orange 600
        } else {
            SDL_SetRenderDrawColor(g_renderer, 30, 41, 59, 255);
        }
        SDL_RenderFillRect(g_renderer, &r);
        SDL_SetRenderDrawColor(g_renderer, 71, 85, 105, 255);
        SDL_RenderDrawRect(g_renderer, &r);

        GameRenderer::drawText(g_renderer, btn.text, btn.x + 6, btn.y + 8, 1, {255, 255, 255, 255});
    }

    // Draw Ammo Buttons (if slingshot)
    if (g_currentTool == TOOL_SLINGSHOT) {
        GameRenderer::drawText(g_renderer, "Ammo:", 300, 52, 1, {148, 163, 184, 255});
        for (const auto& btn : g_ammoButtons) {
            SDL_Rect r = { btn.x, btn.y, btn.w, btn.h };
            if (btn.isSelected) {
                SDL_SetRenderDrawColor(g_renderer, 13, 148, 136, 255); // Teal 600
            } else {
                SDL_SetRenderDrawColor(g_renderer, 30, 41, 59, 255);
            }
            SDL_RenderFillRect(g_renderer, &r);
            SDL_SetRenderDrawColor(g_renderer, 71, 85, 105, 255);
            SDL_RenderDrawRect(g_renderer, &r);

            GameRenderer::drawText(g_renderer, btn.text, btn.x + 6, btn.y + 8, 1, {255, 255, 255, 255});
        }
    } else if (g_currentTool == TOOL_SPAWN) {
        GameRenderer::drawText(g_renderer, "Item:", 300, 52, 1, {148, 163, 184, 255});
        for (const auto& btn : g_spawnButtons) {
            SDL_Rect r = { btn.x, btn.y, btn.w, btn.h };
            if (btn.isSelected) {
                SDL_SetRenderDrawColor(g_renderer, 5, 150, 105, 255); // Emerald 600
            } else {
                SDL_SetRenderDrawColor(g_renderer, 30, 41, 59, 255);
            }
            SDL_RenderFillRect(g_renderer, &r);
            SDL_SetRenderDrawColor(g_renderer, 71, 85, 105, 255);
            SDL_RenderDrawRect(g_renderer, &r);

            GameRenderer::drawText(g_renderer, btn.text, btn.x + 5, btn.y + 8, 1, {255, 255, 255, 255});
        }
    }

    // Telemetry & Stats (Right side of header)
    std::stringstream ssStats;
    ssStats << "FPS:" << g_fps << " | Bodies:" << g_world.bodies.size() << " | Joints:" << g_world.constraints.size()
            << " | Targets:" << g_targetsDestroyed << "/" << g_targetsTotal;
    GameRenderer::drawText(g_renderer, ssStats.str(), 620, 18, 1, {56, 189, 248, 255});

    std::stringstream ssKeys;
    ssKeys << "Hotkeys: [1-5] Levels | [R] Reset | [T] SlowMo | [G] Gravity | [TAB] Tool";
    GameRenderer::drawText(g_renderer, ssKeys.str(), 620, 38, 1, {148, 163, 184, 255});

    // Level Clear Banner
    if (g_levelComplete) {
        SDL_Rect bannerRect = { SCREEN_WIDTH / 2 - 160, 100, 320, 48 };
        SDL_SetRenderDrawColor(g_renderer, 22, 101, 52, 230); // Green 800
        SDL_RenderFillRect(g_renderer, &bannerRect);
        SDL_SetRenderDrawColor(g_renderer, 74, 222, 128, 255);
        SDL_RenderDrawRect(g_renderer, &bannerRect);

        GameRenderer::drawText(g_renderer, "LEVEL COMPLETED!", SCREEN_WIDTH / 2 - 65, 114, 1, {255, 255, 255, 255});
        GameRenderer::drawText(g_renderer, "Press [1-5] for next level", SCREEN_WIDTH / 2 - 80, 130, 1, {187, 247, 208, 255});
    }

    SDL_RenderPresent(g_renderer);
}

// -------------------------------------------------------------
// Main Game Update Tick
// -------------------------------------------------------------
static void mainLoopStep() {
    uint32_t currentTime = SDL_GetTicks();
    float dt = (currentTime - g_lastTime) / 1000.0f;
    if (dt > 0.04f) dt = 0.04f;
    g_lastTime = currentTime;

    // Track FPS
    g_frameCount++;
    if (currentTime - g_fpsTimer >= 1000) {
        g_fps = g_frameCount;
        g_frameCount = 0;
        g_fpsTimer = currentTime;
    }

    // Process SDL Events
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_isRunning = false;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            handleMouseDown(e.button.x, e.button.y, e.button.button);
        } else if (e.type == SDL_MOUSEMOTION) {
            handleMouseMove(e.motion.x, e.motion.y);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            handleMouseUp(e.button.x, e.button.y, e.button.button);
        } else if (e.type == SDL_KEYDOWN) {
            handleKeyDown(e.key.keysym.sym);
        }
    }

    // Physics Step
    g_world.step(dt);

    // Target tracking
    int remaining = 0;
    for (const auto& b : g_world.bodies) {
        if (b->isTarget && !b->isDead) {
            remaining++;
        }
    }
    g_targetsDestroyed = g_targetsTotal - remaining;
    if (g_targetsTotal > 0 && remaining == 0 && !g_levelComplete) {
        g_levelComplete = true;
        g_score += 1000;
    }

    // Render Frame
    renderFrame();
}

// -------------------------------------------------------------
// Entry Point
// -------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    g_window = SDL_CreateWindow(
        "C++ Physics Engine (Emscripten / WebAssembly)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!g_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    g_renderer = SDL_CreateRenderer(
        g_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g_renderer) {
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }

    initUIButtons();
    loadLevel(1);

    g_lastTime = SDL_GetTicks();
    g_fpsTimer = g_lastTime;

#ifdef __EMSCRIPTEN__
    // 0 = simulate infinite loop via browser requestAnimationFrame
    emscripten_set_main_loop(mainLoopStep, 0, 1);
#else
    // Native Desktop Loop
    while (g_isRunning) {
        mainLoopStep();
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
#endif

    return 0;
}
