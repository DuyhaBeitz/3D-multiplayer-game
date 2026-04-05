#include "World.hpp"
#include "GameMetadata.hpp"
#include "Scene.hpp"

#if WITH_RENDER
void WorldData::Draw(const GameDrawingData &drawing_data) const {
    for (const auto& [key, actor_data] : actors) {
        if (drawing_data.actors_except.find(key) == drawing_data.actors_except.end()) actor_data.Draw(drawing_data);
    }
}
#endif