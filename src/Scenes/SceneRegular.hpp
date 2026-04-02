#pragma once

#include <Scene.hpp>

#include "ResourceData.hpp"
#include "Constants.hpp"
#include "Physics.hpp"
#include "SpaceActorPartitioner.hpp"

#if WITH_RENDER
#include "GameDrawingData.hpp"
#endif

class SceneRegular : public SceneBase {
protected:

    std::string m_heightmap_image_path;
    uint32_t m_seed;
    Vector3 m_heightmap_scale;
    int m_trees_count;
    int m_grass_count;

    HeightmapData m_heightmap{};
    std::map<ActorKey, ActorData> m_static_actors{};
    ActorPartitioner m_partitioner;

    virtual void PostSetup() {};

public:
    SceneRegular(std::string heightmap_image_path, uint32_t seed, Vector3 heightmap_scale, int trees_count, int grass_count);

    virtual void Setup();
    virtual void InitNewPlayer(GameState &state, uint32_t id); 
    virtual void SolveCollisionWith(BodyData &other) const;
    //virtual void Update(WorldData& world);
};