#pragma once

#include <raylib.h>
#include <raymath.h>
#include "Resources.hpp"
#include <map>

#include "Actor.hpp"

struct WorldData {
    std::map<uint32_t, ActorData> actors;

    WorldData() {}

    void Draw(uint32_t except_id) const {
        DrawGrid(100, 10);

        for (const auto& [key, actor_data] : actors) {
            if (key != except_id) actor_data.Draw();
        }
    }

    void Update(float dt) {
        for (auto& [key, actor_data] : actors) {
            actor_data.Update(dt);
        }
    }
};