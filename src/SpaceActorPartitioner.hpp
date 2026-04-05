#pragma once
#include "Actor.hpp"
#include "SpacePartition.hpp"
#include <unordered_map>

struct GameState;

class ActorPartitioner {
private:
    std::map<ActorKey, ActorData>* m_actors = nullptr;
    PartitionGrid m_grid{};

    std::unordered_map<ActorKey, PartitionUnit> m_units{}; // vector copies objects on resize, which causes problems with pointers used in grid

    // cannot just update grid immediately on_actor_move,
    // so we first mark them, then on global update touch grid
    
    // otherwise, there could be situations,
    // when traversing grid caused physics collisions,
    // which causes actors to move,
    // which caused their units on grid to move
    // invalidating linked list prev/next pointers
    std::unordered_set<PartitionUnit*> m_marked_units{};

    ActorData& AddActor(ActorKey actor_key);
    void RemoveActor(ActorKey actor_key);

public:
    void UpdateView();
    ActorData& GetActor(ActorKey actor_key) { return (*m_actors).at(actor_key); }
    
    ActorPartitioner(std::map<ActorKey, ActorData>* actors) 
    : m_actors(actors)
    {
    }

    PartitionGrid& GetGrid() { return m_grid; }
    const PartitionGrid& GetGrid() const { return m_grid; }
};