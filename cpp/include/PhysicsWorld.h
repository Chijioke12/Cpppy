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
#include <string>

struct ScorePopup {
    Vector2 position;
    std::string text;
    float life;
    float maxLife;
    unsigned int color;
    float scale;
};

class PhysicsWorld {
public:
    std::vector<std::shared_ptr<RigidBody>> bodies;
    std::vector<std::shared_ptr<Constraint>> constraints;
    std::vector<ScorePopup> scorePopups;
    ParticleSystem particleSystem;

    Vector2 gravity;
    float airResistance;
    int velocityIterations;
    int positionIterations;
    float timeScale;

    int nextBodyId;
    int nextConstraintId;

    int totalScore;
    int targetsRemaining;
    int totalTargets;

    PhysicsWorld()
        : gravity(0.0f, 980.0f), airResistance(0.001f),
          velocityIterations(8), positionIterations(4), timeScale(1.0f),
          nextBodyId(1), nextConstraintId(1),
          totalScore(0), targetsRemaining(0), totalTargets(0)
    {}

    void clear() {
        bodies.clear();
        constraints.clear();
        scorePopups.clear();
        particleSystem.particles.clear();
        nextBodyId = 1;
        nextConstraintId = 1;
        targetsRemaining = 0;
        totalTargets = 0;
    }

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

    void addScorePopup(Vector2 pos, const std::string& text, unsigned int color = 0xFFEB3B, float scale = 1.8f) {
        ScorePopup sp;
        sp.position = pos;
        sp.text = text;
        sp.life = 1.4f;
        sp.maxLife = 1.4f;
        sp.color = color;
        sp.scale = scale;
        scorePopups.push_back(sp);
    }

    void applyExplosion(Vector2 epicenter, float radius, float maxForce, float damage) {
        particleSystem.emitExplosion(epicenter, 45);
        addScorePopup(epicenter + Vector2(0, -30), "BOOM!", 0xFF3D00, 2.2f);

        for (auto& b : bodies) {
            if (b->bodyType == BODY_STATIC || b->isDead) continue;

            Vector2 delta = b->position - epicenter;
            float dist = delta.length();
            if (dist < radius) {
                float factor = 1.0f - (dist / radius);
                Vector2 dir = (dist > 0.001f) ? (delta / dist) : Vector2(0, -1);
                
                // Blast impulse & upwards loft
                Vector2 blastImpulse = (dir * maxForce * factor) + Vector2(0, -maxForce * factor * 0.5f);
                b->applyImpulse(blastImpulse, Vector2(0, 0));
                b->takeDamage(damage * factor);
            }
        }
    }

    void triggerTNT(RigidBody* tntBody) {
        if (!tntBody || tntBody->isExploded) return;
        tntBody->isExploded = true;
        tntBody->isDead = true;
        totalScore += tntBody->scoreValue;
        applyExplosion(tntBody->position, 220.0f, 3200.0f, 250.0f);
    }

