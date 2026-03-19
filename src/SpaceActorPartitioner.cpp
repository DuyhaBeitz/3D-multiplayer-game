#include "SpaceActorPartitioner.hpp"

ActorData &ActorPartitioner::AddActor(ActorKey actor_key) {
    ActorData& actor = (*m_actors).at(actor_key);
    PartitionUnit unit(&m_grid, actor.body.position.x, actor.body.position.z);
    unit.user_data = reinterpret_cast<void*>(&actor.body);
    m_units.push_back(unit);
    m_grid.add(&m_units.back());

    m_known_keys.emplace(actor_key);
    
    int s = m_units.size();
    actor.body.SetOnUpdatePos(
        [this, s](Vector3){
            m_marked_units.emplace(&(m_units[s-1]));
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

    for (auto unit_ptr : m_marked_units) {
        BodyData* body = reinterpret_cast<BodyData*>(unit_ptr->user_data);
        unit_ptr->move(body->position.x, body->position.z);
    }

    m_marked_units.clear();
}