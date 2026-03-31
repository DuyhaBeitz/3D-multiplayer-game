#include "SpaceActorPartitioner.hpp"
#include <unordered_set>

ActorData &ActorPartitioner::AddActor(ActorKey actor_key) {
    ActorData& actor = (*m_actors).at(actor_key);
    PartitionUnit unit(&m_grid, actor.body.position.x, actor.body.position.z);
    unit.user_data = reinterpret_cast<void*>(&actor.body);
    
    if (m_units.find(actor_key) != m_units.end()) {
        RemoveActor(actor_key);
    }

    m_units[actor_key] = unit;
    m_grid.add(&m_units[actor_key]);

    actor.body.SetOnUpdatePos(
        [this, actor_key](Vector3){
            m_marked_units.emplace(&(m_units[actor_key]));
        }
    );

    return actor;
}

void ActorPartitioner::RemoveActor(ActorKey actor_key) {
    m_grid.remove(&m_units[actor_key]);
    m_units.erase(actor_key);
}

void ActorPartitioner::UpdateView() {
    // detect added actors
    for (auto& [actor_key, actor_data] : *m_actors) {
        if (m_units.find(actor_key) == m_units.end()) {
            AddActor(actor_key);
        }
    }

    // update moved actors
    for (auto unit_ptr : m_marked_units) {
        BodyData* body = reinterpret_cast<BodyData*>(unit_ptr->user_data);
        unit_ptr->move(body->position.x, body->position.z);
    }
    m_marked_units.clear();

    // detect removed actors
    std::unordered_set<ActorKey> for_deletion{};
    for (auto& [actor_key, unit] : m_units) {
        if ((*m_actors).find(actor_key) == (*m_actors).end()) {
            for_deletion.emplace(actor_key);
        }
    }    
    for (auto& actor_key : for_deletion) {
        RemoveActor(actor_key);
    }
}