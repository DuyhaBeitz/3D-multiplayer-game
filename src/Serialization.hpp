#pragma once

#include <nlohmann/json.hpp>
#include "World.hpp"

inline nlohmann::json SerializeVector3(const Vector3& v) {
    return {v.x, v.y, v.z};
}

inline nlohmann::json SerializeData(const SphereData& s) {
    return {
        {"rel_center", SerializeVector3(s.rel_center)},
        {"radius", s.radius}
    };
}

inline nlohmann::json SerializeData(const BoxData& b) {
    return {
        {"rel_center", SerializeVector3(b.rel_center)},
        {"half_extents", SerializeVector3(b.half_extents)}
    };
}

inline nlohmann::json SerializeData(const CollisionShape& c) {
    nlohmann::json j;
    j["type"] = static_cast<int>(c.collision_type);
    switch (c.collision_type) {
        case CollisionType::Sphere: j["sphere"] = SerializeData(c.sphere); break;
        case CollisionType::Box:    j["box"]    = SerializeData(c.box);    break;
    }
    return j;
}

inline nlohmann::json SerializeData(const BodyData& b) {
    nlohmann::json j;
    j["pos"] = SerializeVector3(b.position);
    j["vel"] = SerializeVector3(b.velocity);

    j["collisions"] = nlohmann::json::array();
    for (const auto& c : b.collisions) {
        j["collisions"].push_back(SerializeData(c));
    }
    return j;
}

inline nlohmann::json SerializeData(const ActorData& a) {
    return {
        {"body", SerializeData(a.body)},
        {"yaw", a.yaw},
        {"pitch", a.pitch},
        {"model_key", a.model_key}
    };
}

inline nlohmann::json SerializeData(const WorldData& w) {
    nlohmann::json j;
    j["actors"] = nlohmann::json::array();
    for (const auto& [key, a] : w.actors) {
        j["actors"].push_back({key, SerializeData(a)});
    }
    return j;
}