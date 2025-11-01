#pragma once

#include "GameBase.hpp"
#include <variant>
#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <memory>

#include "World.hpp"
#include "Serialization.hpp"

#include "Constants.hpp"

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


    template <class Archive>
    void serialize(Archive& ar) {
        ar(players, world_data);
    }
};

struct DrawingData {
    uint32_t self_id;
    Model& model;
};

struct SerializedGameState {
    uint32_t tick;
    uint32_t size;          // number of valid bytes
    uint8_t bytes[4096];    // max packet size
};

class Game : public GameBase<GameState, GameEvent, SerializedGameState> {
public:
    void InitNewPlayer(GameState& state, uint32_t id) {
        BodyData body_data;
        CollisionShape sphere(SphereData{13});
        body_data.shapes.push_back(sphere);
        ActorData actor_data(body_data, R_MODEL_PLAYER);
        
        actor_data.render_data.offset = {0, -12, 0};

        actor_data.body.position = Vector3{0, 10, 0};

        PlayerData player_data;
        player_data.actor_key = state.world_data.AddActor(actor_data);
        
        snprintf(
            state.world_data.GetActor(player_data.actor_key).name,
            sizeof(state.world_data.GetActor(player_data.actor_key).name),
            "player%d",
            state.players.size());

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
        SerializedGameState sgs{};
        
        // Serialize into a temporary stream
        std::ostringstream os(std::ios::binary);
        {
            cereal::BinaryOutputArchive archive(os);
            archive(state);
        }

        // Copy into the fixed-size buffer
        std::string str = os.str();
        sgs.size = static_cast<uint32_t>(str.size());
        if (sgs.size > sizeof(sgs.bytes)) {
            throw std::runtime_error("Serialized state exceeds buffer size");
        }
        std::memcpy(sgs.bytes, str.data(), sgs.size);

        return sgs;
    }

    GameState Deserialize(SerializedGameState data) {
        // Wrap the raw buffer in a stringstream for Cereal
        std::istringstream is(std::string(reinterpret_cast<const char*>(data.bytes), data.size),
                            std::ios::binary);

        cereal::BinaryInputArchive archive(is);
        GameState gs;
        archive(gs);
        return gs;
    }

    void InitGame(GameState& state) {
        { // floor
        BoxData box_data;
        box_data.half_extents = Vector3{1000, 100, 1000};

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
        
        {
        BoxData box_data;
        box_data.half_extents = Vector3{12, 12, 12};

        BodyData body_data;
        body_data.position = Vector3{0, 20, 40};
        body_data.shapes.push_back(CollisionShape(box_data));

        ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
        state.world_data.GetActor(actor_key).render_data.model_key = R_MODEL_CUBE_EXCLAMATION;
        }

        for (int i = 0; i < 6; i++) {
            SphereData sphere_data;
            sphere_data.radius = 10;

            BodyData body_data;
            body_data.position = Vector3{40, 20.f*i, 40};
            body_data.shapes.push_back(CollisionShape(sphere_data));

            ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
        }
    }
};

inline Camera GetCameraFromPos(Vector3 pos, Vector3 target) {
    Camera3D camera = { 0 };
    Vector3 cam_offset = {0, 5, 0};
    camera.position = pos;
    camera.target = target;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;    
}

inline Camera GetCameraFromActor(const ActorData& actor_data) {
    Vector3 cam_offset = {0, 5, 0};
    Vector3 position = actor_data.body.position + cam_offset;
    Vector3 target =  position + actor_data.VForward();

    // Vector3 target =  actor_data.body.position + cam_offset;
    // Vector3 position = target - actor_data.VForward()*130;
    

    return GetCameraFromPos(position, target);
}