#ifndef COLLISION_H
#define COLLISION_H

#include "RigidBody.h"
#include "Vector2.h"
#include <algorithm>
#include <vector>
#include <limits>

struct Contact {
    Vector2 point;
    float penetration;
    float normalImpulse;
    float tangentImpulse;
    Vector2 rA; // vector from bodyA center to contact
    Vector2 rB; // vector from bodyB center to contact
};

struct Manifold {
    RigidBody* bodyA;
    RigidBody* bodyB;
    Vector2 normal; // Points from A to B
    std::vector<Contact> contacts;
    float restitution;
    float staticFriction;
    float dynamicFriction;
    bool isColliding;

    Manifold() : bodyA(nullptr), bodyB(nullptr), normal(0, 0),
                 restitution(0.2f), staticFriction(0.4f), dynamicFriction(0.3f),
                 isColliding(false) {}
};

class Collision {
public:
    static bool checkCollision(RigidBody* a, RigidBody* b, Manifold& m) {
        m.bodyA = a;
        m.bodyB = b;
        m.contacts.clear();
        m.isColliding = false;

        // AABB early exit
        if (!a->getAABB().overlaps(b->getAABB())) {
            return false;
        }

        m.restitution = std::min(a->restitution, b->restitution);
        m.staticFriction = std::sqrt(a->friction * b->friction);
        m.dynamicFriction = m.staticFriction * 0.8f;

        if (a->shapeType == SHAPE_CIRCLE && b->shapeType == SHAPE_CIRCLE) {
            return circleVsCircle(a, b, m);
        } else if (a->shapeType == SHAPE_CIRCLE && (b->shapeType == SHAPE_BOX || b->shapeType == SHAPE_POLYGON)) {
            return circleVsPolygon(a, b, m);
        } else if ((a->shapeType == SHAPE_BOX || a->shapeType == SHAPE_POLYGON) && b->shapeType == SHAPE_CIRCLE) {
            bool hit = circleVsPolygon(b, a, m);
            if (hit) {
                m.normal = -m.normal;
                m.bodyA = a;
                m.bodyB = b;
            }
            return hit;
        } else {
            return polygonVsPolygon(a, b, m);
        }
    }

    static bool isPointInsidePolygon(const Vector2& pt, const std::vector<Vector2>& verts) {
        bool inside = false;
        size_t j = verts.size() - 1;
        for (size_t i = 0; i < verts.size(); j = i++) {
            if (((verts[i].y > pt.y) != (verts[j].y > pt.y)) &&
                (pt.x < (verts[j].x - verts[i].x) * (pt.y - verts[i].y) / (verts[j].y - verts[i].y) + verts[i].x)) {
                inside = !inside;
            }
        }
        return inside;
    }

private:
    static bool circleVsCircle(RigidBody* a, RigidBody* b, Manifold& m) {
        Vector2 delta = b->position - a->position;
        float distSq = delta.lengthSq();
        float rSum = a->radius + b->radius;

        if (distSq >= rSum * rSum) return false;

        float dist = std::sqrt(distSq);
        m.isColliding = true;

        if (dist > 0.0001f) {
            m.normal = delta / dist;
            Contact c;
            c.penetration = rSum - dist;
            c.point = a->position + m.normal * a->radius;
            c.normalImpulse = 0.0f;
            c.tangentImpulse = 0.0f;
            m.contacts.push_back(c);
        } else {
            m.normal = Vector2(0, -1);
            Contact c;
            c.penetration = a->radius;
            c.point = a->position;
            c.normalImpulse = 0.0f;
            c.tangentImpulse = 0.0f;
            m.contacts.push_back(c);
        }

        return true;
    }

