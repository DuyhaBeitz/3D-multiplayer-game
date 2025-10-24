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

enum class CollisionType : uint8_t {
    Sphere,
    Box
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
    CollisionType collision_type;

    union {
        SphereData sphere;
        BoxData box;
    };

    CollisionShape(const SphereData& s) : collision_type(CollisionType::Sphere), sphere(s) {}
    CollisionShape(const BoxData& b) : collision_type(CollisionType::Box), box(b) {}
};

struct BodyData {
    Vector3 position = {};
    Vector3 velocity = {};
    std::vector<CollisionShape> collisions;
};