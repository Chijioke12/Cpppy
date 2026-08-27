#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "RigidBody.h"
#include "Constraint.h"
#include "Collision.h"
#include "ParticleSystem.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

struct GravityWell {
    Vector2 position;
    float strength; // positive = attractor, negative = repulsor
    float radius;
    bool isBlackHole;
};

class PhysicsWorld {
public:
    std::vector<std::shared_ptr<RigidBody>> bodies;
    std::vector<std::shared_ptr<Constraint>> constraints;
    std::vector<GravityWell> gravityWells;
    ParticleSystem particleSystem;

    Vector2 gravity;
    float airResistance;
    int velocityIterations;
    int positionIterations;
    float timeScale;

    // Next entity ID
    int nextBodyId;
    int nextConstraintId;

    // Mouse drag constraint
    int draggedBodyId;
    Vector2 dragLocalAnchor;
    Vector2 dragTargetPos;
    bool isDragging;

    // Performance metrics
    int totalCollisionsLastStep;
    float stepDurationMs;

    PhysicsWorld()
        : gravity(0.0f, 980.0f), airResistance(0.002f),
          velocityIterations(8), positionIterations(4), timeScale(1.0f),
          nextBodyId(1), nextConstraintId(1),
          draggedBodyId(-1), dragLocalAnchor(0, 0), dragTargetPos(0, 0), isDragging(false),
          totalCollisionsLastStep(0), stepDurationMs(0.0f)
    {}

    std::shared_ptr<RigidBody> createCircle(BodyType bType, MaterialType mat, Vector2 pos, float radius) {
        auto body = std::make_shared<RigidBody>(nextBodyId++, bType, SHAPE_CIRCLE, mat, pos);
        body->setCircle(radius);
        bodies.push_back(body);
        return body;
    }

    std::shared_ptr<RigidBody> createBox(BodyType bType, MaterialType mat, Vector2 pos, float width, float height, float angle = 0.0f) {
        auto body = std::make_shared<RigidBody>(nextBodyId++, bType, SHAPE_BOX, mat, pos);
        body->angle = angle;
        body->setBox(width, height);
        bodies.push_back(body);
        return body;
    }

    std::shared_ptr<Constraint> createConstraint(ConstraintType type, std::shared_ptr<RigidBody> a, std::shared_ptr<RigidBody> b,
                                                 Vector2 localA, Vector2 localB, float length = -1.0f) {
        auto c = std::make_shared<Constraint>(nextConstraintId++, type, a.get(), b.get(), localA, localB, length);
        constraints.push_back(c);
        return c;
    }

    void addGravityWell(Vector2 pos, float strength, float radius, bool isBlackHole = false) {
        GravityWell gw;
        gw.position = pos;
        gw.strength = strength;
        gw.radius = radius;
        gw.isBlackHole = isBlackHole;
        gravityWells.push_back(gw);
    }

    std::shared_ptr<RigidBody> getBodyById(int id) {
        for (auto& b : bodies) {
            if (b->id == id) return b;
        }
        return nullptr;
    }

    void startDrag(float x, float y) {
        Vector2 mousePos(x, y);
        for (auto it = bodies.rbegin(); it != bodies.rend(); ++it) {
            auto& b = *it;
            if (b->bodyType == BODY_STATIC) continue;

            if (b->shapeType == SHAPE_CIRCLE) {
                if (Vector2::distance(b->position, mousePos) <= b->radius) {
                    draggedBodyId = b->id;
                    dragLocalAnchor = (mousePos - b->position).rotate(-b->angle);
                    dragTargetPos = mousePos;
                    isDragging = true;
                    return;
                }
            } else {
                if (Collision::isPointInsidePolygon(mousePos, b->worldVertices)) {
                    draggedBodyId = b->id;
                    dragLocalAnchor = (mousePos - b->position).rotate(-b->angle);
                    dragTargetPos = mousePos;
                    isDragging = true;
                    return;
                }
            }
        }
    }

    void updateDrag(float x, float y) {
        if (isDragging) {
            dragTargetPos = Vector2(x, y);
        }
    }

    void endDrag() {
        isDragging = false;
        draggedBodyId = -1;
    }

