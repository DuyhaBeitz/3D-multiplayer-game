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
    s.center = DeserializeVector3(j["center"]);
    s.radius = j["radius"];
    return s;
}

inline BoxData DeserializeBox(const nlohmann::json& j) {
    if (!j.contains("center") || !j.contains("half_extents")) {
        throw std::runtime_error("BoxData requires 'center' and 'half_extents' fields");
    }
    BoxData b;
    b.center = DeserializeVector3(j["center"]);
    b.half_extents = DeserializeVector3(j["half_extents"]);
    return b;
}

inline CollisionShape DeserializeShape(const nlohmann::json& j) {
    std::string type = j["type"].get<std::string>();
    if (type == "sphere") {
        return CollisionShape(DeserializeSphere(j["sphere"]));
    } else if (type == "box") {
        return CollisionShape(DeserializeBox(j["box"]));
    } else {
        throw std::runtime_error("Unknown collision shape type: " + type);
    }
}

inline BodyData DeserializeBody(const nlohmann::json& j) {
    if (!j.contains("pos") || !j.contains("vel") || !j.contains("inv_m") || !j.contains("restitution") || !j.contains("shapes")) {
        throw std::runtime_error(
        "BodyData requires 'pos', 'vel', 'inv_m', 'restitution' and 'shapes' fields"
        );
    }

    BodyData b;
    b.position = DeserializeVector3(j["pos"]);
    b.velocity = DeserializeVector3(j["vel"]);
    b.inverse_mass = j["inv_m"].get<float>();
    b.restitution = j["restitution"].get<float>();    

    for (const auto& cj : j["shapes"]) b.shapes.push_back(DeserializeShape(cj));
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

inline PlayerData DeserializePlayer(const nlohmann::json& j) {
    if (!j.contains("actor_key")) {
        throw std::runtime_error("PlayerData requires 'actor_key' field");
    }

    PlayerData p;
    p.actor_key = j["actor_key"].get<ActorKey>();
    return p;
}

#include "Serialization.hpp"
inline WorldData DeserializeWorld(const nlohmann::json& j) {
    WorldData w;

    for (auto it = j["actors"].begin(); it != j["actors"].end(); ++it) {
        uint32_t key = std::stoul(it.key());
        w.AddActor(key, DeserializeActor(it.value()));
    }

    // std::cout << "json: " << j["actors"] << std::endl << std::endl;

    // std::cout << "actual: " << SerializeWorld(w)["actors"] << std::endl << std::endl;

    return w;
}