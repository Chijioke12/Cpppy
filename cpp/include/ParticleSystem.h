#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include "Vector2.h"
#include <vector>
#include <cstdlib>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    float life;
    float maxLife;
    float size;
    float startSize;
    unsigned int color;
    int type; // 0: spark, 1: smoke, 2: fire, 3: dust, 4: water, 5: debris
};

class ParticleSystem {
private:
    static float rand01() {
        return float(std::rand()) / float(RAND_MAX);
    }
    static float randRange(float a, float b) {
        return a + rand01() * (b - a);
    }

public:
    std::vector<Particle> particles;
    size_t maxParticles;

    ParticleSystem(size_t maxCount = 600) : maxParticles(maxCount) {}

    void emit(Vector2 pos, Vector2 vel, float life, float size, unsigned int color, int type = 0) {
        if (particles.size() >= maxParticles) {
            particles.erase(particles.begin());
        }
        Particle p;
        p.position = pos;
        p.velocity = vel;
        p.life = life;
        p.maxLife = life;
        p.size = size;
        p.startSize = size;
        p.color = color;
        p.type = type;
        particles.push_back(p);
    }

    void emitExplosion(Vector2 epicenter, int count = 40) {
        for (int i = 0; i < count; ++i) {
            float angle = rand01() * 6.283185f;
            float speed = randRange(50.0f, 400.0f);
            Vector2 vel(std::cos(angle) * speed, std::sin(angle) * speed);
            float life = randRange(0.4f, 1.2f);
            float size = randRange(3.0f, 11.0f);

            unsigned int color = 0xFF5722;
            int type = 2; // Fire
            int r = std::rand() % 3;
            if (r == 0) {
                color = 0xFFEB3B; // Yellow spark
                type = 0;
            } else if (r == 1) {
                color = 0xFF3D00; // Bright orange
                type = 2;
            } else {
                color = 0x757575; // Smoke
                type = 1;
            }

            emit(epicenter, vel, life, size, color, type);
        }
    }

    void emitSparks(Vector2 pos, Vector2 normal, int count = 6) {
        for (int i = 0; i < count; ++i) {
            float spread = (rand01() - 0.5f) * 1.5f;
            Vector2 dir = normal.rotate(spread);
            float speed = randRange(80.0f, 280.0f);
            Vector2 vel = dir * speed;
            float life = randRange(0.2f, 0.5f);
            emit(pos, vel, life, 2.5f, 0xFFD54F, 0);
        }
    }

    void emitDebris(Vector2 pos, unsigned int matColor, int count = 8) {
        for (int i = 0; i < count; ++i) {
            float angle = rand01() * 6.283185f;
            float speed = randRange(30.0f, 180.0f);
            Vector2 vel(std::cos(angle) * speed, std::sin(angle) * speed);
            float life = randRange(0.6f, 1.2f);
            float size = randRange(2.0f, 6.0f);
            emit(pos, vel, life, size, matColor, 5);
        }
    }

    void update(float dt, const Vector2& gravity) {
        for (size_t i = 0; i < particles.size();) {
            Particle& p = particles[i];
            p.life -= dt;
            if (p.life <= 0.0f) {
                particles.erase(particles.begin() + i);
                continue;
            }

            // Physics on particle
            if (p.type == 0 || p.type == 3 || p.type == 5) { // Sparks / dust / debris fall
                p.velocity += gravity * dt * 0.5f;
            } else if (p.type == 1 || p.type == 2) { // Smoke / fire rises
                p.velocity.y -= 40.0f * dt;
                p.velocity.x *= 0.98f;
            }

            p.position += p.velocity * dt;
            float progress = p.life / p.maxLife;
            if (p.type == 1) {
                p.size = p.startSize * (2.0f - progress); // Smoke expands
            } else {
                p.size = p.startSize * progress; // Sparks shrink
            }

            ++i;
        }
    }

    void clear() {
        particles.clear();
    }
};

#endif // PARTICLESYSTEM_H
