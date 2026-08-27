#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "RigidBody.h"
#include "Constraint.h"
#include "ParticleSystem.h"
#include <box2d/box2d.h>
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

class PhysicsWorld;

class GameContactListener : public b2ContactListener {
public:
    PhysicsWorld* world;

    GameContactListener(PhysicsWorld* w) : world(w) {}

    void BeginContact(b2Contact* contact) override;
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;
};

class PhysicsWorld {
public:
    static constexpr float PPM = 30.0f; // 30 pixels per 1 Box2D meter

    std::unique_ptr<b2World> b2world;
    std::unique_ptr<GameContactListener> contactListener;

    std::vector<std::shared_ptr<RigidBody>> bodies;
    std::vector<std::shared_ptr<Constraint>> constraints;
    std::vector<ScorePopup> scorePopups;
    ParticleSystem particleSystem;

    Vector2 gravity;
    float timeScale;

    int nextBodyId;
    int nextConstraintId;

    int totalScore;
    int targetsRemaining;
    int totalTargets;

    PhysicsWorld()
        : gravity(0.0f, 660.0f), timeScale(1.0f),
          nextBodyId(1), nextConstraintId(1),
          totalScore(0), targetsRemaining(0), totalTargets(0)
    {
        initB2World();
    }

    void initB2World() {
        b2Vec2 b2Gravity(0.0f, gravity.y / PPM); // ~22.0 m/s^2 downwards
        b2world = std::make_unique<b2World>(b2Gravity);
        contactListener = std::make_unique<GameContactListener>(this);
        b2world->SetContactListener(contactListener.get());
    }

    void clear() {
        bodies.clear();
        constraints.clear();
        scorePopups.clear();
        particleSystem.particles.clear();
        nextBodyId = 1;
        nextConstraintId = 1;
        targetsRemaining = 0;
        totalTargets = 0;

        // Recreate clean Box2D world
        initB2World();
    }

    std::shared_ptr<RigidBody> createCircle(BodyType bType, MaterialType mat, Vector2 pos, float radius) {
        auto body = std::make_shared<RigidBody>(nextBodyId++, bType, SHAPE_CIRCLE, mat, pos);
        body->setCircle(radius);

        b2BodyDef bodyDef;
        bodyDef.type = (bType == BODY_STATIC) ? b2_staticBody : (bType == BODY_KINEMATIC ? b2_kinematicBody : b2_dynamicBody);
        bodyDef.position.Set(pos.x / PPM, pos.y / PPM);
        bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(body.get());
        if (mat == MAT_PROJECTILE) {
            bodyDef.bullet = true; // Continuous collision detection for fast projectiles!
        }

        b2Body* b2_b = b2world->CreateBody(&bodyDef);

        b2CircleShape circleShape;
        circleShape.m_radius = radius / PPM;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &circleShape;
        fixtureDef.density = body->density;
        fixtureDef.friction = body->friction;
        fixtureDef.restitution = body->restitution;

        b2Fixture* b2_f = b2_b->CreateFixture(&fixtureDef);

        body->b2_body = b2_b;
        body->b2_fixture = b2_f;
        body->syncFromB2(PPM);

        bodies.push_back(body);
        return body;
    }

    std::shared_ptr<RigidBody> createBox(BodyType bType, MaterialType mat, Vector2 pos, float width, float height, float angle = 0.0f) {
        auto body = std::make_shared<RigidBody>(nextBodyId++, bType, SHAPE_BOX, mat, pos);
        body->angle = angle;
        body->setBox(width, height);

        b2BodyDef bodyDef;
        bodyDef.type = (bType == BODY_STATIC) ? b2_staticBody : (bType == BODY_KINEMATIC ? b2_kinematicBody : b2_dynamicBody);
        bodyDef.position.Set(pos.x / PPM, pos.y / PPM);
        bodyDef.angle = angle;
        bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(body.get());

        b2Body* b2_b = b2world->CreateBody(&bodyDef);

        b2PolygonShape boxShape;
        boxShape.SetAsBox((width * 0.5f) / PPM, (height * 0.5f) / PPM);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = body->density;
        fixtureDef.friction = body->friction;
        fixtureDef.restitution = body->restitution;

        b2Fixture* b2_f = b2_b->CreateFixture(&fixtureDef);

        body->b2_body = b2_b;
        body->b2_fixture = b2_f;
        body->syncFromB2(PPM);

        bodies.push_back(body);
        return body;
    }

