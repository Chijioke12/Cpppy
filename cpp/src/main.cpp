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
#include "../include/ParticleSystem.h"
#include "../include/PhysicsWorld.h"
#include "../include/GameLevels.h"
#include "../include/GameRenderer.h"
#include "../include/TextureManager.h"

// -------------------------------------------------------------
// Constants & Game State
// -------------------------------------------------------------
static const int SCREEN_WIDTH = 1060;
static const int SCREEN_HEIGHT = 680;

static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;

static PhysicsWorld g_world;
static int g_currentLevel = 1;
static LevelInfo g_currentLevelInfo;
static bool g_isRunning = true;

// Gameplay & Slingshot Variables
static Vector2 g_slingshotPos(180.0f, 490.0f);
static Vector2 g_leftForkAnchor(168.0f, 458.0f);
static Vector2 g_rightForkAnchor(192.0f, 458.0f);

static bool g_isAiming = false;
static Vector2 g_aimPos(180.0f, 490.0f);
static const float MAX_PULL_DISTANCE = 95.0f;

// Projectile Roster Queue
static std::vector<int> g_ammoQueue;
static int g_activeBirdType = 0;
static std::shared_ptr<RigidBody> g_activeFlightBird = nullptr;
static float g_flightTimer = 0.0f;
static float g_settleTimer = 0.0f;

// Scoring & Stats
static int g_score = 0;
static int g_highScores[6] = { 0, 0, 0, 0, 0, 0 };
static int g_levelStars[6] = { 0, 0, 0, 0, 0, 0 };
static bool g_levelWon = false;
static bool g_levelLost = false;
static float g_victoryAnimTimer = 0.0f;

// Trajectory Line History
static std::vector<Vector2> g_lastFlightPath;

// Performance & Slow-Mo
static uint32_t g_lastTime = 0;
static int g_fps = 60;
static int g_frameCount = 0;
static uint32_t g_fpsTimer = 0;

