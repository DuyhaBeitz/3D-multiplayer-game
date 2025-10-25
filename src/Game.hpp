#pragma once

#include "GameBase.hpp"
#include <variant>
#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <memory>

#include "World.hpp"
#include "Serialization.hpp"
#include "Deserialization.hpp"

constexpr int max_string_len = 2048;

constexpr int iters_per_sec = 60;
constexpr double dt = 1.f/iters_per_sec;

// client also uses that
constexpr uint32_t tick_period = iters_per_sec/20; // broadcast game state every 100 ms
constexpr uint32_t receive_tick_period = iters_per_sec; // allow late received events
constexpr uint32_t send_tick_period = tick_period; // sync client's tick with server's tick
constexpr uint32_t server_lateness = receive_tick_period;
// ensuring that we're not substructing bigger uint32_t from the smaller one
constexpr uint32_t max_lateness = server_lateness+tick_period+receive_tick_period;

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

struct GameState {
    std::map<uint32_t, PlayerData> players;
    WorldData world_data;

    bool PlayerExists(uint32_t id) const {
        return players.find(id) != players.end();
    }

    const PlayerData& GetPlayer(uint32_t id) const {
        if (PlayerExists(id)) return players.at(id);
        else throw std::runtime_error("Player doesn't exist");
    }

    const ActorData& GetActor(uint32_t id) const {
        return world_data.GetActor(GetPlayer(id).actor_key);
    }

    PlayerData& GetPlayer(uint32_t id) {
        if (PlayerExists(id)) return players.at(id);
        else throw std::runtime_error("Player doesn't exist");
    }

    ActorData& GetActor(uint32_t id) {
        return world_data.GetActor(GetPlayer(id).actor_key);
    }

    void ApplyInput(PlayerInput input, uint32_t id) {
        ActorData& actor_data = GetActor(id);
        auto fw = actor_data.VForward() * Vector3{1, 0, 1} * input.Normalized().x;
        auto rt = actor_data.VRight() * input.Normalized().y;
        
        actor_data.body.velocity += (fw + rt)*dt*hor_speed;

        actor_data.yaw += input.mouse_x;
        actor_data.pitch += input.mouse_y;
        actor_data.pitch = Clamp(actor_data.pitch, -PI/2*0.9, PI/2*0.9);
        
        if (actor_data.body.on_ground && input.up) {
            actor_data.body.velocity.y = jump_impulse;
            actor_data.body.on_ground = false;
        }        
    }
};

inline nlohmann::json SerializeGame(const GameState& state) {
    nlohmann::json j;

    j["players"] = nlohmann::json::array();
    for (const auto& [id, p] : state.players) {
        j["players"].push_back({id, SerializePlayer(p)});
    }

    j["world"] = SerializeWorld(state.world_data);
    return j;
}

inline GameState DeserializeGame(nlohmann::json j) {
    GameState state{};

    for (const auto& item : j["players"]) {
        if (!item.is_array() || item.size() != 2) continue;

        uint32_t id = item[0].get<uint32_t>();
        const nlohmann::json& player_json = item[1];
        
        state.players.insert({id, DeserializePlayer(player_json)});
    }

    state.world_data = DeserializeWorld(j["world"]);
    return state;
}

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
    uint32_t self_id;
    Model& model;
};

class Game : public GameBase<GameState, GameEvent, SerializedGameState> {
public:
    void InitNewPlayer(GameState& state, uint32_t id) {
        BodyData body_data;
        CollisionShape sphere(SphereData{13});
        body_data.shapes.push_back(sphere);
        ActorData actor_data(body_data);

        actor_data.pitch = 0;
        actor_data.yaw = 0;
        actor_data.model_key = R_MODEL_PLAYER;

        actor_data.body.position = Vector3{0, 10, 0};
        actor_data.body.velocity = Vector3{0, 0, 0};

        PlayerData player_data;
        player_data.actor_key = state.world_data.AddActor(actor_data);

        state.players[id] = player_data;
    }

    virtual void ApplyEvent(GameState& state, const GameEvent& event, uint32_t id) {
        switch (event.event_id) {
        case EV_PLAYER_JOIN:
            InitNewPlayer(state, id);
            break;

        case EV_PLAYER_LEAVE:
            state.world_data.RemoveActor(state.players.at(id).actor_key);
            break;

        case EV_PLAYER_INPUT:
            if (std::holds_alternative<PlayerInput>(event.data)) {
                auto input = std::get<PlayerInput>(event.data);
                if (state.world_data.ActorExists(state.players[id].actor_key)) {
                    state.ApplyInput(input, id);
                }
            }
            break; 

        default:
            break;
        }
    }

    virtual void Draw(const GameState& state, const void* data) {
        const ActorKey except_key = *static_cast<const ActorKey*>(data);
        state.world_data.Draw(except_key);
    }

    virtual void UpdateGameLogic(GameState& state) {
        state.world_data.Update(dt);
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

        const ActorKey* except_key = static_cast<const ActorKey*>(data);

        for (auto& [actor_key, actor] : state2.world_data.actors) {
            if (actor_key != *except_key) {
                if (state1.world_data.ActorExists(actor_key)) {
                    ActorData& actor_data = lerped.world_data.GetActor(actor_key);

                    actor_data.body.position = Vector3Lerp(
                        state1.world_data.GetActor(actor_key).body.position,
                        state2.world_data.GetActor(actor_key).body.position,
                        alpha
                    );
                    actor_data.body.UpdateShapePositions();
                }
            }
        }
        return lerped;
    };

    virtual SerializedGameState Serialize(const GameState& state) {
        return SerializedGameState(SerializeGame(state).dump().c_str());
    }

    GameState Deserialize(SerializedGameState data) {
        return DeserializeGame(nlohmann::json::parse(std::string(data.text, max_string_len)));
    }

    void InitGame(GameState& state) {
        { // floor
        BoxData box_data;
        box_data.half_extents = Vector3{100, 100, 100};

        BodyData body_data;
        body_data.position = Vector3{0, -100, 0};
        body_data.velocity = Vector3{0, 0, 0};
        body_data.inverse_mass = 0;
        body_data.shapes.push_back(CollisionShape(box_data));

        state.world_data.AddActor(ActorData(body_data));
        }

        {
        BoxData box_data;
        box_data.half_extents = Vector3{30, 5, 30};

        BodyData body_data;
        body_data.position = Vector3{40, 20, 0};
        body_data.shapes.push_back(CollisionShape(box_data));

        state.world_data.AddActor(ActorData(body_data));
        }
    }
};

inline Camera GetCameraFromActor(const ActorData& actor_data) {
    Camera3D camera = { 0 };
    Vector3 cam_offset = {0, 5, 0};
    camera.position = actor_data.body.position + cam_offset;
    camera.target = camera.position + actor_data.VForward();
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}