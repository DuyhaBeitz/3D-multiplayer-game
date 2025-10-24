#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include "World.hpp"

inline Vector3 DeserializeVector3(const nlohmann::json& j) {
    if (!j.is_array() || j.size() != 3) {
        throw std::runtime_error("Vector3 must be a JSON array with exactly 3 elements");
    }
     return { j[0].get<float>(), j[1].get<float>(), j[2].get<float>() };
}

inline SphereData DeserializeSphere(const nlohmann::json& j) {
    SphereData s;
    s.rel_center = DeserializeVector3(j["rel_center"]);
    s.radius = j["radius"];
    return s;
}

inline BoxData DeserializeBox(const nlohmann::json& j) {
    if (!j.contains("rel_center") || !j.contains("half_extents")) {
        throw std::runtime_error("BoxData requires 'rel_center' and 'half_extents' fields");
    }
    BoxData b;
    b.rel_center = DeserializeVector3(j["rel_center"]);
    b.half_extents = DeserializeVector3(j["half_extents"]);
    return b;
}

inline CollisionShape DeserializeCollisionShape(const nlohmann::json& j) {
    CollisionType type = static_cast<CollisionType>(j["type"].get<int>());
    switch (type) {
        case CollisionType::Sphere: return CollisionShape(DeserializeSphere(j["sphere"]));
        case CollisionType::Box:    return CollisionShape(DeserializeBox(j["box"]));
        default: throw std::runtime_error("Unknown collision type in JSON");
    }
}

inline BodyData DeserializeBody(const nlohmann::json& j) {
    if (!j.contains("pos") || !j.contains("vel")) {
        throw std::runtime_error("BodyData requires 'position' and 'velocity' fields");
    }

    BodyData b;
    b.position = DeserializeVector3(j["pos"]);
    b.velocity = DeserializeVector3(j["vel"]);

    for (const auto& cj : j["collisions"]) b.collisions.push_back(DeserializeCollisionShape(cj));
    return b;
}

inline ActorData DeserializeActor(const nlohmann::json& j) {
    if (!j.contains("yaw") || !j.contains("pitch")) {
        throw std::runtime_error("ActorData requires 'yaw' and 'pitch' fields");
    }

    ActorData a(DeserializeBody(j["body"]));
    a.yaw = j["yaw"].get<float>();
    a.pitch = j["pitch"].get<float>();
    a.model_key = j["model_key"].get<uint16_t>();
    return a;
}

inline WorldData DeserializeWorld(const nlohmann::json& j) {
    WorldData w;

    for (const auto& item : j["actors"]) {
        if (!item.is_array() || item.size() != 2) continue;

        uint32_t key = item[0].get<uint32_t>();
        const nlohmann::json& actor_json = item[1];

        w.actors.insert({key, DeserializeActor(actor_json)});
    }

    return w;
}