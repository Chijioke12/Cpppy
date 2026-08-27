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
    MAT_METAL = 1,
    MAT_STONE = 2,
    MAT_GLASS = 3,
    MAT_RUBBER = 4,
    MAT_TNT = 5,
    MAT_FLESH = 6,
    MAT_TARGET = 7,
    MAT_ICE = 8,
    MAT_BOUNCER = 9,
    MAT_PLANET = 10,
    MAT_BLACK_HOLE = 11,
    MAT_PORTAL = 12,
    MAT_ROAD = 13
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
    bool isCollected;
    bool isSleeping;
    float sleepTimer;

    // Motor / Kinematic speed
    float motorSpeed;

    // Visual attributes
    unsigned int color;
    float opacity;
    bool isGlow;

    RigidBody(int _id, BodyType _bType, ShapeType _sType, MaterialType _mat, Vector2 _pos)
        : id(_id), tag(""), bodyType(_bType), shapeType(_sType), material(_mat),
          position(_pos), velocity(0, 0), force(0, 0),
          angle(0.0f), angularVelocity(0.0f), torque(0.0f),
          mass(1.0f), invMass(1.0f), inertia(1.0f), invInertia(1.0f),
          restitution(0.2f), friction(0.4f), density(1.0f),
          radius(20.0f), width(40.0f), height(40.0f),
          health(100.0f), maxHealth(100.0f), isDead(false), isExploded(false),
          isTarget(false), isCollected(false), isSleeping(false), sleepTimer(0.0f),
          motorSpeed(0.0f), color(0xFFFFFF), opacity(1.0f), isGlow(false)
    {
        applyMaterialProperties();
    }

    void applyMaterialProperties() {
        switch (material) {
            case MAT_WOOD:
                density = 0.6f;
                restitution = 0.25f;
                friction = 0.5f;
                health = maxHealth = 75.0f;
                color = 0xB58863;
                break;
            case MAT_STONE:
                density = 2.4f;
                restitution = 0.1f;
                friction = 0.7f;
                health = maxHealth = 220.0f;
                color = 0x8A8D91;
                break;
            case MAT_METAL:
                density = 4.0f;
                restitution = 0.15f;
                friction = 0.35f;
                health = maxHealth = 400.0f;
                color = 0x4E5D6C;
                break;
            case MAT_GLASS:
                density = 1.2f;
                restitution = 0.05f;
                friction = 0.2f;
                health = maxHealth = 20.0f;
                color = 0x90CAF9;
                opacity = 0.85f;
                break;
            case MAT_RUBBER:
                density = 1.1f;
                restitution = 0.88f;
                friction = 0.8f;
                health = maxHealth = 120.0f;
                color = 0xFF5722;
                break;
            case MAT_TNT:
                density = 0.8f;
                restitution = 0.2f;
                friction = 0.5f;
                health = maxHealth = 30.0f;
                color = 0xD32F2F;
                break;
            case MAT_FLESH:
                density = 1.0f;
                restitution = 0.15f;
                friction = 0.6f;
                health = maxHealth = 80.0f;
                color = 0xF48FB1;
                break;
            case MAT_TARGET:
                density = 0.9f;
                restitution = 0.3f;
                friction = 0.5f;
                health = maxHealth = 40.0f;
                color = 0x4CAF50;
                isTarget = true;
                break;
            case MAT_BOUNCER:
                density = 2.0f;
                restitution = 1.6f;
                friction = 0.1f;
                color = 0x00E676;
                isGlow = true;
                break;
            case MAT_PLANET:
                density = 10.0f;
                restitution = 0.05f;
                friction = 0.6f;
                color = 0x3F51B5;
                break;
            case MAT_BLACK_HOLE:
                density = 50.0f;
                restitution = 0.0f;
                friction = 0.0f;
                color = 0x212121;
                isGlow = true;
                break;
            case MAT_ROAD:
                density = 5.0f;
                restitution = 0.1f;
                friction = 0.85f;
                color = 0x37474F;
                break;
            default:
                break;
        }
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
        calculateMass();
        updateTransform();
    }

    void setCircle(float r) {
        shapeType = SHAPE_CIRCLE;
        radius = r;
        localVertices.clear();
        calculateMass();
    }

    void calculateMass() {
        if (bodyType == BODY_STATIC) {
            mass = 0.0f;
            invMass = 0.0f;
            inertia = 0.0f;
            invInertia = 0.0f;
            return;
        }

        if (shapeType == SHAPE_CIRCLE) {
            float area = 3.14159265f * radius * radius;
            mass = area * density * 0.01f;
            inertia = 0.5f * mass * radius * radius;
        } else if (shapeType == SHAPE_BOX) {
            float area = width * height;
            mass = area * density * 0.01f;
            inertia = (1.0f / 12.0f) * mass * (width * width + height * height);
        } else {
            mass = 10.0f;
            inertia = 1000.0f;
        }

        // Clamp mass bounds for numerical stability
        if (mass < 0.1f) mass = 0.1f;
        if (inertia < 1.0f) inertia = 1.0f;

        invMass = 1.0f / mass;
        invInertia = 1.0f / inertia;
    }

    void updateTransform() {
        if (shapeType == SHAPE_BOX || shapeType == SHAPE_POLYGON) {
            worldVertices.resize(localVertices.size());
            normals.resize(localVertices.size());

            float cosA = std::cos(angle);
            float sinA = std::sin(angle);

            for (size_t i = 0; i < localVertices.size(); ++i) {
                Vector2 local = localVertices[i];
                worldVertices[i] = Vector2(
                    position.x + (local.x * cosA - local.y * sinA),
                    position.y + (local.x * sinA + local.y * cosA)
                );
            }

            for (size_t i = 0; i < worldVertices.size(); ++i) {
                Vector2 current = worldVertices[i];
                Vector2 next = worldVertices[(i + 1) % worldVertices.size()];
                Vector2 edge = next - current;
                normals[i] = Vector2(-edge.y, edge.x).normalized();
            }
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
            } else {
                aabb.min = worldVertices[0];
                aabb.max = worldVertices[0];
                for (size_t i = 1; i < worldVertices.size(); ++i) {
                    aabb.min.x = std::min(aabb.min.x, worldVertices[i].x);
                    aabb.min.y = std::min(aabb.min.y, worldVertices[i].y);
                    aabb.max.x = std::max(aabb.max.x, worldVertices[i].x);
                    aabb.max.y = std::max(aabb.max.y, worldVertices[i].y);
                }
            }
        }
        return aabb;
    }

    void applyForce(const Vector2& f) {
        if (bodyType != BODY_DYNAMIC) return;
        force += f;
    }

    void applyForceAtPoint(const Vector2& f, const Vector2& p) {
        if (bodyType != BODY_DYNAMIC) return;
        force += f;
        Vector2 r = p - position;
        torque += r.cross(f);
    }

    void applyImpulse(const Vector2& impulse, const Vector2& contactVector) {
        if (bodyType != BODY_DYNAMIC) return;
        velocity += impulse * invMass;
        angularVelocity += invInertia * contactVector.cross(impulse);
    }

    void takeDamage(float dmg) {
        if (bodyType == BODY_STATIC) return;
        health -= dmg;
        if (health <= 0.0f) {
            health = 0.0f;
            isDead = true;
            if (material == MAT_TNT) {
                isExploded = true;
            }
        }
    }

    void integrate(float dt, const Vector2& gravity, float airResistance) {
        if (bodyType == BODY_STATIC) return;

        if (bodyType == BODY_KINEMATIC) {
            angle += motorSpeed * dt;
            updateTransform();
            return;
        }

        // Apply forces & gravity
        Vector2 totalAccel = (force * invMass) + gravity;
        velocity += totalAccel * dt;
        angularVelocity += (torque * invInertia) * dt;

        // Apply linear & angular damping
        float linearDamping = std::pow(1.0f - airResistance, dt * 60.0f);
        float angularDamping = std::pow(0.985f, dt * 60.0f);
        velocity *= linearDamping;
        angularVelocity *= angularDamping;

        // Position update
        position += velocity * dt;
        angle += angularVelocity * dt;

        // Reset accumulator
        force = Vector2(0, 0);
        torque = 0.0f;

        updateTransform();
    }
};

#endif // RIGIDBODY_H
