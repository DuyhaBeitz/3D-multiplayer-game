#pragma once

#include <cstdint>
#include <cstddef>
#include <raylib.h>

#if WITH_RENDER
#include "GameDrawingData.hpp"
#endif

struct GameState;
struct BodyData;

enum class Scenes : uint8_t {
    None = 0,
    Desert,
    Green
};

class SceneBase {
private:
public:
    #if WITH_RENDER
    virtual void Draw(const GameDrawingData &drawing_data) const = 0;
    #endif

    virtual void Load() = 0;
    virtual void Unload() = 0;

    virtual void Setup() = 0;
    virtual GameState PopulateState(const GameState &old_state) = 0;

    virtual void InitNewPlayer(GameState &state, uint32_t id) = 0; 
    virtual void SolveCollisionWith(BodyData &other) const = 0;

    virtual Scenes CheckSceneChange(const GameState &state) = 0;
    //virtual void Update(WorldData& world) = 0;
};