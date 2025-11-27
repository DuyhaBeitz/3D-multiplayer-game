#pragma once

#include <raylib.h>
#include <raymath.h>
#include "Resources.hpp"
#include <map>

#include "Actor.hpp"

struct PlayerData {
    ActorKey actor_key = 0;
    template <class Archive>
    void serialize(Archive& ar) {
        ar(actor_key);
    }
};

class GameMetadata;

struct WorldData {
    ActorKey new_actor_key = 0;
    std::map<ActorKey, ActorData> actors{};

    WorldData() {
        /*
        If you add actor here it won't be updated so mismatch will happen
        So do not add actors here or do anything stupid, because then mismatch will happen
        */        
    }

    void Draw(GameDrawingData &drawing_data) const;

    void Update(float delta_time, const GameMetadata& game_metadata);

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

    template <class Archive>
    void serialize(Archive& ar) {
        ar(new_actor_key, actors);
    }
};