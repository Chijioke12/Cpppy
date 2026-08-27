#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "Vector2.h"
#include <vector>
#include <cmath>
#include <string>

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

struct AABB {
    Vector2 min;
    Vector2 max;

    bool overlaps(const AABB& other) const {
        if (max.x < other.min.x || min.x > other.max.x) return false;
        if (max.y < other.min.y || min.y > other.max.y) return false;
        return true;
    }
};

class RigidBody {
public:
    int id;
    std::string tag;
    std::string textureKey;
    BodyType bodyType;
    ShapeType shapeType;
    MaterialType material;

    Vector2 position;
    Vector2 velocity;
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

    // Polygon / Box local & world vertices
    std::vector<Vector2> localVertices;
    std::vector<Vector2> worldVertices;
    std::vector<Vector2> normals;

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
          position(_pos), velocity(0, 0), force(0, 0),
          angle(0.0f), angularVelocity(0.0f), torque(0.0f),
          mass(1.0f), invMass(1.0f), inertia(1.0f), invInertia(1.0f),
          restitution(0.2f), friction(0.4f), density(1.0f),
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
                density = 0.65f;
                restitution = 0.25f;
                friction = 0.55f;
                health = maxHealth = 90.0f;
                color = 0x8D6E63;
                scoreValue = 500;
                break;
            case MAT_STONE:
                density = 2.4f;
                restitution = 0.12f;
                friction = 0.7f;
                health = maxHealth = 240.0f;
                color = 0x78909C;
                scoreValue = 800;
                break;
            case MAT_GLASS:
                density = 0.9f;
                restitution = 0.05f;
                friction = 0.25f;
                health = maxHealth = 30.0f;
                color = 0x80DEEA;
                opacity = 0.85f;
                scoreValue = 400;
                break;
            case MAT_METAL:
                density = 4.2f;
                restitution = 0.18f;
                friction = 0.4f;
                health = maxHealth = 500.0f;
                color = 0x546E7A;
                scoreValue = 1200;
                break;
            case MAT_TNT:
                density = 1.0f;
                restitution = 0.2f;
                friction = 0.5f;
                health = maxHealth = 40.0f;
                color = 0xE53935;
                scoreValue = 1500;
                break;
            case MAT_TARGET:
                density = 1.2f;
                restitution = 0.35f;
                friction = 0.5f;
                health = maxHealth = 60.0f;
                color = 0x7CB342;
                isTarget = true;
                scoreValue = 5000;
                break;
            case MAT_RUBBER:
                density = 1.1f;
                restitution = 0.85f;
                friction = 0.35f;
                health = maxHealth = 150.0f;
                color = 0xFF5252;
                scoreValue = 600;
                break;
            case MAT_PROJECTILE:
                density = 2.2f;
                restitution = 0.3f;
                friction = 0.5f;
                health = maxHealth = 500.0f;
                color = 0xD32F2F;
                isProjectile = true;
                scoreValue = 0;
                break;
        }
    }

    void setCircle(float _radius) {
        shapeType = SHAPE_CIRCLE;
        radius = _radius;
        computeMass();
    }

    void setBox(float w, float h) {
        shapeType = SHAPE_BOX;
        width = w;
        height = h;

        localVertices.clear();
        float hw = w * 0.5f;
        float hh = h * 0.5f;
        localVertices.push_back(Vector2(-hw, -hh));
        localVertices.push_back(Vector2(hw, -hh));
        localVertices.push_back(Vector2(hw, hh));
        localVertices.push_back(Vector2(-hw, hh));

        updateWorldVertices();
        computeMass();
    }

    void computeMass() {
        if (bodyType == BODY_STATIC) {
            mass = 0.0f;
            invMass = 0.0f;
            inertia = 0.0f;
            invInertia = 0.0f;
            return;
        }

        if (shapeType == SHAPE_CIRCLE) {
            float area = 3.14159265f * radius * radius;
            mass = density * (area / 100.0f);
            inertia = 0.5f * mass * radius * radius;
        } else {
            float area = width * height;
            mass = density * (area / 100.0f);
            inertia = (1.0f / 12.0f) * mass * (width * width + height * height);
        }

        if (mass <= 0.0001f) mass = 0.0001f;
        invMass = 1.0f / mass;
        if (inertia <= 0.0001f) inertia = 0.0001f;
        invInertia = 1.0f / inertia;
    }

    void updateWorldVertices() {
        if (shapeType == SHAPE_CIRCLE) return;

        worldVertices.resize(localVertices.size());
        normals.resize(localVertices.size());

        for (size_t i = 0; i < localVertices.size(); ++i) {
            worldVertices[i] = position + localVertices[i].rotate(angle);
        }

        for (size_t i = 0; i < worldVertices.size(); ++i) {
            size_t next = (i + 1) % worldVertices.size();
            Vector2 edge = worldVertices[next] - worldVertices[i];
            normals[i] = edge.perpendicular().normalized();
        }
    }

    AABB getAABB() const {
        AABB aabb;
        if (shapeType == SHAPE_CIRCLE) {
            aabb.min = Vector2(position.x - radius, position.y - radius);
            aabb.max = Vector2(position.x + radius, position.y + radius);
        } else {
            if (worldVertices.empty()) {
                aabb.min = position;
                aabb.max = position;
                return aabb;
            }
            aabb.min = worldVertices[0];
            aabb.max = worldVertices[0];
            for (size_t i = 1; i < worldVertices.size(); ++i) {
                aabb.min.x = std::min(aabb.min.x, worldVertices[i].x);
                aabb.min.y = std::min(aabb.min.y, worldVertices[i].y);
                aabb.max.x = std::max(aabb.max.x, worldVertices[i].x);
                aabb.max.y = std::max(aabb.max.y, worldVertices[i].y);
            }
        }
        return aabb;
    }

    void applyForce(const Vector2& f) {
        if (bodyType == BODY_STATIC) return;
        force += f;
    }

    void applyForceAtPoint(const Vector2& f, const Vector2& pt) {
        if (bodyType == BODY_STATIC) return;
        force += f;
        torque += (pt - position).cross(f);
    }

    void applyImpulse(const Vector2& impulse, const Vector2& contactVector) {
        if (bodyType == BODY_STATIC) return;
        velocity += impulse * invMass;
        angularVelocity += invInertia * contactVector.cross(impulse);
    }

    void takeDamage(float dmg) {
        if (bodyType == BODY_STATIC) return;
        health -= dmg;
        damageFlash = 0.25f;
        if (health <= 0.0f) {
            health = 0.0f;
            isDead = true;
        }
    }
};

#endif // RIGIDBODY_H
