#pragma once

#include <raylib.h>
#include <raymath.h>

#include "ResourceData.hpp"

#if WITH_RENDER
#include "Resources.hpp"
#include "Audio.hpp"
#endif

#include <map>

#include "Actor.hpp"
#include "SpaceActorPartitioner.hpp"

struct PlayerData {
    ActorKey actor_key = 0;
    template <class Archive>
    void serialize(Archive& ar) {
        ar(actor_key);
    }
};

class GameMetadata;
class SceneBase;

struct WorldData {
    ActorKey new_actor_key = 0;
    std::map<ActorKey, ActorData> actors{};
    ActorPartitioner m_partitioner;

    void HandlePhysicsPair(PartitionUnit* un1, PartitionUnit* un2, void* user_data) {
        if (un1 && un2 && un1 != un2) {
            ActorKey key1 = (ActorKey)reinterpret_cast<uint64_t>(un1->user_data);
            ActorKey key2 = (ActorKey)reinterpret_cast<uint64_t>(un2->user_data);
            BodyData* body1 = &actors[key1].body;
            BodyData* body2 = &actors[key2].body;
            if (body1 && body2) {
                CollisionResult res = body1->CollideWith(*body2);
                if (res.penetration >= 0) {
                    SolveCollision(*body1, *body2, res);
                    #if WITH_RENDER
                    uint32_t tick = (uint32_t)reinterpret_cast<uint64_t>(user_data);
                    Audio::Get().EmitSoundEvent(
                        SoundEvent(FLAG_SOUND_PHYISCS_DD, key1, key2, tick,
                            res.hit_pos, body1->velocity-body2->velocity,
                            R_SOUND_DEFAULT
                        )
                    );
                    #endif
                }
            }
        }
    }

    WorldData() : m_partitioner(&actors) {
        /*
        The world data is constantly copied for reconciliation
        If you add actor here it won't be updated so mismatch will happen
        So do not add actors here or do anything stupid, because then mismatch will happen
        */        

        m_partitioner.GetGrid().SetHandlePairFunc([this](PartitionUnit* un1, PartitionUnit* un2, void* user_data){
            HandlePhysicsPair(un1, un2, user_data);
        });
    }

    WorldData(const WorldData& other)
        : new_actor_key(other.new_actor_key)
        , actors(other.actors)
        , m_partitioner(&actors)
    {
        m_partitioner.GetGrid().SetHandlePairFunc([this](PartitionUnit* un1, PartitionUnit* un2, void* user_data){
            HandlePhysicsPair(un1, un2, user_data);
        });
    }

    WorldData& operator=(const WorldData& other) {
        if (this != &other) {
            new_actor_key = other.new_actor_key;
            actors = other.actors;
            m_partitioner = ActorPartitioner(&actors);

            m_partitioner.GetGrid().SetHandlePairFunc([this](PartitionUnit* un1, PartitionUnit* un2, void* user_data){
                HandlePhysicsPair(un1, un2, user_data);
            });
        }
        return *this;
    }

#if WITH_RENDER
    void Draw(const GameDrawingData &drawing_data) const;
#endif

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