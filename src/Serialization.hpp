#pragma once

#include <nlohmann/json.hpp>
#include "World.hpp"

inline nlohmann::json SerializeVector3(const Vector3& v) {
    return {v.x, v.y, v.z};
}

inline nlohmann::json SerializeSphere(const SphereData& s) {
    return {
        {"rel_center", SerializeVector3(s.rel_center)},
        {"radius", s.radius}
    };
}

inline nlohmann::json SerializeBox(const BoxData& b) {
    return {
        {"rel_center", SerializeVector3(b.rel_center)},
        {"half_extents", SerializeVector3(b.half_extents)}
    };
}

inline nlohmann::json SerializeShape(const CollisionShape& shape) {
    nlohmann::json j;
    std::visit([&](auto&& s) {
        using T = std::decay_t<decltype(s)>;
        if constexpr (std::is_same_v<T, SphereData>) {
            j["type"] = "sphere";
            j["sphere"] = SerializeSphere(s);
        } else if constexpr (std::is_same_v<T, BoxData>) {
            j["type"] = "box";
            j["box"] = SerializeBox(s);
        }
    }, shape.shape);
    return j;
}

inline nlohmann::json SerializeBody(const BodyData& b) {
    nlohmann::json j;
    j["pos"] = SerializeVector3(b.position);
    j["vel"] = SerializeVector3(b.velocity);

    j["shapes"] = nlohmann::json::array();
    for (const auto& c : b.collisions) {
        j["shapes"].push_back(SerializeShape(c));
    }
    return j;
}

inline nlohmann::json SerializeActor(const ActorData& a) {
    return {
        {"body", SerializeBody(a.body)},
        {"yaw", a.yaw},
        {"pitch", a.pitch},
        {"model_key", a.model_key}
    };
}

inline nlohmann::json SerializeWorld(const WorldData& w) {
    nlohmann::json j;
    j["actors"] = nlohmann::json::array();
    for (const auto& [key, a] : w.actors) {
        j["actors"].push_back({key, SerializeActor(a)});
    }
    return j;
}