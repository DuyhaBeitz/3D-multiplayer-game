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

    uint32_t m_seed;
    Vector3 m_heightmap_scale;
    int m_trees_count;
    int m_grass_count;

    HeightmapData m_heightmap{};
    std::map<ActorKey, ActorData> m_static_actors{};
    ActorPartitioner m_partitioner;

    float m_tree_scale = 1.0f;
    float m_grass_scale = 1.0f;

    virtual void SetupHeightmap() = 0;
    virtual void PostSetup() {};

public:
    SceneRegular(uint32_t seed, Vector3 heightmap_scale, int trees_count, int grass_count, float tree_scale = 1.0f, float grass_scale = 1.0f);

    virtual void Setup();
    virtual void SolveCollisionWith(BodyData &other) const;
    //virtual void Update(WorldData& world);
};