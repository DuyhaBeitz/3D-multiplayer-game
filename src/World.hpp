#pragma once

#include <raylib.h>
#include <raymath.h>

#include "ResourceData.hpp"
#if WITH_RENDER
#include "Resources.hpp"
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

    WorldData() : m_partitioner(&actors) {
        /*
        The world data is constantly copied for reconciliation
        If you add actor here it won't be updated so mismatch will happen
        So do not add actors here or do anything stupid, because then mismatch will happen
        */        

        m_partitioner.GetGrid().SetHandlePairFunc(
            [](PartitionUnit* un1, PartitionUnit* un2){
                if (un1 && un2) {
                    BodyData* body1 = reinterpret_cast<BodyData*>(un1->user_data);
                    BodyData* body2 = reinterpret_cast<BodyData*>(un2->user_data);
                    if (body1 && body2) SolveCollision(*body1, *body2, body1->CollideWith(*body2));
                }
            }
        );
    }

    WorldData(const WorldData& other)
        : new_actor_key(other.new_actor_key)
        , actors(other.actors)
        , m_partitioner(&actors)
    {
        m_partitioner.GetGrid().SetHandlePairFunc(
            [](PartitionUnit* un1, PartitionUnit* un2){
                if (un1 && un2) {
                    BodyData* body1 = reinterpret_cast<BodyData*>(un1->user_data);
                    BodyData* body2 = reinterpret_cast<BodyData*>(un2->user_data);
                    if (body1 && body2) SolveCollision(*body1, *body2, body1->CollideWith(*body2));
                }
            }
        );
    }

    WorldData& operator=(const WorldData& other) {
        if (this != &other) {
            new_actor_key = other.new_actor_key;
            actors = other.actors;
            m_partitioner = ActorPartitioner(&actors);
            m_partitioner.GetGrid().SetHandlePairFunc(
                [](PartitionUnit* un1, PartitionUnit* un2){
                if (un1 && un2) {
                    BodyData* body1 = reinterpret_cast<BodyData*>(un1->user_data);
                    BodyData* body2 = reinterpret_cast<BodyData*>(un2->user_data);
                    if (body1 && body2) SolveCollision(*body1, *body2, body1->CollideWith(*body2));
                }
            }
            );
        }
        return *this;
    }

#if WITH_RENDER
    void Draw(const GameDrawingData &drawing_data) const;
#endif

    void Update(float delta_time, const GameMetadata& game_metadata, const SceneBase* scene);

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