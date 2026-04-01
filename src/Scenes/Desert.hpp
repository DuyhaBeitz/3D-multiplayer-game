#pragma once

#include <Scene.hpp>

#include "ResourceData.hpp"
#include "Constants.hpp"
#include "Physics.hpp"
#include "SpaceActorPartitioner.hpp"

#if WITH_RENDER
#include "GameDrawingData.hpp"
#endif

class Desert : public SceneBase {
private:
    HeightmapData m_heightmap{};
    std::map<ActorKey, ActorData> m_static_actors{};
    ActorPartitioner m_partitioner;

    void SetupStatic();

public:
    Desert();

    #if WITH_RENDER
    virtual void LoadResources();
    virtual void UnloadResources();
    virtual void Draw(const GameDrawingData &drawing_data) const;
    #endif

    virtual void Setup();
    virtual GameState PopulateState(const GameState &old_state);
        
    virtual void InitNewPlayer(GameState &state, uint32_t id);
    virtual void SolveCollisionWith(BodyData &other) const;
    //virtual void Update(WorldData& world);
};