#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "RigidBody.h"
#include "Vector2.h"
#include <box2d/box2d.h>

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

    b2Joint* b2_joint;

    float restLength;
    float breakForce;
    bool isBroken;
    bool isActive;

    unsigned int color;

    Constraint(int _id, ConstraintType _type, RigidBody* _a, RigidBody* _b,
               Vector2 _anchorA, Vector2 _anchorB, float _length = -1.0f)
        : id(_id), type(_type), bodyA(_a), bodyB(_b),
          localAnchorA(_anchorA), localAnchorB(_anchorB),
          b2_joint(nullptr),
          breakForce(5000.0f), isBroken(false), isActive(true),
          color(0x9E9E9E)
    {
        Vector2 worldA = getWorldAnchorA();
        Vector2 worldB = getWorldAnchorB();
        if (_length < 0.0f) {
            restLength = Vector2::distance(worldA, worldB);
        } else {
            restLength = _length;
        }

        if (type == CONSTRAINT_ROD) {
            color = 0x795548;
            breakForce = 8000.0f;
        } else if (type == CONSTRAINT_ROPE) {
            color = 0xD7CCC8;
            breakForce = 4500.0f;
        } else if (type == CONSTRAINT_SPRING) {
            color = 0x00BCD4;
            breakForce = 3500.0f;
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
};

#endif // CONSTRAINT_H
