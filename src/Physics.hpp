#pragma once

#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <stdint.h>

constexpr double gravity = 40;
constexpr double floor_lvl = 0;
constexpr double hor_speed = 60;
constexpr double jump_impulse = 40;

struct CollisionResult {
    Vector3 normal{};
    float penetration = -1;
};

/*****************************************/
struct SphereData {
    Vector3 rel_center{};
    float radius = 1.0f;
};

struct BoxData {
    Vector3 rel_center{};
    Vector3 half_extents{};
};
/*****************************************/
struct CollisionShape {
    std::variant<SphereData, BoxData> shape;

    CollisionShape(const SphereData& s) : shape(s) {}
    CollisionShape(const BoxData& b) : shape(b) {}

    bool IsSphere() const { return std::holds_alternative<SphereData>(shape); }
    bool IsBox() const { return std::holds_alternative<BoxData>(shape); }

    const SphereData* AsSphere() const { return std::get_if<SphereData>(&shape); }
    const BoxData* AsBox() const { return std::get_if<BoxData>(&shape); }
};

struct BodyData {
    Vector3 position = {};
    Vector3 velocity = {};
    std::vector<CollisionShape> collisions; 
};