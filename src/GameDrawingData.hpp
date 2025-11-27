#pragma once

#include "GameMetadata.hpp"

struct GameDrawingData {
    std::set<ActorKey> actors_except{};
    GameMetadata& game_metadata;
};