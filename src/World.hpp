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
        /*
        If you add actor here it won't be updated so mismatch will happen
        So do not add actors here or do anything stupid, because then mismatch will happen
        */        
    }

    void Draw(ActorKey except_key) const {
        DrawGrid(10, 10);

        for (const auto& [key, actor_data] : actors) {
            if (key != except_key) actor_data.Draw();
        }
    }

void Update(float delta_time) {
    constexpr int phys_iters = 1;
    float sub_dt = delta_time / phys_iters;
    
    for (int i = 0; i < phys_iters; i++) {
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
        
        for (auto& [body1, body2] : collisions) {
            SolveCollision(*body1, *body2, body1->CollideWith(*body2));
        }
        
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