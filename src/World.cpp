#include "World.hpp"
#include "GameMetadata.hpp"

void WorldData::Draw(GameDrawingData &drawing_data) const {
    for (const auto& [key, actor_data] : actors) {
        if (drawing_data.actors_except.find(key) == drawing_data.actors_except.end()) actor_data.Draw(drawing_data);
    }
}

void WorldData::Update(float delta_time, const GameMetadata& game_metadata){
    constexpr int phys_iters = 10;
    float sub_dt = delta_time / phys_iters;
    
    for (int i = 0; i < phys_iters; i++) {
        std::vector<std::pair<BodyData*, BodyData*>> collisions;
        
        auto actor_it1 = actors.begin();
        while (actor_it1 != actors.end()) {
            auto actor_it2 = actor_it1;
            ++actor_it2;
            
            while (actor_it2 != actors.end()) {
                auto& body1 = actor_it1->second.body;
                auto& body2 = actor_it2->second.body;
                
                auto collision = body1.CollideWith(body2);
                if (collision.penetration > 0) {
                    collisions.emplace_back(&body1, &body2);
                }
                ++actor_it2;
            }
            ++actor_it1;
        }
        
        for (auto& [body1, body2] : collisions) {
            SolveCollision(*body1, *body2, body1->CollideWith(*body2));
        }

        for (auto& [actor_key, actor_data] : actors) {
            game_metadata.GetHeightmap().SolveCollisionWith(actor_data.body);
        }        
        
        for (auto& [key, actor_data] : actors) {
            actor_data.Update(sub_dt);
        }
    }
}