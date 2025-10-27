#pragma once

#include <nlohmann/json.hpp>
#include "World.hpp"

inline nlohmann::json SerializeVector3(const Vector3& v) {
    return {v.x, v.y, v.z};
}

inline nlohmann::json SerializeSphere(const SphereData& s) {
    return {
        {"center", SerializeVector3(s.center)},
        {"radius", s.radius}
    };
}

inline nlohmann::json SerializeBox(const BoxData& b) {
    return {
        {"center", SerializeVector3(b.center)},
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
    j["acc"] = SerializeVector3(b.acceleration);
    j["on_gr"] = b.on_ground;
    j["inv_m"] = b.inverse_mass;
    j["restitution"] = b.restitution;    

    j["shapes"] = nlohmann::json::array();
    for (const auto& c : b.shapes) {
        j["shapes"].push_back(SerializeShape(c));
    }
    return j;
}

inline nlohmann::json SerializeActorRenderData(const ActorRenderData& render_data) {
    return {
        {"model_key", render_data.model_key},
        {"sec_count", render_data.sec_count},
        {"anim_id", render_data.anim_id},
        {"offset", SerializeVector3(render_data.offset)}
    };    
}

inline nlohmann::json SerializeActor(const ActorData& a) {
    return {
        {"body", SerializeBody(a.body)},
        {"yaw", a.yaw},
        {"pitch", a.pitch},
        {"render_data", SerializeActorRenderData(a.render_data)}
    };
}

inline nlohmann::json SerializePlayer(const PlayerData& p) {
    return {
        {"actor_key", p.actor_key}
    };
}

inline nlohmann::json SerializeWorld(const WorldData& w) {
    nlohmann::json j;
    j["actors"] = nlohmann::json::object();
    for (const auto& [key, a] : w.actors) {
        j["actors"][std::to_string(key)] = SerializeActor(a);
    }
    return j;
}