#include "World.hpp"
#include "GameMetadata.hpp"
#include "StaticWorld.hpp"

#if WITH_RENDER
void WorldData::Draw(const GameDrawingData &drawing_data) const {
    for (const auto& [key, actor_data] : actors) {
        if (drawing_data.actors_except.find(key) == drawing_data.actors_except.end()) actor_data.Draw(drawing_data);
    }
}
#endif

void WorldData::Update(float delta_time, const GameMetadata& game_metadata, const StaticWorld& static_world){
    constexpr int phys_iters = 1;
    float sub_dt = delta_time / phys_iters;
    
    m_partitioner.UpdateView();
    for (int i = 0; i < phys_iters; i++) {

        m_partitioner.GetGrid().iterate_cells();

        for (auto& [actor_key, actor_data] : actors) {
            static_world.SolveCollisionWith(actor_data.body);
        }
        
        for (auto& [key, actor_data] : actors) {
            actor_data.Update(sub_dt);
        }
        m_partitioner.UpdateView();
    }
}