    void step(float dt) {
        float scaledDt = dt * timeScale;
        if (scaledDt <= 0.0f) return;

        // Substepping for rock-solid stability
        const int subSteps = 4;
        float subDt = scaledDt / (float)subSteps;

        for (int stepIdx = 0; stepIdx < subSteps; ++stepIdx) {
            stepSubStep(subDt, stepIdx == 0);
        }

        // Handle Destroyed Bodies & Targets
        for (auto& b : bodies) {
            if (b->isDead && !b->isExploded) {
                b->isExploded = true;
                if (b->isTarget && !b->isTargetEliminated) {
                    b->isTargetEliminated = true;
                    targetsRemaining--;
                    totalScore += b->scoreValue;
                    particleSystem.emitExplosion(b->position, 25);
                    particleSystem.emitDebris(b->position, 0xAEEA00, 15);
                    addScorePopup(b->position, "+5000", 0x76FF03, 2.4f);
                } else if (b->material == MAT_TNT) {
                    triggerTNT(b.get());
                } else if (b->material != MAT_PROJECTILE) {
                    totalScore += b->scoreValue;
                    particleSystem.emitDebris(b->position, b->color, 12);
                    addScorePopup(b->position, "+" + std::to_string(b->scoreValue), 0xFFD54F, 1.4f);
                }
            }
        }

        // Update Particle System & Popups
        particleSystem.update(scaledDt, gravity);

        for (size_t i = 0; i < scorePopups.size();) {
            scorePopups[i].life -= scaledDt;
            scorePopups[i].position.y -= 45.0f * scaledDt; // Drift upwards
            if (scorePopups[i].life <= 0.0f) {
                scorePopups.erase(scorePopups.begin() + i);
            } else {
                ++i;
            }
        }
    }

private:
    void stepSubStep(float subDt, bool isFirstSubStep) {
        // 1. Update Projectile Fuse Timers & Flight Times
        for (auto& b : bodies) {
            if (b->isDead) continue;
            if (isFirstSubStep && b->damageFlash > 0.0f) b->damageFlash -= subDt * 4.0f;

            if (b->isProjectile) {
                b->flightTime += subDt;

                // Bomb fuse timer
                if (b->fuseTimer > 0.0f) {
                    b->fuseTimer -= subDt;
                    particleSystem.emit(b->position + Vector2(0, -15), Vector2(0, -30), 0.3f, 3.0f, 0xFF9800, 0);
                    if (b->fuseTimer <= 0.0f) {
                        applyExplosion(b->position, 240.0f, 3600.0f, 350.0f);
                        b->isDead = true;
                    }
                }

                // Trail emission
                if (b->velocity.length() > 60.0f) {
                    if (b->projectileType == 1) { // Bomb
                        particleSystem.emit(b->position, -b->velocity * 0.1f, 0.35f, 4.0f, 0x616161, 1);
                    } else if (b->projectileType == 3 && b->abilityUsed) { // Drill rocket fire
                        particleSystem.emit(b->position, -b->velocity * 0.2f, 0.4f, 6.0f, 0x00E676, 2);
                    } else {
                        particleSystem.emit(b->position, Vector2(0, -5), 0.25f, 2.5f, 0xFFFFFF, 1);
                    }
                }
            }

            // Kinematic & Dynamic integration
            if (b->bodyType == BODY_DYNAMIC) {
                b->velocity += (gravity + b->force * b->invMass) * subDt;
                b->velocity *= (1.0f - airResistance * 0.5f);
                b->angularVelocity += (b->torque * b->invInertia) * subDt;
                b->angularVelocity *= (1.0f - airResistance * 1.5f);

                // Gentle sleep damping for resting stacks
                if (b->velocity.lengthSq() < 0.25f && std::abs(b->angularVelocity) < 0.05f) {
                    b->velocity = Vector2(0, 0);
                    b->angularVelocity = 0.0f;
                }

                b->position += b->velocity * subDt;
                b->angle += b->angularVelocity * subDt;
                b->updateWorldVertices();

                b->force = Vector2(0, 0);
                b->torque = 0.0f;
            }
        }

        // 2. Constraints Solver
        for (int iter = 0; iter < 4; ++iter) {
            for (auto& c : constraints) {
                if (!c->isActive) continue;
                c->solve();
            }
        }

        // 3. Collision Detection & Response
        std::vector<Manifold> manifolds;
        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {
                auto& a = bodies[i];
                auto& b = bodies[j];
                if (a->bodyType == BODY_STATIC && b->bodyType == BODY_STATIC) continue;
                if (a->isDead || b->isDead) continue;

                Manifold m;
                if (Collision::checkCollision(a.get(), b.get(), m)) {
                    manifolds.push_back(m);
                }
            }
        }

