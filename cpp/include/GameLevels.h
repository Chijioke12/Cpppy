#ifndef GAMELEVELS_H
#define GAMELEVELS_H

#include "PhysicsWorld.h"
#include <memory>
#include <string>
#include <vector>

struct LevelInfo {
    int id;
    std::string name;
    std::string description;
    std::vector<int> birdQueue; // 0: Red, 1: Bomb, 2: Split, 3: Drill, 4: Rubber
    int targetScoreGoal;
};

class GameLevels {
public:
    static LevelInfo getLevelInfo(int levelId) {
        LevelInfo info;
        info.id = levelId;
        if (levelId == 1) {
            info.name = "Outpost Assault";
            info.description = "Demolish the frontier wooden watchtower!";
            info.birdQueue = { 0, 0, 1 };
            info.targetScoreGoal = 18000;
        } else if (levelId == 2) {
            info.name = "TNT Munitions Vault";
            info.description = "Ignite the explosive core for a chain reaction!";
            info.birdQueue = { 2, 1, 0 };
            info.targetScoreGoal = 28000;
        } else if (levelId == 3) {
            info.name = "Iron Bastion Keep";
            info.description = "Use the Iron Drill to pierce reinforced steel girders!";
            info.birdQueue = { 3, 2, 1, 0 };
            info.targetScoreGoal = 36000;
        } else if (levelId == 4) {
            info.name = "Trebuchet Highlands";
            info.description = "Ricochet through double cliffside towers!";
            info.birdQueue = { 4, 3, 2, 1 };
            info.targetScoreGoal = 45000;
        } else {
            info.name = "Royal King's Citadel";
            info.description = "Topple the Grand King Pig's royal fortress!";
            info.birdQueue = { 1, 3, 2, 4, 0 };
            info.targetScoreGoal = 60000;
        }
        return info;
    }

    // Helper to spawn a textured block
    static std::shared_ptr<RigidBody> addBlock(PhysicsWorld& world, MaterialType mat, const std::string& texKey, Vector2 pos, float w, float h, float angle = 0.0f) {
        auto b = world.createBox(BODY_DYNAMIC, mat, pos, w, h, angle);
        b->textureKey = texKey;
        return b;
    }

    // Helper to spawn an enemy pig
    static std::shared_ptr<RigidBody> addEnemy(PhysicsWorld& world, const std::string& type, Vector2 pos, float radius = 22.0f) {
        auto b = world.createCircle(BODY_DYNAMIC, MAT_TARGET, pos, radius);
        b->tag = "enemy";
        b->textureKey = "enemy_" + type;
        if (type == "king") {
            b->isBoss = true;
            b->health = b->maxHealth = 140.0f;
            b->scoreValue = 10000;
        } else if (type == "helmet") {
            b->health = b->maxHealth = 90.0f;
            b->scoreValue = 7000;
        } else {
            b->scoreValue = 5000;
        }
        world.targetsRemaining++;
        world.totalTargets++;
        return b;
    }

    // Helper to spawn a TNT barrel
    static std::shared_ptr<RigidBody> addTNT(PhysicsWorld& world, Vector2 pos) {
        auto b = world.createBox(BODY_DYNAMIC, MAT_TNT, pos, 44.0f, 44.0f);
        b->textureKey = "tnt_crate";
        return b;
    }

    // LEVEL 1: Outpost Assault
    static void buildLevel1(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);

        // Ground Platform
        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";

        // Slingshot perch mound
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 750.0f;
        float baseFloor = screenHeight - 50.0f;

