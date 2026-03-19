#include "SpaceActorPartitioner.hpp"

ActorData &ActorPartitioner::AddActor(ActorKey actor_key) {
    ActorData& actor = (*m_actors).at(actor_key);
    PartitionUnit unit(&m_grid, actor.body.position.x, actor.body.position.z);
    unit.user_data = reinterpret_cast<void*>(&actor.body);
    m_units.push_back(unit);
    m_grid.add(&m_units.back());

    m_known_keys.emplace(actor_key);
    
    actor.body.SetOnUpdatePos(
        [this, &unit](Vector3 new_pos){
            m_grid.move(&unit, new_pos.x, new_pos.z);
        }
    );

    return actor;
}

void ActorPartitioner::UpdateView() {
    for (auto& [actor_key, actor_data] : *m_actors) {
        if (m_known_keys.find(actor_key) == m_known_keys.end()) {
            AddActor(actor_key);
        }
    }    
}