    std::shared_ptr<Constraint> createConstraint(ConstraintType type, std::shared_ptr<RigidBody> a, std::shared_ptr<RigidBody> b,
                                                 Vector2 localA, Vector2 localB, float length = -1.0f) {
        auto c = std::make_shared<Constraint>(nextConstraintId++, type, a.get(), b.get(), localA, localB, length);

        if (a && b && a->b2_body && b->b2_body) {
            b2Vec2 anchorA(localA.x / PPM, localA.y / PPM);
            b2Vec2 anchorB(localB.x / PPM, localB.y / PPM);

            if (type == CONSTRAINT_ROD || type == CONSTRAINT_SPRING || type == CONSTRAINT_ROPE) {
                b2DistanceJointDef jd;
                b2Vec2 worldA = a->b2_body->GetWorldPoint(anchorA);
                b2Vec2 worldB = b->b2_body->GetWorldPoint(anchorB);
                jd.Initialize(a->b2_body, b->b2_body, worldA, worldB);
                if (length > 0.0f) {
                    jd.length = length / PPM;
                }
                if (type == CONSTRAINT_ROD) {
                    jd.minLength = jd.length;
                    jd.maxLength = jd.length;
                } else if (type == CONSTRAINT_ROPE) {
                    jd.minLength = 0.0f;
                    jd.maxLength = jd.length;
                } else if (type == CONSTRAINT_SPRING) {
                    b2LinearStiffness(jd.stiffness, jd.damping, 4.0f, 0.4f, a->b2_body, b->b2_body);
                }
                c->b2_joint = b2world->CreateJoint(&jd);
            }
        }

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

        b2Vec2 epicMeters(epicenter.x / PPM, epicenter.y / PPM);
        float radiusMeters = radius / PPM;

        for (auto& b : bodies) {
            if (b->isDead || !b->b2_body || b->bodyType == BODY_STATIC) continue;

            b2Vec2 bPos = b->b2_body->GetPosition();
            b2Vec2 delta = bPos - epicMeters;
            float dist = delta.Length();

            if (dist < radiusMeters) {
                float factor = 1.0f - (dist / radiusMeters);
                b2Vec2 dir = (dist > 0.001f) ? (1.0f / dist) * delta : b2Vec2(0.0f, -1.0f);

                // Blast impulse & upwards loft in Box2D units
                float impulseMag = (maxForce / PPM) * factor;
                b2Vec2 blastImpulse = impulseMag * dir + b2Vec2(0.0f, -impulseMag * 0.4f);

                b->b2_body->ApplyLinearImpulse(blastImpulse, b->b2_body->GetWorldCenter(), true);
                b->takeDamage(damage * factor);
            }
        }
    }

    void triggerTNT(RigidBody* tntBody) {
        if (!tntBody || tntBody->isExploded) return;
        tntBody->isExploded = true;
        tntBody->isDead = true;
        totalScore += tntBody->scoreValue;
        applyExplosion(tntBody->position, 240.0f, 3800.0f, 300.0f);
    }