        // Sequential Impulse Solver
        for (int iter = 0; iter < velocityIterations; ++iter) {
            for (auto& m : manifolds) {
                RigidBody* a = m.bodyA;
                RigidBody* b = m.bodyB;
                float numContacts = std::max(1.0f, (float)m.contacts.size());

                for (auto& c : m.contacts) {
                    Vector2 rA = c.point - a->position;
                    Vector2 rB = c.point - b->position;

                    Vector2 vA = a->velocity + Vector2(-a->angularVelocity * rA.y, a->angularVelocity * rA.x);
                    Vector2 vB = b->velocity + Vector2(-b->angularVelocity * rB.y, b->angularVelocity * rB.x);
                    Vector2 relativeVel = vB - vA;

                    float contactVel = relativeVel.dot(m.normal);
                    if (contactVel > 0.0f) continue; // Moving apart

                    float rACrossN = rA.cross(m.normal);
                    float rBCrossN = rB.cross(m.normal);
                    float invMassSum = a->invMass + b->invMass + (rACrossN * rACrossN) * a->invInertia + (rBCrossN * rBCrossN) * b->invInertia;

                    if (invMassSum <= 0.00001f) continue;

                    // Restitution cutoff: Disable restitution at low velocities (< 60 px/s) to prevent resting vibration & structure collapse!
                    float effectiveRestitution = (std::abs(contactVel) > 60.0f) ? m.restitution : 0.0f;
                    float impulseMag = -(1.0f + effectiveRestitution) * contactVel / invMassSum;
                    impulseMag /= numContacts;

                    Vector2 impulse = m.normal * impulseMag;

                    a->applyImpulse(-impulse, rA);
                    b->applyImpulse(impulse, rB);

                    // Friction Impulse
                    Vector2 tangent = relativeVel - (m.normal * contactVel);
                    float tanLen = tangent.length();
                    if (tanLen > 0.001f) {
                        tangent = tangent / tanLen;
                        float rACrossT = rA.cross(tangent);
                        float rBCrossT = rB.cross(tangent);
                        float invMassSumT = a->invMass + b->invMass + (rACrossT * rACrossT) * a->invInertia + (rBCrossT * rBCrossT) * b->invInertia;
                        float jt = -relativeVel.dot(tangent) / (invMassSumT + 0.0001f);
                        jt /= numContacts;

                        float maxFriction = impulseMag * m.dynamicFriction;
                        jt = std::max(-maxFriction, std::min(maxFriction, jt));
                        Vector2 frictionImpulse = tangent * jt;

                        a->applyImpulse(-frictionImpulse, rA);
                        b->applyImpulse(frictionImpulse, rB);
                    }

                    // Calculate Collision Damage from relative velocity impact (only on true impacts, not resting contact)
                    float impactSpeed = std::abs(contactVel);
                    if (impactSpeed > 220.0f && iter == 0) {
                        float dmgA = (impactSpeed - 180.0f) * 0.4f * (b->mass / (a->mass + b->mass + 0.1f));
                        float dmgB = (impactSpeed - 180.0f) * 0.4f * (a->mass / (a->mass + b->mass + 0.1f));

                        if (a->isProjectile && a->projectileType == 3 && a->abilityUsed) {
                            dmgB *= 3.5f;
                        }
                        if (b->isProjectile && b->projectileType == 3 && b->abilityUsed) {
                            dmgA *= 3.5f;
                        }

                        a->takeDamage(dmgA);
                        b->takeDamage(dmgB);

                        // Trigger Bomb Bird fuse on first solid collision
                        if (a->isProjectile && a->projectileType == 1 && a->fuseTimer < 0.0f) {
                            a->fuseTimer = 1.2f;
                        }
                        if (b->isProjectile && b->projectileType == 1 && b->fuseTimer < 0.0f) {
                            b->fuseTimer = 1.2f;
                        }

                        // Emit sparks and debris
                        particleSystem.emitSparks(c.point, m.normal, 4);
                        if (a->material == MAT_WOOD || b->material == MAT_WOOD) {
                            particleSystem.emitDebris(c.point, 0x8D6E63, 3);
                        } else if (a->material == MAT_STONE || b->material == MAT_STONE) {
                            particleSystem.emitDebris(c.point, 0x78909C, 3);
                        } else if (a->material == MAT_GLASS || b->material == MAT_GLASS) {
                            particleSystem.emitDebris(c.point, 0x80DEEA, 5);
                        }

                        // TNT Impact Detonation
                        if (a->material == MAT_TNT && impactSpeed > 280.0f) triggerTNT(a);
                        if (b->material == MAT_TNT && impactSpeed > 280.0f) triggerTNT(b);
                    }
                }
            }
        }

        // Positional Penetration Correction (Baumgarte stabilization)
        const float percent = 0.25f;
        const float slop = 0.1f;
        for (auto& m : manifolds) {
            RigidBody* a = m.bodyA;
            RigidBody* b = m.bodyB;
            float numContacts = std::max(1.0f, (float)m.contacts.size());
            for (auto& c : m.contacts) {
                float correctionMag = std::max(c.penetration - slop, 0.0f) / (a->invMass + b->invMass + 0.0001f) * (percent / numContacts);
                Vector2 correction = m.normal * correctionMag;
                if (a->bodyType == BODY_DYNAMIC) a->position -= correction * a->invMass;
                if (b->bodyType == BODY_DYNAMIC) b->position += correction * b->invMass;
                a->updateWorldVertices();
                b->updateWorldVertices();
            }
        }
    }
};

#endif // PHYSICSWORLD_H
