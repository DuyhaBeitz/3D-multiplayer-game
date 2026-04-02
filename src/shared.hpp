#pragma once

#include <EasyNet/EasyNetShared.hpp>
#include "Game.hpp"

int server_port = 7777;
enum NetMsg : MessageType {
    PLAYER_INPUT = 0,
    PLAYER_JOIN,
    PLAYER_LEAVE,
    GAME_STATE,
    GAME_TICK,
    PLAYER_ID,
    CHAT_MESSAGE,
    GAME_METADATA,
    NAME_CHANGE,
    SCENE_INITIAL,
    SCENE_CHANGE
};

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