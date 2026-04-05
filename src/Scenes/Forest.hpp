#pragma once

#include "SceneRegular.hpp"

#include "ResourceData.hpp"
#include "Constants.hpp"
#include "Physics.hpp"
#include "SpaceActorPartitioner.hpp"

#if WITH_RENDER
#include "GameDrawingData.hpp"
#endif

class Forest : public SceneRegular {
private:

    #if WITH_RENDER
    void LoadResources();
    void UnloadResources();

    std::unordered_map<ActorKey, R3D_Light> m_lights;
    
    R3D_Light CreateLight() {
        R3D_Light light = R3D_CreateLight(R3D_LIGHT_SPOT);
        R3D_SetShadowUpdateMode(light, R3D_SHADOW_UPDATE_INTERVAL);
        R3D_SetLightActive(light, true);
        R3D_SetLightRange(light, 50.0f);
        R3D_SetShadowSoftness(light, 3.2f);
        R3D_SetShadowDepthBias(light, 0.001f);
        R3D_EnableShadow(light);
        return light;
    }

    R3D_Cubemap m_cubemap;
    R3D_AmbientMap m_ambient_map;
    #endif

    Vector3 m_door_position{};

    virtual void PostSetup() override; 
    virtual void SetupHeightmap();

    Image LoadHeightmapImage() {
        return LoadImageFromPerlinNoise(89323, 128, 128, Vector2{0.015, 0.015}, 10);
    }

public:
    Forest();

    #if WITH_RENDER
    virtual void Draw(const GameDrawingData &drawing_data) const;
    #endif

    virtual void Load();
    virtual void Unload();

    virtual GameState PopulateState(const GameState &old_state);

    virtual Scenes CheckSceneChange(const GameState &state);
    virtual void InitNewPlayer(GameState &state, uint32_t id); 

    virtual void UpdateActorVisuals(GameState &state, ActorKey actor_key, uint32_t tick, void* user_data) override;

    //virtual void Update(WorldData& world);
};