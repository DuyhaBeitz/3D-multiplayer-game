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
constexpr double floor_lvl = 0;
constexpr double hor_speed = 60;
constexpr double jump_impulse = 20;


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
    float mouse_x;
    float mouse_y;
    bool right;
    bool left;
    bool forw;
    bool back;
    bool up;
    bool down;
  
    void Detect() {
        if (!IsCursorHidden()){
            mouse_x = 0;
            mouse_y = 0;    
        }
        else {
            mouse_x = GetMouseDelta().x / 450;
            mouse_y = -GetMouseDelta().y / 450;
        }
        right = IsKeyDown(KEY_D);
        left = IsKeyDown(KEY_A);
        forw = IsKeyDown(KEY_W);
        back = IsKeyDown(KEY_S);
        up = IsKeyDown(KEY_SPACE);
        down = IsKeyDown(KEY_LEFT_CONTROL);
    }
  
    Vector2 Normalized() {
        float f = forw - back;
        float r = right - left;
        return Vector2Normalize({f, r});
    }

    int UpDown() { return up - down; }

    bool IsEmpty() {
        return mouse_x == 0.f && mouse_y == 0.f && Normalized().x == 0 && Normalized().y == 0 && UpDown() == 0;
    }
};

struct GameEvent {
    EventId event_id; // for sending over the net
    std::variant<std::monostate, PlayerJoin, PlayerLeave, PlayerInput> data;
};

struct PlayerState {
    Vector3 position;
    Vector3 velocity;
    float yaw;
    float pitch;

    Vector3 VForward() const {
        return Vector3{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
    }

    Vector3 VRight() const {
        return Vector3{cos(yaw+PI/2) * cos(pitch), 0, sin(yaw+PI/2) * cos(pitch)};
    }

    void ApplyInput(PlayerInput input) {
        yaw += input.mouse_x;
        pitch += input.mouse_y;

        velocity += VForward()*input.Normalized().x;
        velocity += VRight()*input.Normalized().y;
        if (position.y == floor_lvl && input.up) {
            velocity.y = jump_impulse;
        }        
    }
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
    PlayerState self;
    uint32_t self_id;
    Model& model;
};

class Game : public GameBase<GameState, GameEvent, SerializedGameState> {
public:
    PlayerState InitNewPlayer(const GameState& state, uint32_t id) {
        PlayerState player;
        player.pitch = 0;
        player.yaw = 0;
        player.position = Vector3{0, 0, 0};
        player.velocity = Vector3{0, 0, 0};
        return player;
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
                    state.players[id].ApplyInput(input);
                }
            }
            break; 

        default:
            break;
        }
    }

    virtual void Draw(const GameState& state, const void* data) {}

    void DrawWorld(const GameState& state) {
        DrawGrid(100, 10);
    }

    void DrawPlayers(const GameState& state, const void* data) {
        const DrawingData* drawing_data = static_cast<const DrawingData*>(data);

        for (const auto& [id, player] : state.players) {
            if (id != drawing_data->self_id) {     
                DrawModelEx(drawing_data->model, player.position, Vector3{0, 1, 0}, -player.yaw*180/PI + 90, Vector3{10, 10, 10}, WHITE);
                DrawLine3D(player.position, player.position + player.VForward()*6, GREEN);                
                DrawLine3D(player.position, player.position + player.VRight()*6, BLUE);                
            }
        }
    }

    virtual void UpdateGameLogic(GameState& state) {
        for (auto& [id, player] : state.players) {
            player.velocity.y -= gravity*dt;
            player.position += player.velocity;
            player.position.y = fmax(floor_lvl, player.position.y);
            player.velocity *= 0.7;
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
                    //*out << "\tINPUT\t" << "X: " << input.GetX() << " up: " << input.up << std::endl;
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
                    lerped.players[id].position = Vector3Lerp(state1.players.at(id).position, state2.players.at(id).position, alpha);
                }
            }
        }
        return lerped;
    };

    virtual SerializedGameState Serialize(const GameState& state) {
        nlohmann::json j;
        for (const auto& [id, player] : state.players) {
            j["players"][std::to_string(id)] = {
                {"yw", player.yaw},
                {"pt", player.pitch},
                {"px", player.position.x},
                {"py", player.position.y},
                {"pz", player.position.z},
                {"vx", player.velocity.x},
                {"vy", player.velocity.y},
                {"vz", player.velocity.z},
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
            ps.yaw = player_json["yw"].get<float>();
            ps.pitch = player_json["pt"].get<float>();
            ps.position.x = player_json["px"].get<float>();
            ps.position.y = player_json["py"].get<float>();
            ps.position.z = player_json["pz"].get<float>();
            ps.velocity.x = player_json["vx"].get<float>();
            ps.velocity.y = player_json["vy"].get<float>();
            ps.velocity.z = player_json["vz"].get<float>();
            state.players[id] = ps;
        }
        return state;
    }
};

inline Camera GetCameraFromPlayer(const PlayerState& player) {
    Camera3D camera = { 0 };
    Vector3 cam_offset = {0, 5, 0};
    camera.position = player.position + cam_offset;
    camera.target = camera.position + player.VForward();
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}