#pragma once

#include <EasyNet/EasyNetShared.hpp>
#include "Game.hpp"

int server_port = 7777;

constexpr MessageType MSG_PLAYER_INPUT = MSG_USER_BASE;
constexpr MessageType MSG_GAME_STATE = MSG_USER_BASE+1;
constexpr MessageType MSG_GAME_TICK = MSG_USER_BASE+2;
constexpr MessageType MSG_PLAYER_ID = MSG_USER_BASE+3;

struct PlayerInputPacketData {
    PlayerInput input;
    uint32_t tick;

    PlayerInputPacketData() = default;
};