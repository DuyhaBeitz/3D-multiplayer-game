#pragma once

#include <EasyNet/EasyNetShared.hpp>
#include "Game.hpp"

int server_port = 7777;

constexpr MessageType MSG_PLAYER_INPUT = MSG_USER_BASE;
constexpr MessageType MSG_GAME_STATE = MSG_USER_BASE+1;
constexpr MessageType MSG_GAME_TICK = MSG_USER_BASE+2;
constexpr MessageType MSG_PLAYER_ID = MSG_USER_BASE+3;
constexpr MessageType MSG_CHAT_MESSAGE = MSG_USER_BASE+4;
constexpr MessageType MSG_GAME_METADATA = MSG_USER_BASE+5;
constexpr MessageType MSG_NAME_CHANGE = MSG_USER_BASE+6;

struct PlayerInputPacketData {
    PlayerInput input{};
    uint32_t tick{};

    PlayerInputPacketData() = default;
};

struct TextPacketData {
    char text[max_string_len] = {};
    TextPacketData(const char* str) { 
        std::strncpy(text, str, sizeof(text));
        text[sizeof(text)-1] = '\0';
    }
    TextPacketData() = default; // needed for packet data, because before copying the data, the lvalue is declared
};