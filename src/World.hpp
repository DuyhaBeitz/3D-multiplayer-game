#pragma once

#include <raylib.h>
#include <raymath.h>
#include "Resources.hpp"
#include <map>

#include "Actor.hpp"

using actor_key = uint16_t;

struct WorldData {
    actor_key new_actor_key = 0;
    std::map<actor_key, ActorData> actors;

    WorldData() {}

    void Draw(actor_key except_key) const {
        DrawGrid(100, 10);

        for (const auto& [key, actor_data] : actors) {
            if (key != except_key) actor_data.Draw();
        }
    }

    void Update(float dt) {
        for (auto& [key, actor_data] : actors) {
            actor_data.Update(dt);
        }
    }

    bool ActorExists(actor_key key) {return actors.find(key) != actors.end();}
    actor_key AddActor(ActorData actor_data) {
        actor_key key = new_actor_key;
        actors.insert({key, actor_data});

        new_actor_key++;

        return key;
    }

    void RemoveActor(actor_key key) {actors.erase(key);}
};