    static bool circleVsPolygon(RigidBody* circle, RigidBody* poly, Manifold& m) {
        // Transform circle into polygon local space
        float cosA = std::cos(-poly->angle);
        float sinA = std::sin(-poly->angle);
        Vector2 relPos = circle->position - poly->position;
        Vector2 localCirclePos(
            relPos.x * cosA - relPos.y * sinA,
            relPos.x * sinA + relPos.y * cosA
        );

        float hw = poly->width * 0.5f;
        float hh = poly->height * 0.5f;

        // Closest point on box
        Vector2 closestLocal(
            std::clamp(localCirclePos.x, -hw, hw),
            std::clamp(localCirclePos.y, -hh, hh)
        );

        bool inside = false;
        if (localCirclePos.x == closestLocal.x && localCirclePos.y == closestLocal.y) {
            inside = true;
            // Find closest axis
            float dx = hw - std::abs(localCirclePos.x);
            float dy = hh - std::abs(localCirclePos.y);
            if (dx < dy) {
                closestLocal.x = (localCirclePos.x > 0) ? hw : -hw;
            } else {
                closestLocal.y = (localCirclePos.y > 0) ? hh : -hh;
            }
        }

        Vector2 diff = localCirclePos - closestLocal;
        float distSq = diff.lengthSq();

        if (!inside && distSq > circle->radius * circle->radius) {
            return false;
        }

        float dist = std::sqrt(distSq);
        float cosWorld = std::cos(poly->angle);
        float sinWorld = std::sin(poly->angle);

        Vector2 localNorm;
        if (inside) {
            localNorm = -diff.normalized();
            if (localNorm.lengthSq() < 0.001f) {
                localNorm = (std::abs(localCirclePos.x) > std::abs(localCirclePos.y)) ?
                    Vector2((localCirclePos.x > 0) ? 1.0f : -1.0f, 0) :
                    Vector2(0, (localCirclePos.y > 0) ? 1.0f : -1.0f);
            }
        } else {
            localNorm = (dist > 0.0001f) ? diff / dist : Vector2(0, 1);
        }

        Vector2 worldNorm(
            localNorm.x * cosWorld - localNorm.y * sinWorld,
            localNorm.x * sinWorld + localNorm.y * cosWorld
        );

        Vector2 closestWorld(
            poly->position.x + (closestLocal.x * cosWorld - closestLocal.y * sinWorld),
            poly->position.y + (closestLocal.x * sinWorld + closestLocal.y * cosWorld)
        );

        m.isColliding = true;
        m.normal = -worldNorm; // Normal pointing from circle to polygon

        Contact c;
        c.penetration = inside ? (circle->radius + dist) : (circle->radius - dist);
        c.point = closestWorld;
        c.normalImpulse = 0.0f;
        c.tangentImpulse = 0.0f;
        m.contacts.push_back(c);

        return true;
    }

    static void projectPolygon(const std::vector<Vector2>& verts, const Vector2& axis, float& minP, float& maxP) {
        minP = axis.dot(verts[0]);
        maxP = minP;
        for (size_t i = 1; i < verts.size(); ++i) {
            float p = axis.dot(verts[i]);
            if (p < minP) minP = p;
            if (p > maxP) maxP = p;
        }
    }

    static bool polygonVsPolygon(RigidBody* a, RigidBody* b, Manifold& m) {
        float minOverlap = std::numeric_limits<float>::max();
        Vector2 smallestAxis(0, 0);

        // Test normals of Body A
        for (const auto& axis : a->normals) {
            float minA, maxA, minB, maxB;
            projectPolygon(a->worldVertices, axis, minA, maxA);
            projectPolygon(b->worldVertices, axis, minB, maxB);

            if (maxA < minB || maxB < minA) return false;

            float overlap = std::min(maxA - minB, maxB - minA);
            if (overlap < minOverlap) {
                minOverlap = overlap;
                smallestAxis = axis;
            }
        }

        // Test normals of Body B
        for (const auto& axis : b->normals) {
            float minA, maxA, minB, maxB;
            projectPolygon(a->worldVertices, axis, minA, maxA);
            projectPolygon(b->worldVertices, axis, minB, maxB);

            if (maxA < minB || maxB < minA) return false;

            float overlap = std::min(maxA - minB, maxB - minA);
            if (overlap < minOverlap) {
                minOverlap = overlap;
                smallestAxis = axis;
            }
        }

        // Make sure normal points from A to B
        Vector2 d = b->position - a->position;
        if (d.dot(smallestAxis) < 0.0f) {
            smallestAxis = -smallestAxis;
        }

        m.isColliding = true;
        m.normal = smallestAxis;

        // Clip vertices to find contact points
        findPolygonContacts(a, b, smallestAxis, m.contacts, minOverlap);

        return true;
    }

    static void findPolygonContacts(RigidBody* a, RigidBody* b, const Vector2& normal, std::vector<Contact>& contacts, float overlap) {
        // Collect vertices of B that penetrate A
        for (const auto& vert : b->worldVertices) {
            if (isPointInsidePolygon(vert, a->worldVertices)) {
                Contact c;
                c.point = vert;
                c.penetration = overlap;
                c.normalImpulse = 0.0f;
                c.tangentImpulse = 0.0f;
                contacts.push_back(c);
            }
        }

        // Collect vertices of A that penetrate B
        for (const auto& vert : a->worldVertices) {
            if (isPointInsidePolygon(vert, b->worldVertices)) {
                Contact c;
                c.point = vert;
                c.penetration = overlap;
                c.normalImpulse = 0.0f;
                c.tangentImpulse = 0.0f;
                contacts.push_back(c);
            }
        }

        // Fallback midpoint if vertices are along flat edges
        if (contacts.empty()) {
            Contact c;
            c.point = (a->position + b->position) * 0.5f;
            c.penetration = overlap;
            c.normalImpulse = 0.0f;
            c.tangentImpulse = 0.0f;
            contacts.push_back(c);
        }
    }
};

#endif // COLLISION_H