// -------------------------------------------------------------
// Level Loading & Management
// -------------------------------------------------------------
static void loadLevel(int levelId) {
    g_currentLevel = levelId;
    g_currentLevelInfo = GameLevels::getLevelInfo(levelId);
    g_ammoQueue = g_currentLevelInfo.birdQueue;

    g_levelWon = false;
    g_levelLost = false;
    g_isAiming = false;
    g_activeFlightBird = nullptr;
    g_flightTimer = 0.0f;
    g_settleTimer = 0.0f;
    g_victoryAnimTimer = 0.0f;
    g_lastFlightPath.clear();

    if (!g_ammoQueue.empty()) {
        g_activeBirdType = g_ammoQueue.front();
        g_ammoQueue.erase(g_ammoQueue.begin());
    }

    if (levelId == 1) {
        GameLevels::buildLevel1(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
    } else if (levelId == 2) {
        GameLevels::buildLevel2(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
    } else if (levelId == 3) {
        GameLevels::buildLevel3(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
    } else if (levelId == 4) {
        GameLevels::buildLevel4(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
    } else {
        GameLevels::buildLevel5(g_world, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
    }
}

// -------------------------------------------------------------
// Slingshot Launch & Mid-Air Abilities
// -------------------------------------------------------------
static std::string getBirdTextureKey(int birdType) {
    switch (birdType) {
        case 0: return "projectile_red";
        case 1: return "projectile_bomb";
        case 2: return "projectile_split";
        case 3: return "projectile_drill";
        case 4: return "projectile_rubber";
        default: return "projectile_red";
    }
}

static void launchBird() {
    Vector2 pullOffset = g_slingshotPos - g_aimPos;
    float pullDist = pullOffset.length();
    if (pullDist < 12.0f) {
        g_isAiming = false;
        return;
    }

    float powerMultiplier = 14.5f;
    Vector2 launchVelocity = pullOffset * powerMultiplier;

    // Create active flight bird
    auto bird = g_world.createCircle(BODY_DYNAMIC, MAT_PROJECTILE, g_slingshotPos, 22.0f);
    bird->projectileType = g_activeBirdType;
    bird->textureKey = getBirdTextureKey(g_activeBirdType);
    bird->isArmed = true;
    bird->setVelocity(launchVelocity, PhysicsWorld::PPM);

    if (g_activeBirdType == 0) { // Red Striker
        bird->mass = 4.5f;
        bird->invMass = 1.0f / 4.5f;
    } else if (g_activeBirdType == 1) { // Bombardier
        bird->mass = 5.5f;
        bird->invMass = 1.0f / 5.5f;
    } else if (g_activeBirdType == 2) { // Tri-Splitter
        bird->mass = 2.8f;
        bird->invMass = 1.0f / 2.8f;
    } else if (g_activeBirdType == 3) { // Iron Drill
        bird->mass = 7.0f;
        bird->invMass = 1.0f / 7.0f;
    } else if (g_activeBirdType == 4) { // Rubber
        bird->mass = 3.5f;
        bird->restitution = 0.92f;
    }

    g_activeFlightBird = bird;
    g_flightTimer = 0.0f;
    g_settleTimer = 0.0f;
    g_isAiming = false;
    g_lastFlightPath.clear();
    g_lastFlightPath.push_back(g_slingshotPos);

    // Muzzle dust/smoke puff
    g_world.particleSystem.emitDebris(g_slingshotPos, 0x8D6E63, 10);
}

static void triggerBirdAbility() {
    if (!g_activeFlightBird || g_activeFlightBird->isDead || g_activeFlightBird->abilityUsed) return;
    g_activeFlightBird->abilityUsed = true;

    int type = g_activeFlightBird->projectileType;
    Vector2 pos = g_activeFlightBird->position;
    Vector2 vel = g_activeFlightBird->velocity;

    if (type == 0) {
        // Red Striker: Kinetic forward battle cry boost
        g_activeFlightBird->setVelocity(vel.normalized() * (vel.length() + 350.0f), PhysicsWorld::PPM);
        g_world.particleSystem.emitExplosion(pos, 15);
        g_world.addScorePopup(pos, "WAR CRY!", 0xFF5252, 1.8f);
    } else if (type == 1) {
        // Bombardier: Instant detonating shockwave!
        g_world.applyExplosion(pos, 250.0f, 3800.0f, 380.0f);
        g_activeFlightBird->isDead = true;
    } else if (type == 2) {
        // Tri-Splitter: Splits into 3 swift projectiles
        g_world.addScorePopup(pos, "TRIPLE SPLIT!", 0x0288D1, 1.8f);
        g_world.particleSystem.emitExplosion(pos, 15);

        float currentSpeed = std::max(vel.length(), 400.0f);
        float baseAngle = std::atan2(vel.y, vel.x);

        // Projectile 1 (High +15 deg)
        auto b1 = g_world.createCircle(BODY_DYNAMIC, MAT_PROJECTILE, pos + Vector2(0, -10), 16.0f);
        b1->projectileType = 2;
        b1->textureKey = "projectile_split";
        Vector2 vel1 = Vector2(std::cos(baseAngle - 0.26f), std::sin(baseAngle - 0.26f)) * currentSpeed;
        b1->setVelocity(vel1, PhysicsWorld::PPM);
        b1->abilityUsed = true;

        // Projectile 2 (Low -15 deg)
        auto b2 = g_world.createCircle(BODY_DYNAMIC, MAT_PROJECTILE, pos + Vector2(0, 10), 16.0f);
        b2->projectileType = 2;
        b2->textureKey = "projectile_split";
        Vector2 vel2 = Vector2(std::cos(baseAngle + 0.26f), std::sin(baseAngle + 0.26f)) * currentSpeed;
        b2->setVelocity(vel2, PhysicsWorld::PPM);
        b2->abilityUsed = true;

        // Main projectile stays center
        g_activeFlightBird->radius = 16.0f;
    } else if (type == 3) {
        // Iron Drill: Supersonic thruster boost
        g_activeFlightBird->setVelocity(vel.normalized() * (vel.length() * 2.2f + 500.0f), PhysicsWorld::PPM);
        g_world.addScorePopup(pos, "DRILL HYPER DRIVE!", 0x00E676, 2.0f);
        g_world.particleSystem.emitExplosion(pos, 25);
    } else if (type == 4) {
        // Rubber Ricochet: Mega slam bounce
        g_activeFlightBird->setVelocity(Vector2(vel.x * 1.3f, 600.0f), PhysicsWorld::PPM);
        g_world.addScorePopup(pos, "SUPER BOUNCE!", 0xFF4081, 1.8f);
        g_world.particleSystem.emitExplosion(pos, 15);
    }
}

// -------------------------------------------------------------
// Trajectory Arc Prediction
// -------------------------------------------------------------
static void renderTrajectoryPrediction() {
    if (!g_isAiming) return;

    Vector2 pullOffset = g_slingshotPos - g_aimPos;
    float pullDist = pullOffset.length();
    if (pullDist < 10.0f) return;

    Vector2 simPos = g_slingshotPos;
    Vector2 simVel = pullOffset * 14.5f;
    Vector2 gravity = g_world.gravity;
    float simDt = 0.035f;

    for (int step = 0; step < 26; ++step) {
        simVel += gravity * simDt;
        simPos += simVel * simDt;

        if (simPos.y > (float)SCREEN_HEIGHT - 35.0f || simPos.x > (float)SCREEN_WIDTH) break;

        // Draw glowing gradient trajectory dot
        float dotRadius = 4.5f - (step * 0.12f);
        if (dotRadius < 1.8f) dotRadius = 1.8f;

        SDL_Color dotCol = { 255, 235, 59, (Uint8)(255 - step * 8) };
        if (step % 2 == 0) {
            dotCol = { 255, 112, 67, (Uint8)(255 - step * 8) };
        }
        GameRenderer::drawFilledCircle(g_renderer, (int)simPos.x, (int)simPos.y, (int)dotRadius, dotCol);
    }
}

// -------------------------------------------------------------
// Slingshot Render (Front/Back Forks, Rubber Bands & Pouch)
// -------------------------------------------------------------
static void renderSlingshot() {
    auto& tm = TextureManager::getInstance();

    Vector2 pouchPos = g_isAiming ? g_aimPos : g_slingshotPos;

    // Back Fork
    tm.draw("catapult_fork_front", g_slingshotPos.x, g_slingshotPos.y + 10.0f, 44.0f, 90.0f);

    // Left & Right Rubber Tension Bands
    SDL_SetRenderDrawColor(g_renderer, 62, 39, 35, 255);
    for (int offset = -2; offset <= 2; ++offset) {
        SDL_RenderDrawLine(g_renderer, (int)g_leftForkAnchor.x + offset, (int)g_leftForkAnchor.y, (int)pouchPos.x - 12 + offset, (int)pouchPos.y);
        SDL_RenderDrawLine(g_renderer, (int)g_rightForkAnchor.x + offset, (int)g_rightForkAnchor.y, (int)pouchPos.x + 12 + offset, (int)pouchPos.y);
    }

    // Slingshot Leather Pouch
    tm.draw("slingshot_pouch", pouchPos.x, pouchPos.y, 34.0f, 34.0f);

    // Active Ready Bird inside pouch
    if (!g_activeFlightBird && g_ammoQueue.size() >= 0) {
        std::string birdTex = getBirdTextureKey(g_activeBirdType);
        tm.draw(birdTex, pouchPos.x, pouchPos.y, 44.0f, 44.0f);
    }

    // Ammunition Queue standing on ground
    float qx = 100.0f;
    float qy = (float)SCREEN_HEIGHT - 65.0f;
    for (size_t i = 0; i < g_ammoQueue.size(); ++i) {
        std::string waitingTex = getBirdTextureKey(g_ammoQueue[i]);
        tm.draw(waitingTex, qx - i * 36.0f, qy, 34.0f, 34.0f);
    }
}

// -------------------------------------------------------------
// Game Render Loop
// -------------------------------------------------------------
static void renderGame() {
    auto& tm = TextureManager::getInstance();

    // 1. Background Sky & Scenery
    tm.drawTopLeft("background_sky", 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);

    // 2. Trajectory Flight History Line
    if (g_lastFlightPath.size() > 1) {
        SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 70);
        for (size_t i = 1; i < g_lastFlightPath.size(); ++i) {
            SDL_RenderDrawLine(g_renderer, (int)g_lastFlightPath[i - 1].x, (int)g_lastFlightPath[i - 1].y,
                                           (int)g_lastFlightPath[i].x, (int)g_lastFlightPath[i].y);
        }
    }

    // 3. Aiming Trajectory Dots
    renderTrajectoryPrediction();

    // 4. Slingshot Base & Rubber Bands
    renderSlingshot();

    // 5. Render Physics Rigid Bodies with SVG-Converted Textures
    for (const auto& b : g_world.bodies) {
        if (b->isDead) continue;

        if (!b->textureKey.empty()) {
            if (b->shapeType == SHAPE_CIRCLE) {
                tm.draw(b->textureKey, b->position.x, b->position.y, b->radius * 2.0f, b->radius * 2.0f, b->angle);
            } else {
                tm.draw(b->textureKey, b->position.x, b->position.y, b->width, b->height, b->angle);
            }
        } else {
            // Fallback rasterization
            SDL_Color fillColor = GameRenderer::hexToSDL(b->color);
            SDL_Color outlineColor = { 33, 33, 33, 255 };

            if (b->shapeType == SHAPE_CIRCLE) {
                GameRenderer::drawFilledCircle(g_renderer, (int)b->position.x, (int)b->position.y, (int)b->radius, fillColor);
                GameRenderer::drawCircleOutline(g_renderer, (int)b->position.x, (int)b->position.y, (int)b->radius, outlineColor);
            } else {
                GameRenderer::drawPolygon(g_renderer, b->worldVertices, fillColor, outlineColor);
            }
        }

        // Damage flash highlight
        if (b->damageFlash > 0.0f && b->bodyType == BODY_DYNAMIC) {
            GameRenderer::drawFilledCircle(g_renderer, (int)b->position.x, (int)b->position.y, (int)(b->radius * 0.6f), { 255, 255, 255, 140 });
        }
    }

    // 6. Constraints / Joints
    for (const auto& c : g_world.constraints) {
        if (!c->isActive || !c->bodyA || !c->bodyB) continue;
        Vector2 pA = c->getWorldAnchorA();
        Vector2 pB = c->getWorldAnchorB();
        SDL_SetRenderDrawColor(g_renderer, 84, 110, 122, 220);
        SDL_RenderDrawLine(g_renderer, (int)pA.x, (int)pA.y, (int)pB.x, (int)pB.y);
    }

    // 7. Particles (Explosions, Smoke, Sparks, Stars)
    for (const auto& p : g_world.particleSystem.particles) {
        float alphaNorm = (p.life / p.maxLife);
        uint8_t alpha = (uint8_t)(alphaNorm * 255);

        if (p.type == 2) { // Explosion Fireball
            tm.draw("vfx_explosion", p.position.x, p.position.y, p.size * 3.0f, p.size * 3.0f, 0.0f, alpha);
        } else if (p.type == 1) { // Smoke
            tm.draw("vfx_smoke", p.position.x, p.position.y, p.size * 2.5f, p.size * 2.5f, 0.0f, (uint8_t)(alpha * 0.7f));
        } else {
            SDL_Color col = GameRenderer::hexToSDL(p.color);
            col.a = alpha;
            GameRenderer::drawFilledCircle(g_renderer, (int)p.position.x, (int)p.position.y, (int)p.size, col);
        }
    }

    // 8. Floating Score Popups
    for (const auto& sp : g_world.scorePopups) {
        float alphaNorm = (sp.life / sp.maxLife);
        SDL_Color col = GameRenderer::hexToSDL(sp.color);
        col.a = (uint8_t)(alphaNorm * 255);
        GameRenderer::drawText(g_renderer, sp.text, (int)sp.position.x, (int)sp.position.y, (int)sp.scale, col);
    }

    // -------------------------------------------------------------
    // HUD & UI Top Header Bar
    // -------------------------------------------------------------
    // Top banner backdrop
    SDL_Rect hudRect = { 0, 0, SCREEN_WIDTH, 64 };
    SDL_SetRenderDrawColor(g_renderer, 15, 23, 42, 225);
    SDL_RenderFillRect(g_renderer, &hudRect);

    SDL_SetRenderDrawColor(g_renderer, 245, 158, 11, 255);
    SDL_RenderDrawLine(g_renderer, 0, 64, SCREEN_WIDTH, 64);

    // Level Title & Description
    std::string titleStr = "LEVEL " + std::to_string(g_currentLevel) + ": " + g_currentLevelInfo.name;
    GameRenderer::drawText(g_renderer, titleStr, 20, 16, 2, { 254, 240, 138, 255 });
    GameRenderer::drawText(g_renderer, g_currentLevelInfo.description, 20, 42, 1, { 203, 213, 225, 255 });

    // Score & Targets
    std::string scoreStr = "SCORE: " + std::to_string(g_world.totalScore);
    GameRenderer::drawText(g_renderer, scoreStr, 400, 18, 2, { 255, 255, 255, 255 });

    std::string targetStr = "TARGETS: " + std::to_string(g_world.targetsRemaining) + " / " + std::to_string(g_world.totalTargets);
    SDL_Color targetCol = (g_world.targetsRemaining == 0) ? SDL_Color{ 118, 255, 3, 255 } : SDL_Color{ 255, 82, 82, 255 };
    GameRenderer::drawText(g_renderer, targetStr, 400, 42, 1, targetCol);

    // Quick Level Select Buttons [1] [2] [3] [4] [5]
    GameRenderer::drawText(g_renderer, "LEVELS:", 680, 24, 1, { 148, 163, 184, 255 });
    for (int lvl = 1; lvl <= 5; ++lvl) {
        SDL_Rect btnRect = { 740 + (lvl - 1) * 36, 16, 28, 28 };
        if (lvl == g_currentLevel) {
            SDL_SetRenderDrawColor(g_renderer, 245, 158, 11, 255);
        } else {
            SDL_SetRenderDrawColor(g_renderer, 51, 65, 85, 255);
        }
        SDL_RenderFillRect(g_renderer, &btnRect);
        SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 200);
        SDL_RenderDrawRect(g_renderer, &btnRect);

        std::string num = std::to_string(lvl);
        SDL_Color txtCol = (lvl == g_currentLevel) ? SDL_Color{ 15, 23, 42, 255 } : SDL_Color{ 255, 255, 255, 255 };
        GameRenderer::drawText(g_renderer, num, btnRect.x + 8, btnRect.y + 6, 2, txtCol);
    }

    // Reset & Slow-Mo Indicators
    GameRenderer::drawText(g_renderer, "[R] RESET", 940, 16, 1, { 226, 232, 240, 255 });
    std::string slowMoTxt = (g_world.timeScale < 1.0f) ? "[T] SLOW-MO: ON" : "[T] SLOW-MO";
    SDL_Color slowCol = (g_world.timeScale < 1.0f) ? SDL_Color{ 0, 230, 118, 255 } : SDL_Color{ 148, 163, 184, 255 };
    GameRenderer::drawText(g_renderer, slowMoTxt, 940, 36, 1, slowCol);

    // In-Flight Special Ability Banner Hint
    if (g_activeFlightBird && !g_activeFlightBird->isDead && !g_activeFlightBird->abilityUsed) {
        SDL_Rect tipRect = { SCREEN_WIDTH / 2 - 200, 80, 400, 32 };
        SDL_SetRenderDrawColor(g_renderer, 220, 38, 38, 230);
        SDL_RenderFillRect(g_renderer, &tipRect);
        SDL_SetRenderDrawColor(g_renderer, 254, 240, 138, 255);
        SDL_RenderDrawRect(g_renderer, &tipRect);

        std::string abilityHint = "CLICK / SPACE TO ACTIVATE ABILITY!";
        if (g_activeBirdType == 1) abilityHint = "CLICK / SPACE TO DETONATE BOMB!";
        if (g_activeBirdType == 2) abilityHint = "CLICK / SPACE TO TRIPLE SPLIT!";
        if (g_activeBirdType == 3) abilityHint = "CLICK / SPACE TO DRILL THRUST!";
        if (g_activeBirdType == 4) abilityHint = "CLICK / SPACE TO SLAM BOUNCE!";

        GameRenderer::drawText(g_renderer, abilityHint, tipRect.x + 24, tipRect.y + 9, 1, { 255, 255, 255, 255 });
    }

    // -------------------------------------------------------------
    // Victory & Defeat Overlays
    // -------------------------------------------------------------
    if (g_levelWon) {
        g_victoryAnimTimer += 0.016f;

        SDL_Rect modalRect = { SCREEN_WIDTH / 2 - 250, SCREEN_HEIGHT / 2 - 160, 500, 320 };
        SDL_SetRenderDrawColor(g_renderer, 15, 23, 42, 240);
        SDL_RenderFillRect(g_renderer, &modalRect);
        SDL_SetRenderDrawColor(g_renderer, 245, 158, 11, 255);
        SDL_RenderDrawRect(g_renderer, &modalRect);

        GameRenderer::drawText(g_renderer, "VICTORY! LEVEL CLEARED", modalRect.x + 60, modalRect.y + 35, 2, { 254, 240, 138, 255 });

        // Draw 3 Animated Stars
        int starCount = 1;
        if (g_world.totalScore > g_currentLevelInfo.targetScoreGoal * 0.6f) starCount = 2;
        if (g_world.totalScore >= g_currentLevelInfo.targetScoreGoal) starCount = 3;

        for (int s = 0; s < 3; ++s) {
            float starX = modalRect.x + 160 + s * 90;
            float starY = modalRect.y + 115;
            if (s < starCount && g_victoryAnimTimer > (0.3f + s * 0.3f)) {
                tm.draw("vfx_star", starX, starY, 64.0f, 64.0f);
            } else {
                GameRenderer::drawFilledCircle(g_renderer, (int)starX, (int)starY, 18, { 71, 85, 105, 255 });
            }
        }

        std::string finalScoreStr = "FINAL SCORE: " + std::to_string(g_world.totalScore);
        GameRenderer::drawText(g_renderer, finalScoreStr, modalRect.x + 120, modalRect.y + 175, 2, { 255, 255, 255, 255 });

        // Next Level Button
        SDL_Rect nextBtn = { modalRect.x + 80, modalRect.y + 225, 150, 48 };
        SDL_SetRenderDrawColor(g_renderer, 16, 185, 129, 255);
        SDL_RenderFillRect(g_renderer, &nextBtn);
        GameRenderer::drawText(g_renderer, "NEXT LEVEL", nextBtn.x + 20, nextBtn.y + 16, 1, { 255, 255, 255, 255 });

        // Replay Button
        SDL_Rect repBtn = { modalRect.x + 270, modalRect.y + 225, 150, 48 };
        SDL_SetRenderDrawColor(g_renderer, 71, 85, 105, 255);
        SDL_RenderFillRect(g_renderer, &repBtn);
        GameRenderer::drawText(g_renderer, "REPLAY [R]", repBtn.x + 28, repBtn.y + 16, 1, { 255, 255, 255, 255 });
    } else if (g_levelLost) {
        SDL_Rect modalRect = { SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 120, 440, 240 };
        SDL_SetRenderDrawColor(g_renderer, 15, 23, 42, 240);
        SDL_RenderFillRect(g_renderer, &modalRect);
        SDL_SetRenderDrawColor(g_renderer, 239, 68, 68, 255);
        SDL_RenderDrawRect(g_renderer, &modalRect);

        GameRenderer::drawText(g_renderer, "OUT OF AMMUNITION!", modalRect.x + 80, modalRect.y + 40, 2, { 255, 82, 82, 255 });
        GameRenderer::drawText(g_renderer, "Targets Still Survived!", modalRect.x + 120, modalRect.y + 85, 1, { 203, 213, 225, 255 });

        SDL_Rect repBtn = { modalRect.x + 145, modalRect.y + 140, 150, 48 };
        SDL_SetRenderDrawColor(g_renderer, 239, 68, 68, 255);
        SDL_RenderFillRect(g_renderer, &repBtn);
        GameRenderer::drawText(g_renderer, "RETRY [R]", repBtn.x + 35, repBtn.y + 16, 1, { 255, 255, 255, 255 });
    }
}

// -------------------------------------------------------------
// Game Step & Win/Loss Evaluation
// -------------------------------------------------------------
static void updateGame(float dt) {
    g_world.step(dt);

    // Track flight path of active bird
    if (g_activeFlightBird && !g_activeFlightBird->isDead) {
        g_flightTimer += dt;
        if (g_lastFlightPath.empty() || Vector2::distance(g_lastFlightPath.back(), g_activeFlightBird->position) > 16.0f) {
            g_lastFlightPath.push_back(g_activeFlightBird->position);
        }

        // Check if bird has stopped moving or fallen off map
        float speed = g_activeFlightBird->velocity.length();
        if (speed < 15.0f || g_activeFlightBird->position.x > SCREEN_WIDTH + 100.0f || g_activeFlightBird->position.y > SCREEN_HEIGHT + 100.0f) {
            g_settleTimer += dt;
        } else {
            g_settleTimer = 0.0f;
        }

        if (g_settleTimer > 1.8f || g_flightTimer > 12.0f) {
            g_activeFlightBird = nullptr;
            g_settleTimer = 0.0f;

            // Load next bird in queue
            if (!g_ammoQueue.empty()) {
                g_activeBirdType = g_ammoQueue.front();
                g_ammoQueue.erase(g_ammoQueue.begin());
            }
        }
    }

    // Check Victory (All targets destroyed!)
    if (g_world.targetsRemaining == 0 && !g_levelWon) {
        g_levelWon = true;
        // Bonus points for remaining birds
        int bonus = (int)g_ammoQueue.size() * 10000;
        if (bonus > 0) {
            g_world.totalScore += bonus;
            g_world.addScorePopup(g_slingshotPos + Vector2(0, -60), "+ " + std::to_string(bonus) + " AMMO BONUS!", 0x76FF03, 2.0f);
        }
        if (g_world.totalScore > g_highScores[g_currentLevel]) {
            g_highScores[g_currentLevel] = g_world.totalScore;
        }
    }

    // Check Defeat (Out of ammo and all physics settled)
    if (!g_levelWon && !g_levelLost && g_world.targetsRemaining > 0) {
        if (!g_activeFlightBird && g_ammoQueue.empty()) {
            g_settleTimer += dt;
            if (g_settleTimer > 2.5f) {
                g_levelLost = true;
            }
        }
    }
}

// -------------------------------------------------------------
// Main Event Loop
// -------------------------------------------------------------
static void mainLoop() {
    uint32_t currentTime = SDL_GetTicks();
    float dt = (currentTime - g_lastTime) / 1000.0f;
    if (dt > 0.05f) dt = 0.05f; // Clamp delta time
    g_lastTime = currentTime;

    // FPS calculation
    g_frameCount++;
    if (currentTime - g_fpsTimer >= 1000) {
        g_fps = g_frameCount;
        g_frameCount = 0;
        g_fpsTimer = currentTime;
    }

    // Input Handling
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            g_isRunning = false;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x;
            int my = event.button.y;

            if (event.button.button == SDL_BUTTON_LEFT) {
                // Check Header Level Select Buttons
                if (my >= 16 && my <= 44 && mx >= 740 && mx <= 920) {
                    int clickedLevel = (mx - 740) / 36 + 1;
                    if (clickedLevel >= 1 && clickedLevel <= 5) {
                        loadLevel(clickedLevel);
                    }
                    continue;
                }

                // Check Victory Next Level Button
                if (g_levelWon) {
                    SDL_Rect nextBtn = { SCREEN_WIDTH / 2 - 250 + 80, SCREEN_HEIGHT / 2 - 160 + 225, 150, 48 };
                    if (mx >= nextBtn.x && mx <= nextBtn.x + nextBtn.w && my >= nextBtn.y && my <= nextBtn.y + nextBtn.h) {
                        int nextLvl = (g_currentLevel < 5) ? g_currentLevel + 1 : 1;
                        loadLevel(nextLvl);
                        continue;
                    }
                }

                // Check Defeat Retry Button
                if (g_levelLost) {
                    SDL_Rect repBtn = { SCREEN_WIDTH / 2 - 220 + 145, SCREEN_HEIGHT / 2 - 120 + 140, 150, 48 };
                    if (mx >= repBtn.x && mx <= repBtn.x + repBtn.w && my >= repBtn.y && my <= repBtn.y + repBtn.h) {
                        loadLevel(g_currentLevel);
                        continue;
                    }
                }

                // If a bird is currently flying, click triggers its special ability!
                if (g_activeFlightBird && !g_activeFlightBird->isDead && !g_activeFlightBird->abilityUsed) {
                    triggerBirdAbility();
                } else if (!g_activeFlightBird && !g_levelWon && !g_levelLost) {
                    // Check if clicking near slingshot to start aiming
                    Vector2 clickPos((float)mx, (float)my);
                    if (Vector2::distance(clickPos, g_slingshotPos) < 70.0f) {
                        g_isAiming = true;
                        g_aimPos = clickPos;
                    }
                }
            }
        } else if (event.type == SDL_MOUSEMOTION) {
            if (g_isAiming) {
                Vector2 mousePos((float)event.motion.x, (float)event.motion.y);
                Vector2 pullOffset = mousePos - g_slingshotPos;
                if (pullOffset.length() > MAX_PULL_DISTANCE) {
                    pullOffset = pullOffset.normalized() * MAX_PULL_DISTANCE;
                }
                g_aimPos = g_slingshotPos + pullOffset;
            }
        } else if (event.type == SDL_MOUSEBUTTONUP) {
            if (event.button.button == SDL_BUTTON_LEFT && g_isAiming) {
                launchBird();
            }
        } else if (event.type == SDL_KEYDOWN) {
            SDL_Keycode key = event.key.keysym.sym;
            if (key >= SDLK_1 && key <= SDLK_5) {
                loadLevel(key - SDLK_1 + 1);
            } else if (key == SDLK_r) {
                loadLevel(g_currentLevel);
            } else if (key == SDLK_n) {
                int nextLvl = (g_currentLevel < 5) ? g_currentLevel + 1 : 1;
                loadLevel(nextLvl);
            } else if (key == SDLK_t) {
                g_world.timeScale = (g_world.timeScale < 1.0f) ? 1.0f : 0.25f;
            } else if (key == SDLK_SPACE) {
                if (g_activeFlightBird && !g_activeFlightBird->isDead && !g_activeFlightBird->abilityUsed) {
                    triggerBirdAbility();
                }
            }
        }
    }

    // Step Physics & Update Gameplay
    updateGame(dt);

    // Render Everything
    SDL_SetRenderDrawColor(g_renderer, 15, 23, 42, 255);
    SDL_RenderClear(g_renderer);
    renderGame();
    SDL_RenderPresent(g_renderer);
}

// -------------------------------------------------------------
// Entry Point
// -------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    g_window = SDL_CreateWindow(
        "C++ Physics Catapult Game",
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

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_SOFTWARE);
    }

    // Crucial for high-DPI, mobile responsive scaling, and proper touch event coordinate mapping
    SDL_RenderSetLogicalSize(g_renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize SVG -> PNG Texture Manager
    if (!TextureManager::getInstance().init(g_renderer)) {
        std::cerr << "Warning: TextureManager encountered issues during texture decoding." << std::endl;
    }

    // Load Initial Level
    loadLevel(1);

    g_lastTime = SDL_GetTicks();
    g_fpsTimer = g_lastTime;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while (g_isRunning) {
        mainLoop();
    }

    TextureManager::getInstance().cleanup();
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
#endif

    return 0;
}
