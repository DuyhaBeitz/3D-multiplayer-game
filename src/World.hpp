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
        body_data.position = Vector3{30, 10, 0};
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
    constexpr int phys_iters = 1;
    float sub_dt = dt / phys_iters;
    
    for (int i = 0; i < phys_iters; i++) {
        // First, detect all collisions
        std::vector<std::pair<BodyData*, BodyData*>> collisions;
        
        auto actor_it1 = actors.begin();
        while (actor_it1 != actors.end()) {
            auto actor_it2 = actor_it1;
            ++actor_it2;
            
            while (actor_it2 != actors.end()) {
                auto& body1 = actor_it1->second.body;
                auto& body2 = actor_it2->second.body;
                
                auto collision = body1.CollideWith(body2);
                if (collision.penetration > 0) {
                    collisions.emplace_back(&body1, &body2);
                }
                ++actor_it2;
            }
            ++actor_it1;
        }
        
        // Then, resolve all collisions
        for (auto& [body1, body2] : collisions) {
            SolveCollision(*body1, *body2, body1->CollideWith(*body2));
        }
        
        // Finally, update all actors
        for (auto& [key, actor_data] : actors) {
            actor_data.Update(sub_dt);
        }
    }
}

    bool ActorExists(ActorKey key) const {return actors.find(key) != actors.end();}
    
    ActorKey AddActor(const ActorData& actor_data) {
        ActorKey key = new_actor_key;
        AddActor(key, actor_data);
        return key;
    }

    void AddActor(ActorKey key, const ActorData& actor_data) {
        actors.insert({key, actor_data});
        new_actor_key = key+1;
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