    void step(float dt) {
        float scaledDt = dt * timeScale;
        if (scaledDt <= 0.0f) return;

        // Step Box2D world simulation (standard 8 velocity, 3 position iterations)
        b2world->Step(scaledDt, 8, 3);

        // Synchronize all rigid body positions & physics states from Box2D
        for (auto& b : bodies) {
            if (b->b2_body) {
                b->syncFromB2(PPM);
            }

            if (b->damageFlash > 0.0f) {
                b->damageFlash -= scaledDt;
            }

            // Projectile fuse countdown for Bomb bird
            if (b->isProjectile && !b->isDead) {
                b->flightTime += scaledDt;
                if (b->fuseTimer > 0.0f) {
                    b->fuseTimer -= scaledDt;
                    particleSystem.emit(b->position + Vector2(0, -15), Vector2(0, -30), 0.3f, 3.0f, 0xFF9800, 0);
                    if (b->fuseTimer <= 0.0f) {
                        applyExplosion(b->position, 240.0f, 3600.0f, 350.0f);
                        b->isDead = true;
                    }
                }
            }
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

                // Safely remove or disable Box2D body
                if (b->b2_body) {
                    b2world->DestroyBody(b->b2_body);
                    b->b2_body = nullptr;
                    b->b2_fixture = nullptr;
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
};

inline void GameContactListener::BeginContact(b2Contact* contact) {
    b2Fixture* fA = contact->GetFixtureA();
    b2Fixture* fB = contact->GetFixtureB();
    if (!fA || !fB) return;

    RigidBody* rbA = reinterpret_cast<RigidBody*>(fA->GetBody()->GetUserData().pointer);
    RigidBody* rbB = reinterpret_cast<RigidBody*>(fB->GetBody()->GetUserData().pointer);

    if (rbA && rbB) {
        if (rbA->isProjectile && !rbA->isArmed) rbA->isArmed = true;
        if (rbB->isProjectile && !rbB->isArmed) rbB->isArmed = true;
    }
}

inline void GameContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    if (!impulse || impulse->count == 0) return;

    b2Fixture* fA = contact->GetFixtureA();
    b2Fixture* fB = contact->GetFixtureB();
    if (!fA || !fB) return;

    RigidBody* rbA = reinterpret_cast<RigidBody*>(fA->GetBody()->GetUserData().pointer);
    RigidBody* rbB = reinterpret_cast<RigidBody*>(fB->GetBody()->GetUserData().pointer);
    if (!rbA || !rbB) return;

    float maxImpulse = 0.0f;
    for (int i = 0; i < impulse->count; ++i) {
        maxImpulse = std::max(maxImpulse, impulse->normalImpulses[i]);
    }

    // Impact damage threshold: Ignore resting static contact (< 4.5 N*s)
    if (maxImpulse > 4.5f) {
        float effectiveImpulse = maxImpulse - 4.0f;
        float dmgA = effectiveImpulse * 14.0f * (rbB->mass / (rbA->mass + rbB->mass + 0.1f));
        float dmgB = effectiveImpulse * 14.0f * (rbA->mass / (rbA->mass + rbB->mass + 0.1f));

        // Drill Ability Boost
        if (rbA->isProjectile && rbA->projectileType == 3 && rbA->abilityUsed) dmgB *= 3.5f;
        if (rbB->isProjectile && rbB->projectileType == 3 && rbB->abilityUsed) dmgA *= 3.5f;

        // Bombardier Impact Fuse
        if (rbA->isProjectile && rbA->projectileType == 1 && rbA->fuseTimer < 0.0f) rbA->fuseTimer = 1.2f;
        if (rbB->isProjectile && rbB->projectileType == 1 && rbB->fuseTimer < 0.0f) rbB->fuseTimer = 1.2f;

        // Damage Bodies
        if (rbA->bodyType == BODY_DYNAMIC) rbA->takeDamage(dmgA);
        if (rbB->bodyType == BODY_DYNAMIC) rbB->takeDamage(dmgB);

        // Trigger TNT on heavy impact
        if (rbA->material == MAT_TNT && maxImpulse > 8.0f) world->triggerTNT(rbA);
        if (rbB->material == MAT_TNT && maxImpulse > 8.0f) world->triggerTNT(rbB);

        // Impact spark particles
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);
        Vector2 contactPt(worldManifold.points[0].x * PhysicsWorld::PPM, worldManifold.points[0].y * PhysicsWorld::PPM);
        world->particleSystem.emit(contactPt, Vector2(0, -30), 0.25f, 2.5f, 0xFFEB3B, 0);
    }
}

#endif // PHYSICSWORLD_H
