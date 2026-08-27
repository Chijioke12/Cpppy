#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "Vector2.h"
#include <box2d/box2d.h>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

enum ShapeType {
    SHAPE_CIRCLE = 0,
    SHAPE_BOX = 1,
    SHAPE_POLYGON = 2
};

enum BodyType {
    BODY_DYNAMIC = 0,
    BODY_STATIC = 1,
    BODY_KINEMATIC = 2
};

enum MaterialType {
    MAT_WOOD = 0,
    MAT_STONE = 1,
    MAT_GLASS = 2,
    MAT_METAL = 3,
    MAT_TNT = 4,
    MAT_TARGET = 5,
    MAT_RUBBER = 6,
    MAT_PROJECTILE = 7
};

class RigidBody {
public:
    int id;
    std::string tag;
    std::string textureKey;
    BodyType bodyType;
    ShapeType shapeType;
    MaterialType material;

    // Box2D body & fixture references
    b2Body* b2_body;
    b2Fixture* b2_fixture;

    Vector2 position;      // in screen pixels
    Vector2 velocity;      // in px/s
    Vector2 force;

    float angle;           // in radians
    float angularVelocity; // in rad/s
    float torque;

    float mass;
    float invMass;
    float inertia;
    float invInertia;

    float restitution;     // bounciness [0..1]
    float friction;        // friction coefficient [0..1]
    float density;

    // Circle params
    float radius;

    // Box params
    float width;
    float height;

    // Polygon / Box local & world vertices in pixel coordinates
    std::vector<Vector2> localVertices;
    std::vector<Vector2> worldVertices;

    // Gameplay & Destruction attributes
    float health;
    float maxHealth;
    bool isDead;
    bool isExploded;
    bool isTarget;
    bool isTargetEliminated;
    int scoreValue;
    bool isBoss;

    // Projectile Specifics
    bool isProjectile;
    int projectileType;   // 0: Red, 1: Bomb, 2: Split, 3: Drill, 4: Rubber
    bool abilityUsed;
    float flightTime;
    float fuseTimer;
    bool isArmed;

    // Visual attributes
    unsigned int color;
    float opacity;
    float damageFlash;

    RigidBody(int _id, BodyType _bType, ShapeType _sType, MaterialType _mat, Vector2 _pos)
        : id(_id), tag(""), textureKey(""), bodyType(_bType), shapeType(_sType), material(_mat),
          b2_body(nullptr), b2_fixture(nullptr),
          position(_pos), velocity(0, 0), force(0, 0),
          angle(0.0f), angularVelocity(0.0f), torque(0.0f),
          mass(1.0f), invMass(1.0f), inertia(1.0f), invInertia(1.0f),
          restitution(0.1f), friction(0.7f), density(1.0f),
          radius(20.0f), width(40.0f), height(40.0f),
          health(100.0f), maxHealth(100.0f), isDead(false), isExploded(false),
          isTarget(false), isTargetEliminated(false), scoreValue(500), isBoss(false),
          isProjectile(false), projectileType(0), abilityUsed(false), flightTime(0.0f), fuseTimer(-1.0f), isArmed(false),
          color(0xFFFFFF), opacity(1.0f), damageFlash(0.0f)
    {
        applyMaterialProperties();
    }

    void applyMaterialProperties() {
        switch (material) {
            case MAT_WOOD:
                density = 1.0f;
                restitution = 0.05f;
                friction = 0.75f;
                health = maxHealth = 90.0f;
                color = 0x8D6E63;
                scoreValue = 500;
                break;
            case MAT_STONE:
                density = 2.8f;
                restitution = 0.02f;
                friction = 0.85f;
                health = maxHealth = 260.0f;
                color = 0x78909C;
                scoreValue = 800;
                break;
            case MAT_GLASS:
                density = 0.85f;
                restitution = 0.02f;
                friction = 0.5f;
                health = maxHealth = 35.0f;
                color = 0x80DEEA;
                scoreValue = 600;
                break;
            case MAT_METAL:
                density = 4.5f;
                restitution = 0.02f;
                friction = 0.75f;
                health = maxHealth = 550.0f;
                color = 0x455A64;
                scoreValue = 1200;
                break;
            case MAT_TNT:
                density = 1.2f;
                restitution = 0.05f;
                friction = 0.75f;
                health = maxHealth = 25.0f;
                color = 0xD32F2F;
                scoreValue = 1500;
                break;
            case MAT_TARGET:
                density = 1.1f;
                restitution = 0.05f;
                friction = 0.65f;
                health = maxHealth = 40.0f;
                color = 0x76FF03;
                isTarget = true;
                scoreValue = 5000;
                break;
            case MAT_RUBBER:
                density = 1.0f;
                restitution = 0.92f;
                friction = 0.8f;
                health = maxHealth = 150.0f;
                color = 0xE91E63;
                scoreValue = 400;
                break;
            case MAT_PROJECTILE:
                density = 2.2f;
                restitution = 0.2f;
                friction = 0.5f;
                health = maxHealth = 300.0f;
                color = 0xF44336;
                isProjectile = true;
                scoreValue = 0;
                break;
        }
    }

    void setBox(float w, float h) {
        shapeType = SHAPE_BOX;
        width = w;
        height = h;
        float hw = w * 0.5f;
        float hh = h * 0.5f;

        localVertices = {
            Vector2(-hw, -hh),
            Vector2(hw, -hh),
            Vector2(hw, hh),
            Vector2(-hw, hh)
        };
        updateWorldVertices();
    }

    void setCircle(float r) {
        shapeType = SHAPE_CIRCLE;
        radius = r;
        width = r * 2.0f;
        height = r * 2.0f;
    }

    void updateWorldVertices() {
        worldVertices.resize(localVertices.size());
        float c = std::cos(angle);
        float s = std::sin(angle);
        for (size_t i = 0; i < localVertices.size(); ++i) {
            Vector2 v = localVertices[i];
            worldVertices[i] = position + Vector2(v.x * c - v.y * s, v.x * s + v.y * c);
        }
    }

    void syncFromB2(float ppm) {
        if (!b2_body) return;

        b2Vec2 pos = b2_body->GetPosition();
        position = Vector2(pos.x * ppm, pos.y * ppm);
        angle = b2_body->GetAngle();

        b2Vec2 vel = b2_body->GetLinearVelocity();
        velocity = Vector2(vel.x * ppm, vel.y * ppm);
        angularVelocity = b2_body->GetAngularVelocity();
        mass = b2_body->GetMass();
        invMass = (mass > 0.0001f) ? 1.0f / mass : 0.0f;

        if (shapeType == SHAPE_BOX || shapeType == SHAPE_POLYGON) {
            updateWorldVertices();
        }
    }

    void applyImpulse(Vector2 impulsePixels, Vector2 pointOffsetPixels, float ppm) {
        if (!b2_body) return;
        b2Vec2 imp(impulsePixels.x / ppm, impulsePixels.y / ppm);
        b2Vec2 pt = b2_body->GetWorldPoint(b2Vec2(pointOffsetPixels.x / ppm, pointOffsetPixels.y / ppm));
        b2_body->ApplyLinearImpulse(imp, pt, true);
    }

    void setVelocity(Vector2 velPixels, float ppm) {
        velocity = velPixels;
        if (b2_body) {
            b2_body->SetLinearVelocity(b2Vec2(velPixels.x / ppm, velPixels.y / ppm));
        }
    }

    void takeDamage(float amount) {
        health -= amount;
        damageFlash = 0.25f;
        if (health <= 0.0f) {
            health = 0.0f;
            isDead = true;
        }
    }
};

#endif // RIGIDBODY_H
