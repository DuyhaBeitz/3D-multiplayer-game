#pragma once

#include "GameBase.hpp"
#include <variant>
#include <raylib.h>
#include <raymath.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>

constexpr int max_string_len = 1024;

constexpr int iters_per_sec = 60;
constexpr double dt = 1.f/iters_per_sec;
constexpr double gravity = 40;
constexpr double floor_lvl = 500;
constexpr double hor_speed = 60;
constexpr double jump_impulse = 30;


// client also uses that
constexpr uint32_t tick_period = iters_per_sec/10; // broadcast game state every 100 ms
constexpr uint32_t receive_tick_period = iters_per_sec; // allow late received events
constexpr uint32_t send_tick_period = iters_per_sec*2; // sync client's tick with server's tick
constexpr uint32_t server_lateness = receive_tick_period;

enum EventId {
    EV_PLAYER_JOIN = 0,
    EV_PLAYER_LEAVE,
    EV_PLAYER_INPUT,
};

// Event data
struct PlayerJoin {};
struct PlayerLeave {};
struct PlayerInput {
    bool right;
    bool left;
    bool up;

    float GetX() { return right - left; }
};

struct GameEvent {
    EventId event_id; // for sending over the net
    std::variant<std::monostate, PlayerJoin, PlayerLeave, PlayerInput> data;
};

struct PlayerState {
    Vector2 position;
    Vector2 velocity;
};

struct GameState {
    std::map<uint32_t, PlayerState> players;
};

struct SerializedGameState {
    char text[max_string_len];
    uint32_t tick;

    SerializedGameState(const char* str) { 
        std::strncpy(text, str, sizeof(text));
        text[sizeof(text)-1] = '\0';
    }
    SerializedGameState() = default; // needed for packet data, because before copying the data, the lvalue is declared
};

struct DrawingData {
    uint32_t special_id;
    bool inc_exc_sp_id; // to include only the special id, or the other way around: leave only everybody else
    Color color;
    bool uses_special_id = true;
};

class Game : public GameBase<GameState, GameEvent, SerializedGameState> {
public:
    PlayerState InitNewPlayer(const GameState& state, uint32_t id) {
        return PlayerState{Vector2{0, 0}};
    }

    virtual void ApplyEvent(GameState& state, const GameEvent& event, uint32_t id) {
        switch (event.event_id) {
        case EV_PLAYER_JOIN:
            state.players[id] = InitNewPlayer(state, id);
            break;

        case EV_PLAYER_LEAVE:
            state.players.erase(id);
            break;

        case EV_PLAYER_INPUT:
            if (std::holds_alternative<PlayerInput>(event.data)) {
                auto input = std::get<PlayerInput>(event.data);
                if (state.players.find(id) != state.players.end()) {
                    state.players[id].velocity.x += input.GetX() * dt * hor_speed;
                    if (input.up && state.players[id].position.y == floor_lvl) state.players[id].velocity.y -= jump_impulse;
                }
            }
            break; 

        default:
            break;
        }
    }

    virtual void Draw(const GameState& state, const void* data) {
        for (const auto& [id, player] : state.players) {
            const DrawingData* drawing_data = static_cast<const DrawingData*>(data);
            if ((!drawing_data->uses_special_id) || (drawing_data->inc_exc_sp_id == (id == drawing_data->special_id))) {
                DrawCircleV(player.position, 10, drawing_data->color);
            }            
        }
    }

    virtual void UpdateGameLogic(GameState& state) {
        for (auto& [id, player] : state.players) {
            player.velocity.y += gravity*dt;
            player.position += player.velocity;
            if (player.position.y > floor_lvl) {
                player.position.y = floor_lvl;
            }
            player.velocity *= 0.9;
        }
    }

    void OutputHistory() {
        bool to_file = true;
        std::ofstream file("output.txt");
        std::ostream* out = to_file ? &file : &std::cout;
    
        for (auto& [tick, events] : m_event_history) {
            *out << tick << ":" << events.size() << "\n";
            for (auto& [id, event] : events) {
                switch (event.event_id) {
                case EV_PLAYER_INPUT:
                    {
                    auto input = std::get<PlayerInput>(event.data);
                    *out << "\tINPUT\t" << "X: " << input.GetX() << " up: " << input.up << std::endl;
                    }
                    break;
                case EV_PLAYER_JOIN:
                    *out << "\tJOIN\n" << std::endl;
                    break;
                case EV_PLAYER_LEAVE:
                    *out << "\tLEAVE\n" << std::endl;
                    break;
                }         
                *out << std::endl;       
            }
        }
    }

    virtual GameState Lerp(const GameState& state1, const GameState& state2, float alpha, const void* data) {
        alpha = fmin(1, fmax(0, alpha));
        GameState lerped = state2;

        const uint32_t* except_id = static_cast<const uint32_t*>(data);

        for (auto& [id, player] : state2.players) {
            if (id != *except_id) {
                if (state1.players.find(id) != state1.players.end()) {
                    lerped.players[id].position = Vector2Lerp(state1.players.at(id).position, state2.players.at(id).position, alpha);
                }
            }
        }
        return lerped;
    };

    virtual SerializedGameState Serialize(const GameState& state) {
        nlohmann::json j;
        for (const auto& [id, player] : state.players) {
            j["players"][std::to_string(id)] = {
                {"px", player.position.x},
                {"py", player.position.y},
                {"vx", player.velocity.x},
                {"vy", player.velocity.y}
            };
        }
        return SerializedGameState(j.dump().c_str());
    }

    GameState Deserialize(SerializedGameState data) {
        GameState state{};
        nlohmann::json j = nlohmann::json::parse(std::string(data.text, max_string_len));
        for (auto& [id_str, player_json] : j["players"].items()) {
            uint32_t id = static_cast<uint32_t>(std::stoul(id_str));
            PlayerState ps;
            ps.position.x = player_json["px"].get<float>();
            ps.position.y = player_json["py"].get<float>();
            ps.velocity.x = player_json["vx"].get<float>();
            ps.velocity.y = player_json["vy"].get<float>();
            state.players[id] = ps;
        }
        return state;
    }
};

