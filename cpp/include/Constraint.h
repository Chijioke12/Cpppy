#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "RigidBody.h"
#include "Vector2.h"

enum ConstraintType {
    CONSTRAINT_SPRING = 0,
    CONSTRAINT_ROD = 1,
    CONSTRAINT_ROPE = 2,
    CONSTRAINT_WELD = 3,
    CONSTRAINT_ELASTIC = 4
};

class Constraint {
public:
    int id;
    ConstraintType type;
    RigidBody* bodyA;
    RigidBody* bodyB;

    Vector2 localAnchorA;
    Vector2 localAnchorB;

    float restLength;
    float stiffness;     // Spring stiffness k
    float damping;       // Damping coefficient
    float breakForce;    // Max tension before snapping
    bool isBroken;
    bool isActive;

    // Visuals
    unsigned int color;
    float currentStress; // 0..1 ratio of tension to breakForce for color grading

    Constraint(int _id, ConstraintType _type, RigidBody* _a, RigidBody* _b,
               Vector2 _anchorA, Vector2 _anchorB, float _length = -1.0f)
        : id(_id), type(_type), bodyA(_a), bodyB(_b),
          localAnchorA(_anchorA), localAnchorB(_anchorB),
          stiffness(200.0f), damping(5.0f), breakForce(50000.0f), isBroken(false), isActive(true),
          color(0x9E9E9E), currentStress(0.0f)
    {
        Vector2 worldA = getWorldAnchorA();
        Vector2 worldB = getWorldAnchorB();
        if (_length < 0.0f) {
            restLength = Vector2::distance(worldA, worldB);
        } else {
            restLength = _length;
        }

        if (type == CONSTRAINT_ROD) {
            stiffness = 1000.0f;
            damping = 15.0f;
            breakForce = 80000.0f;
            color = 0x795548;
        } else if (type == CONSTRAINT_ROPE) {
            stiffness = 800.0f;
            damping = 10.0f;
            breakForce = 45000.0f;
            color = 0xD7CCC8;
        } else if (type == CONSTRAINT_SPRING) {
            stiffness = 180.0f;
            damping = 4.0f;
            breakForce = 35000.0f;
            color = 0x00BCD4;
        }
    }

    Vector2 getWorldAnchorA() const {
        if (!bodyA) return localAnchorA;
        return bodyA->position + localAnchorA.rotate(bodyA->angle);
    }

    Vector2 getWorldAnchorB() const {
        if (!bodyB) return localAnchorB;
        return bodyB->position + localAnchorB.rotate(bodyB->angle);
    }

    void solve(float dt = 0.016f) {
        if (isBroken || !isActive) return;
        if (!bodyA && !bodyB) return;

        Vector2 pA = getWorldAnchorA();
        Vector2 pB = getWorldAnchorB();
        Vector2 delta = pB - pA;
        float currentDist = delta.length();

        if (currentDist < 0.0001f) return;

        Vector2 dir = delta / currentDist;
        float displacement = currentDist - restLength;

        // Rope constraint: only resists tension (stretching), not compression
        if (type == CONSTRAINT_ROPE && displacement < 0.0f) {
            currentStress = 0.0f;
            return;
        }

        // Relative velocity at anchors
        Vector2 vA = bodyA ? (bodyA->velocity + Vector2(-bodyA->angularVelocity * (pA.y - bodyA->position.y),
                                                         bodyA->angularVelocity * (pA.x - bodyA->position.x))) : Vector2(0, 0);
        Vector2 vB = bodyB ? (bodyB->velocity + Vector2(-bodyB->angularVelocity * (pB.y - bodyB->position.y),
                                                         bodyB->angularVelocity * (pB.x - bodyB->position.x))) : Vector2(0, 0);

        Vector2 relVel = vB - vA;
        float velAlongDir = relVel.dot(dir);

        // Hooke's spring force + damping
        float springForceMag = stiffness * displacement;
        float dampingForceMag = damping * velAlongDir;
        float totalForceMag = springForceMag + dampingForceMag;

        // Measure tension stress
        float tension = std::abs(totalForceMag);
        currentStress = std::min(1.0f, tension / breakForce);

        // Break if exceeds limit
        if (tension > breakForce) {
            isBroken = true;
            isActive = false;
            return;
        }

        Vector2 forceVec = dir * totalForceMag;

        if (bodyA && bodyA->bodyType == BODY_DYNAMIC) {
            bodyA->applyForceAtPoint(forceVec, pA);
        }
        if (bodyB && bodyB->bodyType == BODY_DYNAMIC) {
            bodyB->applyForceAtPoint(-forceVec, pB);
        }
    }

    // Positional relaxation pass for stiff rods and ragdoll joints
    void solvePosition() {
        if (isBroken || !isActive) return;
        if (type != CONSTRAINT_ROD && type != CONSTRAINT_WELD) return;

        Vector2 pA = getWorldAnchorA();
        Vector2 pB = getWorldAnchorB();
        Vector2 delta = pB - pA;
        float currentDist = delta.length();
        if (currentDist < 0.0001f) return;

        float error = currentDist - restLength;
        Vector2 dir = delta / currentDist;

        float invM_A = (bodyA && bodyA->bodyType == BODY_DYNAMIC) ? bodyA->invMass : 0.0f;
        float invM_B = (bodyB && bodyB->bodyType == BODY_DYNAMIC) ? bodyB->invMass : 0.0f;
        float sumInv = invM_A + invM_B;
        if (sumInv <= 0.00001f) return;

        Vector2 correction = dir * (error / sumInv) * 0.4f;

        if (bodyA && bodyA->bodyType == BODY_DYNAMIC) {
            bodyA->position += correction * invM_A;
            bodyA->updateWorldVertices();
        }
        if (bodyB && bodyB->bodyType == BODY_DYNAMIC) {
            bodyB->position -= correction * invM_B;
            bodyB->updateWorldVertices();
        }
    }
};

#endif // CONSTRAINT_H
