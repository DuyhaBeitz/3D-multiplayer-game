#pragma once

#include "SceneRegular.hpp"

#include "ResourceData.hpp"
#include "Constants.hpp"
#include "Physics.hpp"
#include "SpaceActorPartitioner.hpp"

#if WITH_RENDER
#include "GameDrawingData.hpp"
#endif

class Green : public SceneRegular {
private:

    #if WITH_RENDER
    void LoadResources();
    void UnloadResources();

    R3D_Light m_light;
    R3D_Cubemap m_cubemap;
    R3D_AmbientMap m_ambient_map;
    #endif

    Vector3 m_door_position{};

    virtual void PostSetup() override; 
    virtual void SetupHeightmap();

public:
    Green();

    #if WITH_RENDER
    virtual void Draw(const GameDrawingData &drawing_data) const;
    #endif

    virtual void Load();
    virtual void Unload();

    virtual GameState PopulateState(const GameState &old_state);

    virtual Scenes CheckSceneChange(const GameState &state);
    virtual void InitNewPlayer(GameState &state, uint32_t id); 

    //virtual void Update(WorldData& world);
};