        // Ground Level 1 Columns
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx - 70.0f, baseFloor - 55.0f), 24.0f, 110.0f);
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx + 70.0f, baseFloor - 55.0f), 24.0f, 110.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx, baseFloor - 55.0f), 20.0f, 110.0f);
        addEnemy(world, "grunt", Vector2(cx - 35.0f, baseFloor - 22.0f));
        addEnemy(world, "grunt", Vector2(cx + 35.0f, baseFloor - 22.0f));

        // Tier 1 Deck
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 115.0f), 180.0f, 22.0f);

        // Tier 2 Pillars
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx - 50.0f, baseFloor - 175.0f), 24.0f, 100.0f);
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx + 50.0f, baseFloor - 175.0f), 24.0f, 100.0f);
        addEnemy(world, "helmet", Vector2(cx, baseFloor - 140.0f));

        // Tier 2 Deck
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 230.0f), 140.0f, 22.0f);

        // Crown Roof Blocks
        addBlock(world, MAT_WOOD, "wood_block", Vector2(cx - 35.0f, baseFloor - 255.0f), 40.0f, 40.0f);
        addBlock(world, MAT_WOOD, "wood_block", Vector2(cx + 35.0f, baseFloor - 255.0f), 40.0f, 40.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx, baseFloor - 280.0f), 42.0f, 42.0f);
    }

    // LEVEL 2: TNT Munitions Vault
    static void buildLevel2(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 760.0f;
        float baseFloor = screenHeight - 50.0f;

        // Vault Pillars
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 100.0f, baseFloor - 55.0f), 26.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 100.0f, baseFloor - 55.0f), 26.0f, 110.0f);

        // Explosive Core
        addTNT(world, Vector2(cx - 35.0f, baseFloor - 24.0f));
        addTNT(world, Vector2(cx + 35.0f, baseFloor - 24.0f));
        addEnemy(world, "grunt", Vector2(cx, baseFloor - 24.0f));

        // Stone Slab Roof
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 118.0f), 230.0f, 26.0f);

        // Upper Tower
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx - 60.0f, baseFloor - 175.0f), 22.0f, 100.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx + 60.0f, baseFloor - 175.0f), 22.0f, 100.0f);
        addTNT(world, Vector2(cx, baseFloor - 145.0f));
        addEnemy(world, "helmet", Vector2(cx, baseFloor - 195.0f));

        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 232.0f), 160.0f, 22.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx - 45.0f, baseFloor - 260.0f), 44.0f, 44.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx + 45.0f, baseFloor - 260.0f), 44.0f, 44.0f);
        addEnemy(world, "king", Vector2(cx, baseFloor - 260.0f), 24.0f);
    }

    // LEVEL 3: Iron Bastion Keep
    static void buildLevel3(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 740.0f;
        float baseFloor = screenHeight - 50.0f;

        // Front Guard Bunker
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx - 160.0f, baseFloor - 22.0f), 44.0f, 44.0f);
        addEnemy(world, "helmet", Vector2(cx - 160.0f, baseFloor - 66.0f));

        // Steel Main Frame
        addBlock(world, MAT_METAL, "metal_girder", Vector2(cx, baseFloor - 16.0f), 140.0f, 32.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 70.0f, baseFloor - 80.0f), 26.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 70.0f, baseFloor - 80.0f), 26.0f, 110.0f);

        addEnemy(world, "grunt", Vector2(cx - 20.0f, baseFloor - 55.0f));
        addTNT(world, Vector2(cx + 25.0f, baseFloor - 55.0f));

        // Mid Heavy Steel Beam
        addBlock(world, MAT_METAL, "metal_girder", Vector2(cx, baseFloor - 145.0f), 190.0f, 32.0f);

        // Hanging Cage Constraint with Enemy
        auto cage = addBlock(world, MAT_WOOD, "wood_block", Vector2(cx + 140.0f, baseFloor - 80.0f), 42.0f, 42.0f);
        auto anchorBeam = addBlock(world, MAT_METAL, "metal_girder", Vector2(cx + 140.0f, baseFloor - 145.0f), 80.0f, 26.0f);
        world.createConstraint(CONSTRAINT_ROD, anchorBeam, cage, Vector2(0, 13.0f), Vector2(0, -21.0f), 60.0f);
        addEnemy(world, "helmet", Vector2(cx + 140.0f, baseFloor - 110.0f));

        // High Castle Keep
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 45.0f, baseFloor - 210.0f), 24.0f, 100.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 45.0f, baseFloor - 210.0f), 24.0f, 100.0f);
        addEnemy(world, "king", Vector2(cx, baseFloor - 180.0f), 25.0f);

        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 268.0f), 130.0f, 24.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx, baseFloor - 295.0f), 44.0f, 44.0f);
    }

    // LEVEL 4: Trebuchet Highlands
    static void buildLevel4(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float baseFloor = screenHeight - 50.0f;

        // Tower A (Left Fortress)
        float ax = 600.0f;
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(ax - 40.0f, baseFloor - 60.0f), 26.0f, 120.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(ax + 40.0f, baseFloor - 60.0f), 26.0f, 120.0f);
        addEnemy(world, "grunt", Vector2(ax, baseFloor - 24.0f));
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(ax, baseFloor - 130.0f), 120.0f, 22.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(ax, baseFloor - 185.0f), 22.0f, 90.0f);
        addEnemy(world, "helmet", Vector2(ax, baseFloor - 240.0f));

        // Tower B (Right Fortress)
        float bx = 850.0f;
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx - 45.0f, baseFloor - 70.0f), 26.0f, 140.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx + 45.0f, baseFloor - 70.0f), 26.0f, 140.0f);
        addTNT(world, Vector2(bx, baseFloor - 24.0f));
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(bx, baseFloor - 150.0f), 130.0f, 24.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx - 35.0f, baseFloor - 215.0f), 24.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx + 35.0f, baseFloor - 215.0f), 24.0f, 110.0f);
        addEnemy(world, "king", Vector2(bx, baseFloor - 185.0f), 25.0f);
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(bx, baseFloor - 275.0f), 110.0f, 22.0f);

        // Suspension Bridge linking both towers
        auto bridge = addBlock(world, MAT_WOOD, "wood_beam_h", Vector2((ax + bx) * 0.5f, baseFloor - 140.0f), 140.0f, 18.0f);
        addEnemy(world, "grunt", Vector2((ax + bx) * 0.5f, baseFloor - 160.0f));

        // Swinging Pendulum Boulder
        auto ceiling = world.createBox(BODY_STATIC, MAT_STONE, Vector2((ax + bx) * 0.5f, 140.0f), 40.0f, 20.0f);
        auto pendulum = world.createCircle(BODY_DYNAMIC, MAT_STONE, Vector2((ax + bx) * 0.5f, 260.0f), 26.0f);
        pendulum->textureKey = "stone_block";
        world.createConstraint(CONSTRAINT_ROD, ceiling, pendulum, Vector2(0, 10.0f), Vector2(0, 0), 120.0f);
    }

    // LEVEL 5: Royal King's Citadel
    static void buildLevel5(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 750.0f;
        float baseFloor = screenHeight - 50.0f;

        // Fortress Base Cellar with 3 TNT Crates
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 140.0f, baseFloor - 55.0f), 28.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 50.0f, baseFloor - 55.0f), 28.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 50.0f, baseFloor - 55.0f), 28.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 140.0f, baseFloor - 55.0f), 28.0f, 110.0f);

        addTNT(world, Vector2(cx - 95.0f, baseFloor - 24.0f));
        addEnemy(world, "grunt", Vector2(cx, baseFloor - 24.0f));
        addTNT(world, Vector2(cx + 95.0f, baseFloor - 24.0f));

        // Castle Ground Deck (Heavy Stone)
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 118.0f), 320.0f, 26.0f);

        // Floor 2: Steel Truss & Guards
        addBlock(world, MAT_METAL, "metal_girder", Vector2(cx - 90.0f, baseFloor - 165.0f), 90.0f, 30.0f);
        addBlock(world, MAT_METAL, "metal_girder", Vector2(cx + 90.0f, baseFloor - 165.0f), 90.0f, 30.0f);
        addEnemy(world, "helmet", Vector2(cx - 90.0f, baseFloor - 195.0f));
        addEnemy(world, "helmet", Vector2(cx + 90.0f, baseFloor - 195.0f));

        // Floor 3: Glass Sanctum & Royal Balcony
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx - 45.0f, baseFloor - 240.0f), 22.0f, 110.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx + 45.0f, baseFloor - 240.0f), 22.0f, 110.0f);
        addTNT(world, Vector2(cx, baseFloor - 200.0f));

        // High Throne Turret
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 302.0f), 160.0f, 24.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx - 45.0f, baseFloor - 335.0f), 42.0f, 42.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx + 45.0f, baseFloor - 335.0f), 42.0f, 42.0f);

        // THE GRAND KING PIG
        addEnemy(world, "king", Vector2(cx, baseFloor - 340.0f), 28.0f);
    }
};

#endif // GAMELEVELS_H
