#ifndef VECTOR2_H
#define VECTOR2_H

#include <cmath>
#include <algorithm>

struct Vector2 {
    float x;
    float y;

    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}

    Vector2 operator+(const Vector2& v) const { return Vector2(x + v.x, y + v.y); }
    Vector2 operator-(const Vector2& v) const { return Vector2(x - v.x, y - v.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const { return (s != 0.0f) ? Vector2(x / s, y / s) : Vector2(0.0f, 0.0f); }
    
    Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
    Vector2& operator-=(const Vector2& v) { x -= v.x; y -= v.y; return *this; }
    Vector2& operator*=(float s) { x *= s; y *= s; return *this; }
    Vector2& operator/=(float s) { if (s != 0.0f) { x /= s; y /= s; } return *this; }
    Vector2 operator-() const { return Vector2(-x, -y); }

    float lengthSq() const { return x * x + y * y; }
    float length() const { return std::sqrt(lengthSq()); }

    Vector2 normalized() const {
        float len = length();
        return (len > 0.00001f) ? Vector2(x / len, y / len) : Vector2(0.0f, 0.0f);
    }

    float dot(const Vector2& v) const { return x * v.x + y * v.y; }
    float cross(const Vector2& v) const { return x * v.y - y * v.x; }

    Vector2 perpendicular() const { return Vector2(-y, x); }
    Vector2 rotate(float radians) const {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return Vector2(x * c - y * s, x * s + y * c);
    }

    static float distance(const Vector2& a, const Vector2& b) {
        return (a - b).length();
    }

    static float distanceSq(const Vector2& a, const Vector2& b) {
        return (a - b).lengthSq();
    }

    static Vector2 lerp(const Vector2& a, const Vector2& b, float t) {
        return a + (b - a) * t;
    }
};

inline Vector2 operator*(float s, const Vector2& v) {
    return Vector2(v.x * s, v.y * s);
}

#endif // VECTOR2_H
