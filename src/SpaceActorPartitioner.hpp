#pragma once
#include "Actor.hpp"
#include "SpacePartition.hpp"
#include <list>
#include <unordered_set>

class ActorPartitioner {
private:
    std::map<ActorKey, ActorData>* m_actors = nullptr;
    PartitionGrid m_grid{};
    std::list<PartitionUnit> m_units{}; // vector copies objects on resize, which causes problems with pointers used in grid
    std::unordered_set<ActorKey> m_known_keys{};

    ActorData& AddActor(ActorKey actor_key);
public:
    void UpdateView();
    
    ActorPartitioner(std::map<ActorKey, ActorData>* actors) 
    : m_actors(actors)
    {
    }

    PartitionGrid& GetGrid() { return m_grid; }
    const PartitionGrid& GetGrid() const { return m_grid; }
};