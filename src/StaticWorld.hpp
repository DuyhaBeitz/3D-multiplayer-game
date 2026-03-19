#pragma once

#include "ResourceData.hpp"
#include "Constants.hpp"
#include "Physics.hpp"
#include "SpacePartition.hpp"

#if WITH_RENDER
#include "GameDrawingData.hpp"
#endif

#include <list>

/*
Static objects, like terrain, don't need to be synced
It's also persistent between two ticks, so no need for prev/current state 
Only updated on signal (smth like MSG_LEVEL_CHANGE)
*/

class GameMetadata;
class ActorData;

class StaticWorld {
private:
    HeightmapData m_heightmap{};
    ModelKey m_heightmap_model_key{};

    std::map<ActorKey, ActorData> m_static_actors{};
    PartitionGrid m_grid{};
    std::list<PartitionUnit> m_units{}; // vector copies objects on resize, which causes problems with pointers used in grid

    ActorData& AddStaticActor(ActorKey actor_key, ActorData static_actor);

public:
    StaticWorld();

#if WITH_RENDER
    void Draw(const GameDrawingData &drawing_data) const;
#endif


    HeightmapData& GetHeightmap() {
        return m_heightmap;
    }

    const HeightmapData& GetHeightmap() const {
        return m_heightmap;
    }

    void SolveCollisionWith(BodyData &other) const;

// #if WITH_RENDER
//     void LoadVisuals(const GameMetadata& game_metadata);
// #endif

    void SetupWorld(const GameMetadata& game_metadata);
    void Load(const GameMetadata& game_metadata);
    
};