#pragma once

#include <raylib.h>
#include <raymath.h>
#include "Resources.hpp"
#include <map>

#include "Actor.hpp"

using ActorKey = uint16_t;

struct PlayerData {
    ActorKey actor_key;
};

struct WorldData {
    ActorKey new_actor_key = 0;
    std::map<ActorKey, ActorData> actors;

    WorldData() {
        BoxData box_data;
        box_data.half_extents = Vector3{10, 10, 10};

        BodyData body_data;
        body_data.position = Vector3{30, 0, 0};
        body_data.shapes.push_back(CollisionShape(box_data));

        AddActor(ActorData(body_data));
    }

    void Draw(ActorKey except_key) const {
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

    bool ActorExists(ActorKey key) const {return actors.find(key) != actors.end();}
    ActorKey AddActor(ActorData actor_data) {
        ActorKey key = new_actor_key;
        actors.insert({key, actor_data});

        new_actor_key++;

        return key;
    }

    void RemoveActor(ActorKey key) {actors.erase(key);}

    const ActorData& GetActor(ActorKey key) const {
        if (ActorExists(key)) return actors.at(key);
        else throw std::runtime_error("Actor doesn't exist");   
    }

    ActorData& GetActor(ActorKey key) {
        if (ActorExists(key)) return actors.at(key);
        else throw std::runtime_error("Actor doesn't exist");   
    }
};