    void applyExplosion(Vector2 epicenter, float radius, float maxImpulse, float maxDamage) {
        particleSystem.emitExplosion(epicenter, 45);

        for (auto& b : bodies) {
            if (b->bodyType == BODY_STATIC) continue;

            Vector2 delta = b->position - epicenter;
            float dist = delta.length();

            if (dist < radius) {
                float falloff = 1.0f - (dist / radius);
                Vector2 dir = (dist > 0.001f) ? (delta / dist) : Vector2(0, -1);

                // Impulse blast
                float impulseMag = maxImpulse * falloff;
                b->applyImpulse(dir * impulseMag, Vector2(0, 0));

                // Damage
                float dmg = maxDamage * falloff;
                b->takeDamage(dmg);

                particleSystem.emitSparks(b->position, dir, 8);
            }
        }

        // Snap constraints near blast
        for (auto& c : constraints) {
            Vector2 pA = c->getWorldAnchorA();
            Vector2 pB = c->getWorldAnchorB();
            if (Vector2::distance(pA, epicenter) < radius * 0.7f ||
                Vector2::distance(pB, epicenter) < radius * 0.7f) {
                c->isBroken = true;
            }
        }
    }

    void step(float dt) {
        float effectiveDt = dt * timeScale;
        if (effectiveDt <= 0.0f) return;

        // Sub-stepping for ultra stability
        int subSteps = 2;
        float subDt = effectiveDt / float(subSteps);

        for (int stepIdx = 0; stepIdx < subSteps; ++stepIdx) {
            stepInternal(subDt);
        }

        particleSystem.update(effectiveDt, gravity);
    }

private:
    void stepInternal(float dt) {
        // 1. Apply gravity wells / planetary attraction
        for (auto& b : bodies) {
            if (b->bodyType != BODY_DYNAMIC) continue;

            for (const auto& gw : gravityWells) {
                Vector2 delta = gw.position - b->position;
                float distSq = delta.lengthSq();
                float minDist = 20.0f;
                if (distSq < minDist * minDist) distSq = minDist * minDist;

                float dist = std::sqrt(distSq);
                if (dist < gw.radius) {
                    Vector2 dir = delta / dist;
                    float forceMag = (gw.strength * 10000.0f * b->mass) / distSq;
                    b->applyForce(dir * forceMag);

                    if (gw.isBlackHole && dist < 35.0f) {
                        b->takeDamage(200.0f); // Shred in singularity
                    }
                }
            }
        }

        // 2. Mouse dragging spring force
        if (isDragging && draggedBodyId > 0) {
            auto b = getBodyById(draggedBodyId);
            if (b && b->bodyType == BODY_DYNAMIC) {
                Vector2 anchorWorld = b->position + dragLocalAnchor.rotate(b->angle);
                Vector2 delta = dragTargetPos - anchorWorld;
                Vector2 dragForce = delta * 600.0f - b->velocity * 25.0f;
                b->applyForceAtPoint(dragForce, anchorWorld);
            }
        }

        // 3. Integrate velocities & candidate positions
        for (auto& b : bodies) {
            b->integrate(dt, gravity, airResistance);
        }

        // 4. Solve constraints
        for (auto& c : constraints) {
            c->solve(dt);
        }

        // 5. Broadphase & Narrowphase collision detection
        std::vector<Manifold> manifolds;
        totalCollisionsLastStep = 0;

        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                auto& a = bodies[i];
                auto& b = bodies[j];

                // Skip static-static
                if (a->bodyType == BODY_STATIC && b->bodyType == BODY_STATIC) continue;
                if (a->isDead || b->isDead) continue;

                Manifold m;
                if (Collision::checkCollision(a.get(), b.get(), m)) {
                    manifolds.push_back(m);
                    totalCollisionsLastStep++;

                    // Visual sparks & sound trigger on high impact
                    Vector2 relVel = b->velocity - a->velocity;
                    float impactSpeed = relVel.length();
                    if (impactSpeed > 220.0f) {
                        Vector2 contactPt = m.contacts.empty() ? (a->position + b->position) * 0.5f : m.contacts[0].point;
                        particleSystem.emitSparks(contactPt, m.normal, 4);

                        // Impact damage
                        float damage = (impactSpeed - 200.0f) * 0.15f;
                        if (a->material == MAT_GLASS || a->material == MAT_TNT || a->material == MAT_TARGET) a->takeDamage(damage * 2.0f);
                        if (b->material == MAT_GLASS || b->material == MAT_TNT || b->material == MAT_TARGET) b->takeDamage(damage * 2.0f);
                    }
                }
            }
        }

        // 6. Sequential Impulses: Velocity Resolution Iterations
        for (int iter = 0; iter < velocityIterations; ++iter) {
            for (auto& m : manifolds) {
                resolveCollisionVelocity(m);
            }
        }

        // 7. Baumgarte Positional Stabilization
        for (int iter = 0; iter < positionIterations; ++iter) {
            for (auto& m : manifolds) {
                resolveCollisionPosition(m);
            }
            for (auto& c : constraints) {
                c->solvePosition();
            }
        }

        // 8. Handle broken TNT explosions & dead body cleanup
        std::vector<Vector2> pendingExplosions;
        for (auto& b : bodies) {
            if (b->isExploded) {
                pendingExplosions.push_back(b->position);
                b->isExploded = false;
            }
        }

        for (const auto& explPos : pendingExplosions) {
            applyExplosion(explPos, 160.0f, 1800.0f, 150.0f);
        }

        // Remove dead / broken bodies
        for (size_t i = 0; i < bodies.size();) {
            if (bodies[i]->isDead && bodies[i]->bodyType != BODY_STATIC) {
                particleSystem.emitDebris(bodies[i]->position, bodies[i]->color, 12);
                bodies.erase(bodies.begin() + i);
            } else {
                ++i;
            }
        }

        // Remove broken constraints
        for (size_t i = 0; i < constraints.size();) {
            if (constraints[i]->isBroken) {
                constraints.erase(constraints.begin() + i);
            } else {
                ++i;
            }
        }
    }

    void resolveCollisionVelocity(Manifold& m) {
        RigidBody* a = m.bodyA;
        RigidBody* b = m.bodyB;

        for (auto& contact : m.contacts) {
            Vector2 rA = contact.point - a->position;
            Vector2 rB = contact.point - b->position;

            // Velocity at contact points
            Vector2 vA = a->velocity + Vector2(-a->angularVelocity * rA.y, a->angularVelocity * rA.x);
            Vector2 vB = b->velocity + Vector2(-b->angularVelocity * rB.y, b->angularVelocity * rB.x);
            Vector2 relVel = vB - vA;

            // Normal impulse
            float contactVel = relVel.dot(m.normal);
            if (contactVel > 0.0f) continue; // Separating

            float rACrossN = rA.cross(m.normal);
            float rBCrossN = rB.cross(m.normal);
            float invMassSum = a->invMass + b->invMass +
                               (rACrossN * rACrossN) * a->invInertia +
                               (rBCrossN * rBCrossN) * b->invInertia;

            if (invMassSum <= 0.00001f) continue;

            float j = -(1.0f + m.restitution) * contactVel / invMassSum;
            j /= float(m.contacts.size());

            Vector2 impulse = m.normal * j;
            a->applyImpulse(-impulse, rA);
            b->applyImpulse(impulse, rB);

            // Friction impulse (Tangent)
            Vector2 tangent = relVel - (m.normal * contactVel);
            float tanLen = tangent.length();
            if (tanLen > 0.0001f) {
                tangent = tangent / tanLen;
                float rACrossT = rA.cross(tangent);
                float rBCrossT = rB.cross(tangent);
                float invMassSumT = a->invMass + b->invMass +
                                    (rACrossT * rACrossT) * a->invInertia +
                                    (rBCrossT * rBCrossT) * b->invInertia;

                if (invMassSumT > 0.00001f) {
                    float jt = -relVel.dot(tangent) / invMassSumT;
                    jt /= float(m.contacts.size());

                    // Coulomb's Law clamp
                    float maxFriction = j * m.dynamicFriction;
                    jt = std::clamp(jt, -maxFriction, maxFriction);

                    Vector2 frictionImpulse = tangent * jt;
                    a->applyImpulse(-frictionImpulse, rA);
                    b->applyImpulse(frictionImpulse, rB);
                }
            }
        }
    }

    void resolveCollisionPosition(Manifold& m) {
        RigidBody* a = m.bodyA;
        RigidBody* b = m.bodyB;

        const float slop = 0.5f;
        const float percent = 0.4f;

        for (auto& contact : m.contacts) {
            float correctionMag = std::max(contact.penetration - slop, 0.0f) * percent;
            float totalInvMass = a->invMass + b->invMass;
            if (totalInvMass <= 0.00001f) continue;

            Vector2 correction = m.normal * (correctionMag / totalInvMass);

            if (a->bodyType == BODY_DYNAMIC) {
                a->position -= correction * a->invMass;
                a->updateTransform();
            }
            if (b->bodyType == BODY_DYNAMIC) {
                b->position += correction * b->invMass;
                b->updateTransform();
            }
        }
    }

public:
    void clear() {
        bodies.clear();
        constraints.clear();
        gravityWells.clear();
        particleSystem.clear();
        nextBodyId = 1;
        nextConstraintId = 1;
        isDragging = false;
        draggedBodyId = -1;
    }
};

#endif // PHYSICSWORLD_H
