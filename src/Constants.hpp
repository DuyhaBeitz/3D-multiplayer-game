#pragma once

#include <cstdint>

constexpr int max_string_len = 2048;

constexpr int iters_per_sec = 60;
constexpr double dt = 1.f/iters_per_sec;

// client also uses that
constexpr uint32_t tick_period = iters_per_sec/20; // broadcast game state every 100 ms
constexpr uint32_t receive_tick_period = iters_per_sec/2; // allow late received events
constexpr uint32_t send_tick_period = tick_period; // sync client's tick with server's tick
constexpr uint32_t server_lateness = receive_tick_period;
// ensuring that we're not substructing bigger uint32_t from the smaller one
constexpr uint32_t max_lateness = server_lateness+tick_period+receive_tick_period;

constexpr uint32_t broadcast_game_metadata_tick_period = iters_per_sec*2;

constexpr size_t max_chat_messages = 10;
