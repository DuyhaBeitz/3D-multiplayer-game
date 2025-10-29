#pragma once

#include <raylib.h>
#include <raymath.h>
#include "Rendering.hpp"
#include <vector>
#include <stdint.h>
#include <variant>
#include <iostream>

#include "Serialization.hpp"

constexpr float gravity = 220;
//constexpr float floor_lvl = 0;
constexpr float hor_speed = 160;
constexpr float jump_impulse = 120;
/*****************************************/
struct SphereData {
    float radius = 1.0f;
    Vector3 center{};

    template <class Archive>
    void serialize(Archive& ar) {
        ar(radius, center);
    }
};

struct BoxData {
    Vector3 half_extents{};
    Vector3 center{};
    Vector3 Min() const {
        return center - half_extents;
    }
    Vector3 Max() const {
        return center + half_extents;
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(half_extents, center);
    }
};
/*****************************************/
struct CollisionShape {
    std::variant<SphereData, BoxData> shape;

    CollisionShape() = default;
    CollisionShape(const SphereData& s) : shape(s) {}
    CollisionShape(const BoxData& b) : shape(b) {}

    bool IsSphere() const { return std::holds_alternative<SphereData>(shape); }
    bool IsBox() const { return std::holds_alternative<BoxData>(shape); }

    const SphereData* AsSphere() const { return std::get_if<SphereData>(&shape); }
    const BoxData* AsBox() const { return std::get_if<BoxData>(&shape); }

    SphereData* AsSphere() { return std::get_if<SphereData>(&shape); }
    BoxData* AsBox() { return std::get_if<BoxData>(&shape); }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(shape);
    }
};

struct CollisionResult {
    Vector3 normal{}; // from B to A (inwards)
    float penetration = -1;

    // for solving BA case when AB is implemented
    void Flip() {
        normal = Vector3Negate(normal);
    }
};

CollisionResult CollideSphereSphere(const SphereData& a, const SphereData& b);
CollisionResult CollideSphereBox(const SphereData& s, const BoxData& b);
CollisionResult CollideBoxBox(const BoxData& a, const BoxData& b);

inline CollisionResult Collide(const CollisionShape& a, const CollisionShape& b) {
    return std::visit([&](auto&& lhs, auto&& rhs) -> CollisionResult {
        using L = std::decay_t<decltype(lhs)>;
        using R = std::decay_t<decltype(rhs)>;

        if constexpr (std::is_same_v<L, SphereData> && std::is_same_v<R, SphereData>) {
            return CollideSphereSphere(lhs, rhs);
        } else if constexpr (std::is_same_v<L, SphereData> && std::is_same_v<R, BoxData>) {
            return CollideSphereBox(lhs, rhs);
        } else if constexpr (std::is_same_v<L, BoxData> && std::is_same_v<R, SphereData>) {
            auto res = CollideSphereBox(rhs, lhs);
            res.Flip();
            return res;
        } else if constexpr (std::is_same_v<L, BoxData> && std::is_same_v<R, BoxData>) {
            return CollideBoxBox(lhs, rhs);
        } else {
            return CollisionResult{};
        }
    }, a.shape, b.shape);
}
/*****************************************/
struct BodyData {
    Vector3 position = {};
    Vector3 velocity = {};
    Vector3 acceleration = {};
    bool on_ground = true;

    // inverse so that we can have infinite mass, and connot have zero mass
    float inverse_mass = 1;
    float restitution = 0;
    std::vector<CollisionShape> shapes; 

    void UpdateShapePositions() {
        for (CollisionShape& shape : shapes) {
            if (shape.IsSphere()) shape.AsSphere()->center = position;
            else if (shape.IsBox()) shape.AsBox()->center = position;
        }
    }

    void ApplyForce(Vector3 force){
        acceleration += force * inverse_mass;
    }

    void ApplyImulse(Vector3 impulse){
        velocity += impulse * inverse_mass;
        if (Vector3DotProduct(impulse, {0, 1, 0})/Vector3Length(impulse) > 0.5) {
            on_ground = true;
        }        
    }

    void Update(float delta_time) {
        Vector3 originalVelocity = velocity;
        velocity += acceleration * delta_time;
        Vector3 averageVelocity = (originalVelocity + velocity) * 0.5f;
        position += averageVelocity * delta_time;
        acceleration = {};
        UpdateShapePositions();
    }

    CollisionResult CollideWith(const BodyData& other) {
        CollisionResult max_res;
        float max_penetration = 0.0f;

        for (const CollisionShape& shapeA : shapes) {
            for (const CollisionShape& shapeB : other.shapes) {
                CollisionResult res = Collide(shapeA, shapeB);
                if (res.penetration > max_penetration) {
                    max_penetration = res.penetration;
                    max_res = res;
                }
            }
        }
        return max_res; 
    }

    void DrawShapes() const {
        // for (const CollisionShape& shape : shapes) {
        //     if (shape.IsSphere()) shape.AsSphere()->Draw();
        //     else if (shape.IsBox()) shape.AsBox()->Draw();
        // }
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(position, velocity, acceleration, on_ground, inverse_mass, restitution, shapes);
    }
};

void SolveCollision(BodyData& bA, BodyData& bB, const CollisionResult& collision_result);