#pragma once

#include <cstdint>
#include <cstddef>
#include <raylib.h>

constexpr int max_string_len = 2048;
constexpr int max_player_name_len = 64;

constexpr int iters_per_sec = 60;
constexpr double dt = 1.f/iters_per_sec;

constexpr size_t max_chat_messages = 3;

using ActorKey = uint16_t;

constexpr Vector3 heightmap0_scale = Vector3{1000, 100, 1000};
constexpr int trees_count = 250;
constexpr int grass_count = 500;

constexpr int raylib_log_level = LOG_ALL;

constexpr const char* server_chat_name = "Server";