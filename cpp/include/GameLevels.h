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
        world.setGravity(Vector2(0.0f, 660.0f));

        // Ground Platform
        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";

        // Slingshot perch mound
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 750.0f;
        float baseFloor = screenHeight - 50.0f;

        // Ground Level 1 Columns (Height 110, Top = baseFloor - 110)
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx - 70.0f, baseFloor - 55.0f), 24.0f, 110.0f);
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx + 70.0f, baseFloor - 55.0f), 24.0f, 110.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx, baseFloor - 55.0f), 20.0f, 110.0f);
        addEnemy(world, "grunt", Vector2(cx - 35.0f, baseFloor - 22.0f));
        addEnemy(world, "grunt", Vector2(cx + 35.0f, baseFloor - 22.0f));

        // Tier 1 Deck (Height 22, Top = baseFloor - 132)
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 121.0f), 180.0f, 22.0f);

        // Tier 2 Pillars (Height 100, Top = baseFloor - 232)
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx - 50.0f, baseFloor - 182.0f), 24.0f, 100.0f);
        addBlock(world, MAT_WOOD, "wood_beam_v", Vector2(cx + 50.0f, baseFloor - 182.0f), 24.0f, 100.0f);
        addEnemy(world, "helmet", Vector2(cx, baseFloor - 154.0f));

        // Tier 2 Deck (Height 22, Top = baseFloor - 254)
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 243.0f), 140.0f, 22.0f);

        // Crown Roof Blocks (Height 40, Top = baseFloor - 294)
        addBlock(world, MAT_WOOD, "wood_block", Vector2(cx - 35.0f, baseFloor - 274.0f), 40.0f, 40.0f);
        addBlock(world, MAT_WOOD, "wood_block", Vector2(cx + 35.0f, baseFloor - 274.0f), 40.0f, 40.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx, baseFloor - 274.0f), 40.0f, 40.0f);
    }

    // LEVEL 2: TNT Munitions Vault
    static void buildLevel2(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.setGravity(Vector2(0.0f, 660.0f));

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 760.0f;
        float baseFloor = screenHeight - 50.0f;

        // Vault Pillars (Height 110, Top = baseFloor - 110)
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 100.0f, baseFloor - 55.0f), 26.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 100.0f, baseFloor - 55.0f), 26.0f, 110.0f);

        // Explosive Core
        addTNT(world, Vector2(cx - 45.0f, baseFloor - 22.0f));
        addTNT(world, Vector2(cx + 45.0f, baseFloor - 22.0f));
        addEnemy(world, "grunt", Vector2(cx, baseFloor - 22.0f));

        // Stone Slab Roof (Height 26, Top = baseFloor - 136)
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 123.0f), 230.0f, 26.0f);

        // Upper Tower (Height 100, Top = baseFloor - 236)
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx - 60.0f, baseFloor - 186.0f), 22.0f, 100.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx + 60.0f, baseFloor - 186.0f), 22.0f, 100.0f);
        addTNT(world, Vector2(cx, baseFloor - 158.0f));
        addEnemy(world, "helmet", Vector2(cx, baseFloor - 200.0f));

        // Upper Deck (Height 22, Top = baseFloor - 258)
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 247.0f), 160.0f, 22.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx - 45.0f, baseFloor - 280.0f), 44.0f, 44.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx + 45.0f, baseFloor - 280.0f), 44.0f, 44.0f);
        addEnemy(world, "king", Vector2(cx, baseFloor - 280.0f), 22.0f);
    }

    // LEVEL 3: Iron Bastion Keep
    static void buildLevel3(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.setGravity(Vector2(0.0f, 660.0f));

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float cx = 740.0f;
        float baseFloor = screenHeight - 50.0f;

        // Front Guard Bunker
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx - 160.0f, baseFloor - 22.0f), 44.0f, 44.0f);
        addEnemy(world, "helmet", Vector2(cx - 160.0f, baseFloor - 66.0f));

        // Steel Main Frame Base (Height 32, Top = baseFloor - 32)
        addBlock(world, MAT_METAL, "metal_girder", Vector2(cx, baseFloor - 16.0f), 180.0f, 32.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 70.0f, baseFloor - 87.0f), 26.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 70.0f, baseFloor - 87.0f), 26.0f, 110.0f);

        addEnemy(world, "grunt", Vector2(cx - 20.0f, baseFloor - 54.0f));
        addTNT(world, Vector2(cx + 25.0f, baseFloor - 54.0f));

        // Mid Heavy Steel Beam (Height 32, Top = baseFloor - 174)
        auto anchorBeam = addBlock(world, MAT_METAL, "metal_girder", Vector2(cx, baseFloor - 158.0f), 190.0f, 32.0f);

        // Hanging Cage Constraint with Enemy
        auto cage = addBlock(world, MAT_WOOD, "wood_block", Vector2(cx + 120.0f, baseFloor - 80.0f), 42.0f, 42.0f);
        world.createConstraint(CONSTRAINT_ROD, anchorBeam, cage, Vector2(60.0f, 16.0f), Vector2(0, -21.0f), 65.0f);
        addEnemy(world, "helmet", Vector2(cx + 120.0f, baseFloor - 102.0f));

        // High Castle Keep (Height 100, Top = baseFloor - 274)
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 45.0f, baseFloor - 224.0f), 24.0f, 100.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 45.0f, baseFloor - 224.0f), 24.0f, 100.0f);
        addEnemy(world, "king", Vector2(cx, baseFloor - 196.0f), 22.0f);

        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 286.0f), 130.0f, 24.0f);
        addBlock(world, MAT_STONE, "stone_block", Vector2(cx, baseFloor - 320.0f), 44.0f, 44.0f);
    }

    // LEVEL 4: Trebuchet Highlands
    static void buildLevel4(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.setGravity(Vector2(0.0f, 660.0f));

        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 25.0f), screenWidth, 50.0f);
        ground->textureKey = "ground_plate";
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(180.0f, screenHeight - 65.0f), 120.0f, 40.0f);

        float baseFloor = screenHeight - 50.0f;

        // Tower A (Left Fortress)
        float ax = 600.0f;
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(ax - 40.0f, baseFloor - 60.0f), 26.0f, 120.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(ax + 40.0f, baseFloor - 60.0f), 26.0f, 120.0f);
        addEnemy(world, "grunt", Vector2(ax, baseFloor - 22.0f));
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(ax, baseFloor - 131.0f), 120.0f, 22.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(ax, baseFloor - 187.0f), 22.0f, 90.0f);
        addEnemy(world, "helmet", Vector2(ax, baseFloor - 164.0f));

        // Tower B (Right Fortress)
        float bx = 850.0f;
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx - 45.0f, baseFloor - 70.0f), 26.0f, 140.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx + 45.0f, baseFloor - 70.0f), 26.0f, 140.0f);
        addTNT(world, Vector2(bx, baseFloor - 22.0f));
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(bx, baseFloor - 152.0f), 130.0f, 24.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx - 35.0f, baseFloor - 219.0f), 24.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(bx + 35.0f, baseFloor - 219.0f), 24.0f, 110.0f);
        addEnemy(world, "king", Vector2(bx, baseFloor - 186.0f), 22.0f);
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(bx, baseFloor - 285.0f), 110.0f, 22.0f);

        // Suspension Bridge linking both towers
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2((ax + bx) * 0.5f, baseFloor - 141.0f), 140.0f, 18.0f);
        addEnemy(world, "grunt", Vector2((ax + bx) * 0.5f, baseFloor - 160.0f));

        // Swinging Pendulum Boulder
        auto ceiling = world.createBox(BODY_STATIC, MAT_STONE, Vector2((ax + bx) * 0.5f, 140.0f), 40.0f, 20.0f);
        auto pendulum = world.createCircle(BODY_DYNAMIC, MAT_STONE, Vector2((ax + bx) * 0.5f, 250.0f), 24.0f);
        pendulum->textureKey = "stone_block";
        world.createConstraint(CONSTRAINT_ROD, ceiling, pendulum, Vector2(0, 10.0f), Vector2(0, 0), 100.0f);
    }

    // LEVEL 5: Royal King's Citadel
    static void buildLevel5(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.setGravity(Vector2(0.0f, 660.0f));

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

        addTNT(world, Vector2(cx - 95.0f, baseFloor - 22.0f));
        addEnemy(world, "grunt", Vector2(cx, baseFloor - 22.0f));
        addTNT(world, Vector2(cx + 95.0f, baseFloor - 22.0f));

        // Castle Ground Deck (Heavy Stone, Height 26, Top = baseFloor - 136)
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 123.0f), 320.0f, 26.0f);

        // Floor 2: Vertical supports (Height 110, Top = baseFloor - 246)
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx - 80.0f, baseFloor - 191.0f), 24.0f, 110.0f);
        addBlock(world, MAT_STONE, "stone_beam_v", Vector2(cx + 80.0f, baseFloor - 191.0f), 24.0f, 110.0f);
        addEnemy(world, "helmet", Vector2(cx - 40.0f, baseFloor - 158.0f));
        addEnemy(world, "helmet", Vector2(cx + 40.0f, baseFloor - 158.0f));

        // Floor 3 Deck (Height 22, Top = baseFloor - 268)
        addBlock(world, MAT_WOOD, "wood_beam_h", Vector2(cx, baseFloor - 257.0f), 200.0f, 22.0f);

        // Floor 3: Glass Sanctum & Royal Balcony (Height 80, Top = baseFloor - 348)
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx - 40.0f, baseFloor - 308.0f), 20.0f, 80.0f);
        addBlock(world, MAT_GLASS, "glass_beam_v", Vector2(cx + 40.0f, baseFloor - 308.0f), 20.0f, 80.0f);
        addTNT(world, Vector2(cx, baseFloor - 279.0f));

        // High Throne Turret (Height 22, Top = baseFloor - 370)
        addBlock(world, MAT_STONE, "stone_beam_h", Vector2(cx, baseFloor - 359.0f), 130.0f, 22.0f);

        // THE GRAND KING PIG
        addEnemy(world, "king", Vector2(cx, baseFloor - 392.0f), 22.0f);
    }
};

#endif // GAMELEVELS_H
