#ifndef GAMELEVELS_H
#define GAMELEVELS_H

#include "PhysicsWorld.h"
#include <memory>
#include <string>

class GameLevels {
public:
    static void spawnRagdoll(PhysicsWorld& world, float x, float y, float scale = 1.0f) {
        // Head
        auto head = world.createCircle(BODY_DYNAMIC, MAT_FLESH, Vector2(x, y - 40.0f * scale), 14.0f * scale);
        head->tag = "ragdoll_head";

        // Torso
        auto torso = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x, y), 20.0f * scale, 45.0f * scale);
        torso->tag = "ragdoll_torso";

        // Upper Arms
        auto leftUpperArm = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x - 20.0f * scale, y - 10.0f * scale), 10.0f * scale, 24.0f * scale);
        auto rightUpperArm = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x + 20.0f * scale, y - 10.0f * scale), 10.0f * scale, 24.0f * scale);

        // Lower Arms
        auto leftLowerArm = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x - 20.0f * scale, y + 16.0f * scale), 8.0f * scale, 24.0f * scale);
        auto rightLowerArm = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x + 20.0f * scale, y + 16.0f * scale), 8.0f * scale, 24.0f * scale);

        // Upper Legs
        auto leftUpperLeg = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x - 8.0f * scale, y + 42.0f * scale), 11.0f * scale, 30.0f * scale);
        auto rightUpperLeg = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x + 8.0f * scale, y + 42.0f * scale), 11.0f * scale, 30.0f * scale);

        // Lower Legs
        auto leftLowerLeg = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x - 8.0f * scale, y + 74.0f * scale), 9.0f * scale, 30.0f * scale);
        auto rightLowerLeg = world.createBox(BODY_DYNAMIC, MAT_FLESH, Vector2(x + 8.0f * scale, y + 74.0f * scale), 9.0f * scale, 30.0f * scale);

        // Constraints: Neck
        auto neck = world.createConstraint(CONSTRAINT_ROD, head, torso, Vector2(0, 14.0f * scale), Vector2(0, -22.0f * scale));
        neck->stiffness = 500.0f;

        // Shoulder joints
        world.createConstraint(CONSTRAINT_ROD, torso, leftUpperArm, Vector2(-10.0f * scale, -18.0f * scale), Vector2(0, -10.0f * scale));
        world.createConstraint(CONSTRAINT_ROD, torso, rightUpperArm, Vector2(10.0f * scale, -18.0f * scale), Vector2(0, -10.0f * scale));

        // Elbow joints
        world.createConstraint(CONSTRAINT_ROD, leftUpperArm, leftLowerArm, Vector2(0, 12.0f * scale), Vector2(0, -12.0f * scale));
        world.createConstraint(CONSTRAINT_ROD, rightUpperArm, rightLowerArm, Vector2(0, 12.0f * scale), Vector2(0, -12.0f * scale));

        // Hip joints
        world.createConstraint(CONSTRAINT_ROD, torso, leftUpperLeg, Vector2(-6.0f * scale, 22.0f * scale), Vector2(0, -15.0f * scale));
        world.createConstraint(CONSTRAINT_ROD, torso, rightUpperLeg, Vector2(6.0f * scale, 22.0f * scale), Vector2(0, -15.0f * scale));

        // Knee joints
        world.createConstraint(CONSTRAINT_ROD, leftUpperLeg, leftLowerLeg, Vector2(0, 15.0f * scale), Vector2(0, -15.0f * scale));
        world.createConstraint(CONSTRAINT_ROD, rightUpperLeg, rightLowerLeg, Vector2(0, 15.0f * scale), Vector2(0, -15.0f * scale));
    }

    static void spawnVehicle(PhysicsWorld& world, float x, float y, float motorVel = 8.0f) {
        // Truck chassis
        auto chassis = world.createBox(BODY_DYNAMIC, MAT_METAL, Vector2(x, y), 120.0f, 32.0f);
        chassis->tag = "truck_chassis";
        chassis->color = 0x1E88E5;

        // Cabin
        auto cabin = world.createBox(BODY_DYNAMIC, MAT_GLASS, Vector2(x + 35.0f, y - 25.0f), 45.0f, 25.0f);
        cabin->color = 0x90CAF9;
        world.createConstraint(CONSTRAINT_ROD, chassis, cabin, Vector2(35.0f, -16.0f), Vector2(0, 12.0f));

        // Wheels
        auto frontWheel = world.createCircle(BODY_DYNAMIC, MAT_RUBBER, Vector2(x + 42.0f, y + 25.0f), 18.0f);
        frontWheel->friction = 0.95f;
        frontWheel->angularVelocity = motorVel;

        auto rearWheel = world.createCircle(BODY_DYNAMIC, MAT_RUBBER, Vector2(x - 42.0f, y + 25.0f), 18.0f);
        rearWheel->friction = 0.95f;
        rearWheel->angularVelocity = motorVel;

        // Suspension Springs
        auto springFront = world.createConstraint(CONSTRAINT_SPRING, chassis, frontWheel, Vector2(42.0f, 16.0f), Vector2(0, 0), 22.0f);
        springFront->stiffness = 350.0f;
        springFront->damping = 12.0f;

        auto springRear = world.createConstraint(CONSTRAINT_SPRING, chassis, rearWheel, Vector2(-42.0f, 16.0f), Vector2(0, 0), 22.0f);
        springRear->stiffness = 350.0f;
        springRear->damping = 12.0f;
    }

    // LEVEL 1: Demolition Castle Siege
    static void buildLevel1(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);
        world.airResistance = 0.002f;

        // Ground & Terrain
        auto ground = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 20.0f), screenWidth, 40.0f);
        ground->color = 0x3E2723;

        // Slingshot catapult mount
        auto mount = world.createBox(BODY_STATIC, MAT_WOOD, Vector2(160.0f, screenHeight - 90.0f), 24.0f, 100.0f);
        mount->color = 0x5D4037;

        // Castle structure on the right
        float baseX = screenWidth - 320.0f;
        float groundY = screenHeight - 40.0f;

        // Foundation blocks (Stone)
        world.createBox(BODY_DYNAMIC, MAT_STONE, Vector2(baseX - 80.0f, groundY - 35.0f), 30.0f, 70.0f);
        world.createBox(BODY_DYNAMIC, MAT_STONE, Vector2(baseX + 80.0f, groundY - 35.0f), 30.0f, 70.0f);
        world.createBox(BODY_DYNAMIC, MAT_STONE, Vector2(baseX, groundY - 35.0f), 30.0f, 70.0f);

        // Floor 1 Beam (Wood)
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(baseX, groundY - 75.0f), 210.0f, 14.0f);

        // Middle room with TNT and Target King
        world.createBox(BODY_DYNAMIC, MAT_TNT, Vector2(baseX - 40.0f, groundY - 105.0f), 34.0f, 34.0f);
        auto king = world.createCircle(BODY_DYNAMIC, MAT_TARGET, Vector2(baseX + 40.0f, groundY - 105.0f), 20.0f);
        king->tag = "target_boss";

        // Second story pillars
        world.createBox(BODY_DYNAMIC, MAT_GLASS, Vector2(baseX - 70.0f, groundY - 145.0f), 20.0f, 65.0f);
        world.createBox(BODY_DYNAMIC, MAT_GLASS, Vector2(baseX + 70.0f, groundY - 145.0f), 20.0f, 65.0f);

        // Floor 2 Beam
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(baseX, groundY - 185.0f), 180.0f, 14.0f);

        // Top tower with guard target
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(baseX - 35.0f, groundY - 225.0f), 18.0f, 60.0f);
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(baseX + 35.0f, groundY - 225.0f), 18.0f, 60.0f);
        world.createBox(BODY_DYNAMIC, MAT_STONE, Vector2(baseX, groundY - 265.0f), 110.0f, 20.0f);

        auto guard = world.createCircle(BODY_DYNAMIC, MAT_TARGET, Vector2(baseX, groundY - 220.0f), 16.0f);
        guard->tag = "target_guard";

        // Secondary small outpost tower
        float outpostX = baseX - 220.0f;
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(outpostX - 25.0f, groundY - 45.0f), 20.0f, 90.0f);
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(outpostX + 25.0f, groundY - 45.0f), 20.0f, 90.0f);
        world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(outpostX, groundY - 95.0f), 85.0f, 12.0f);
        world.createBox(BODY_DYNAMIC, MAT_TNT, Vector2(outpostX, groundY - 120.0f), 30.0f, 30.0f);
        auto outpostTarget = world.createCircle(BODY_DYNAMIC, MAT_TARGET, Vector2(outpostX, groundY - 45.0f), 16.0f);
        outpostTarget->tag = "target_outpost";
    }

    // LEVEL 2: Bridge Constructor & Load Stress Simulation
    static void buildLevel2(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);
        world.airResistance = 0.002f;

        float cliffY = screenHeight - 180.0f;
        float cliffW = 280.0f;

        // Left Cliff
        auto leftCliff = world.createBox(BODY_STATIC, MAT_STONE, Vector2(cliffW * 0.5f, cliffY + 200.0f), cliffW, 400.0f);
        leftCliff->color = 0x455A64;

        // Right Cliff
        auto rightCliff = world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth - cliffW * 0.5f, cliffY + 200.0f), cliffW, 400.0f);
        rightCliff->color = 0x455A64;

        // Left Anchor Point
        auto anchorL = world.createCircle(BODY_STATIC, MAT_METAL, Vector2(cliffW, cliffY), 10.0f);
        // Right Anchor Point
        auto anchorR = world.createCircle(BODY_STATIC, MAT_METAL, Vector2(screenWidth - cliffW, cliffY), 10.0f);

        // Build Truss Bridge across chasm
        int numSegments = 7;
        float span = (screenWidth - cliffW * 2.0f);
        float segW = span / float(numSegments);

        std::vector<std::shared_ptr<RigidBody>> deckNodes;
        deckNodes.push_back(anchorL);

        for (int i = 1; i < numSegments; ++i) {
            float nx = cliffW + i * segW;
            float ny = cliffY;
            auto node = world.createCircle(BODY_DYNAMIC, MAT_METAL, Vector2(nx, ny), 7.0f);
            node->color = 0x9E9E9E;
            deckNodes.push_back(node);
        }
        deckNodes.push_back(anchorR);

        // Deck Road Planks & Rod Constraints
        for (int i = 0; i < numSegments; ++i) {
            auto nA = deckNodes[i];
            auto nB = deckNodes[i + 1];

            // Road plank body
            Vector2 mid = (nA->position + nB->position) * 0.5f;
            auto plank = world.createBox(BODY_DYNAMIC, MAT_ROAD, mid, segW + 4.0f, 10.0f);
            plank->tag = "road_plank";

            auto rodA = world.createConstraint(CONSTRAINT_ROD, nA, plank, Vector2(0, 0), Vector2(-segW * 0.5f, 0));
            rodA->stiffness = 1800.0f;
            rodA->breakForce = 95000.0f;

            auto rodB = world.createConstraint(CONSTRAINT_ROD, plank, nB, Vector2(segW * 0.5f, 0), Vector2(0, 0));
            rodB->stiffness = 1800.0f;
            rodB->breakForce = 95000.0f;

            // Direct node-to-node structural rod
            auto directRod = world.createConstraint(CONSTRAINT_ROD, nA, nB, Vector2(0, 0), Vector2(0, 0));
            directRod->stiffness = 2200.0f;
            directRod->breakForce = 120000.0f;
        }

        // Suspension Tower & Cables
        auto towerL = world.createCircle(BODY_STATIC, MAT_METAL, Vector2(cliffW, cliffY - 120.0f), 12.0f);
        auto towerR = world.createCircle(BODY_STATIC, MAT_METAL, Vector2(screenWidth - cliffW, cliffY - 120.0f), 12.0f);

        // Support Cables
        world.createConstraint(CONSTRAINT_ROPE, towerL, deckNodes[2], Vector2(0, 0), Vector2(0, 0))->breakForce = 100000.0f;
        world.createConstraint(CONSTRAINT_ROPE, towerL, deckNodes[3], Vector2(0, 0), Vector2(0, 0))->breakForce = 100000.0f;
        world.createConstraint(CONSTRAINT_ROPE, towerR, deckNodes[4], Vector2(0, 0), Vector2(0, 0))->breakForce = 100000.0f;
        world.createConstraint(CONSTRAINT_ROPE, towerR, deckNodes[5], Vector2(0, 0), Vector2(0, 0))->breakForce = 100000.0f;

        // Vehicle starting on left bank
        spawnVehicle(world, 140.0f, cliffY - 40.0f, 10.0f);
    }

    // LEVEL 3: Orbital Gravity Slingshot & Black Hole Navigation
    static void buildLevel3(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 0.0f); // Zero Gravity!
        world.airResistance = 0.0002f;

        // Central Sun / Massive Planet
        Vector2 center(screenWidth * 0.5f, screenHeight * 0.5f);
        auto planet1 = world.createCircle(BODY_STATIC, MAT_PLANET, center, 48.0f);
        planet1->color = 0xFF9800; // Glowing Sun
        planet1->isGlow = true;
        planet1->tag = "sun";
        world.addGravityWell(center, 95.0f, 500.0f, false);

        // Orbiting Moon 1
        Vector2 moon1Pos = center + Vector2(180.0f, -60.0f);
        auto moon1 = world.createCircle(BODY_STATIC, MAT_PLANET, moon1Pos, 26.0f);
        moon1->color = 0x00E5FF;
        moon1->tag = "ice_moon";
        world.addGravityWell(moon1Pos, 35.0f, 250.0f, false);

        // Orbiting Moon 2 (Volcanic)
        Vector2 moon2Pos = center + Vector2(-200.0f, 80.0f);
        auto moon2 = world.createCircle(BODY_STATIC, MAT_PLANET, moon2Pos, 22.0f);
        moon2->color = 0xE91E63;
        moon2->tag = "fire_moon";
        world.addGravityWell(moon2Pos, 28.0f, 200.0f, false);

        // Black Hole Singularity
        Vector2 blackHolePos(screenWidth * 0.78f, screenHeight * 0.22f);
        auto blackHole = world.createCircle(BODY_STATIC, MAT_BLACK_HOLE, blackHolePos, 20.0f);
        blackHole->color = 0x111111;
        blackHole->isGlow = true;
        blackHole->tag = "black_hole";
        world.addGravityWell(blackHolePos, 140.0f, 320.0f, true);

        // Energy Crystal Targets in orbit
        auto c1 = world.createCircle(BODY_DYNAMIC, MAT_TARGET, center + Vector2(0, -140.0f), 14.0f);
        c1->velocity = Vector2(75.0f, 0);
        c1->color = 0x76FF03;

        auto c2 = world.createCircle(BODY_DYNAMIC, MAT_TARGET, center + Vector2(0, 140.0f), 14.0f);
        c2->velocity = Vector2(-75.0f, 0);
        c2->color = 0x76FF03;

        auto c3 = world.createCircle(BODY_DYNAMIC, MAT_TARGET, moon1Pos + Vector2(0, 60.0f), 12.0f);
        c3->color = 0x76FF03;

        // Wormhole Exit Gate
        Vector2 portalPos(screenWidth - 100.0f, screenHeight * 0.82f);
        auto portal = world.createCircle(BODY_STATIC, MAT_BOUNCER, portalPos, 32.0f);
        portal->color = 0xAA00FF;
        portal->isGlow = true;
        portal->tag = "warp_portal";

        // Space Probe / Capsule at launch bay
        auto probe = world.createCircle(BODY_DYNAMIC, MAT_METAL, Vector2(100.0f, screenHeight * 0.5f), 12.0f);
        probe->tag = "player_probe";
        probe->color = 0xFFFFFF;
    }

    // LEVEL 4: Ragdoll Pinball & Chaos Arena
    static void buildLevel4(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 750.0f);
        world.airResistance = 0.001f;

        // Arena boundary walls
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 15.0f), screenWidth, 30.0f);
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, 15.0f), screenWidth, 30.0f);
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(15.0f, screenHeight * 0.5f), 30.0f, screenHeight);
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth - 15.0f, screenHeight * 0.5f), 30.0f, screenHeight);

        // Angled deflector ramps
        world.createBox(BODY_STATIC, MAT_METAL, Vector2(120.0f, screenHeight - 120.0f), 180.0f, 20.0f, -0.6f);
        world.createBox(BODY_STATIC, MAT_METAL, Vector2(screenWidth - 120.0f, screenHeight - 120.0f), 180.0f, 20.0f, 0.6f);

        // Super Bouncers (High Restitution)
        auto b1 = world.createCircle(BODY_STATIC, MAT_BOUNCER, Vector2(screenWidth * 0.35f, 220.0f), 38.0f);
        b1->tag = "super_bouncer";
        auto b2 = world.createCircle(BODY_STATIC, MAT_BOUNCER, Vector2(screenWidth * 0.65f, 220.0f), 38.0f);
        b2->tag = "super_bouncer";
        auto b3 = world.createCircle(BODY_STATIC, MAT_BOUNCER, Vector2(screenWidth * 0.5f, 360.0f), 45.0f);
        b3->tag = "super_bouncer";

        // Rotating Spinner Paddles (Kinematic)
        auto paddle1 = world.createBox(BODY_KINEMATIC, MAT_METAL, Vector2(screenWidth * 0.3f, screenHeight - 260.0f), 140.0f, 18.0f);
        paddle1->motorSpeed = 3.5f;
        paddle1->color = 0xFF9100;

        auto paddle2 = world.createBox(BODY_KINEMATIC, MAT_METAL, Vector2(screenWidth * 0.7f, screenHeight - 260.0f), 140.0f, 18.0f);
        paddle2->motorSpeed = -3.5f;
        paddle2->color = 0xFF9100;

        // Targets to hit
        for (int i = 0; i < 5; ++i) {
            float tx = screenWidth * 0.3f + i * (screenWidth * 0.4f / 4.0f);
            auto target = world.createCircle(BODY_DYNAMIC, MAT_TARGET, Vector2(tx, 90.0f), 16.0f);
            target->tag = "pinball_target";
        }

        // Spawn Ragdolls into the arena!
        spawnRagdoll(world, screenWidth * 0.45f, 120.0f, 0.9f);
        spawnRagdoll(world, screenWidth * 0.55f, 120.0f, 0.9f);

        // Pinball Heavy Ball
        auto ball = world.createCircle(BODY_DYNAMIC, MAT_METAL, Vector2(screenWidth * 0.5f, 60.0f), 22.0f);
        ball->restitution = 0.9f;
        ball->color = 0xE0E0E0;
    }

    // LEVEL 5: Total Physics Sandbox & Lab
    static void buildLevel5(PhysicsWorld& world, float screenWidth, float screenHeight) {
        world.clear();
        world.gravity = Vector2(0.0f, 980.0f);
        world.airResistance = 0.002f;

        // Floor and walls
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth * 0.5f, screenHeight - 15.0f), screenWidth, 30.0f);
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(15.0f, screenHeight * 0.5f), 30.0f, screenHeight);
        world.createBox(BODY_STATIC, MAT_STONE, Vector2(screenWidth - 15.0f, screenHeight * 0.5f), 30.0f, screenHeight);

        // Newton's Cradle demonstration
        float cradleX = 220.0f;
        float cradleY = 160.0f;
        float cradleR = 18.0f;
        int numCradles = 5;

        for (int i = 0; i < numCradles; ++i) {
            float bx = cradleX + i * (cradleR * 2.0f);
            auto anchor = world.createCircle(BODY_STATIC, MAT_METAL, Vector2(bx, cradleY), 6.0f);
            
            Vector2 ballPos(bx, cradleY + 140.0f);
            if (i == 0) ballPos = Vector2(bx - 100.0f, cradleY + 90.0f); // Pulled back!

            auto ball = world.createCircle(BODY_DYNAMIC, MAT_METAL, ballPos, cradleR);
            ball->restitution = 0.98f;
            ball->friction = 0.05f;
            ball->color = 0xCFD8DC;

            auto rope = world.createConstraint(CONSTRAINT_ROD, anchor, ball, Vector2(0, 0), Vector2(0, 0), 140.0f);
            rope->stiffness = 2000.0f;
            rope->color = 0x90A4AE;
        }

        // Domino cascade
        float dominoX = screenWidth * 0.45f;
        for (int i = 0; i < 9; ++i) {
            auto domino = world.createBox(BODY_DYNAMIC, MAT_WOOD, Vector2(dominoX + i * 35.0f, screenHeight - 55.0f), 10.0f, 60.0f);
            domino->friction = 0.6f;
            domino->color = (i % 2 == 0) ? 0xFFC107 : 0x03A9F4;
        }

        // TNT stack with wrecking ball
        float tntX = screenWidth - 180.0f;
        world.createBox(BODY_DYNAMIC, MAT_TNT, Vector2(tntX - 25.0f, screenHeight - 45.0f), 35.0f, 35.0f);
        world.createBox(BODY_DYNAMIC, MAT_TNT, Vector2(tntX + 25.0f, screenHeight - 45.0f), 35.0f, 35.0f);
        world.createBox(BODY_DYNAMIC, MAT_TNT, Vector2(tntX, screenHeight - 85.0f), 35.0f, 35.0f);

        // Wrecking ball suspended from ceiling
        auto craneAnchor = world.createCircle(BODY_STATIC, MAT_METAL, Vector2(screenWidth - 180.0f, 100.0f), 10.0f);
        auto wreckingBall = world.createCircle(BODY_DYNAMIC, MAT_METAL, Vector2(screenWidth - 300.0f, 150.0f), 32.0f);
        wreckingBall->density = 8.0f;
        wreckingBall->calculateMass();
        wreckingBall->color = 0x37474F;

        auto craneChain = world.createConstraint(CONSTRAINT_ROD, craneAnchor, wreckingBall, Vector2(0, 0), Vector2(0, 0), 220.0f);
        craneChain->stiffness = 2500.0f;
        craneChain->breakForce = 500000.0f;

        // Ragdoll tester
        spawnRagdoll(world, screenWidth * 0.4f, 260.0f, 0.85f);
    }
};

#endif // GAMELEVELS_H
