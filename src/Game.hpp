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
    EV_PLAYER_INPUT
};

// Event data
struct PlayerJoin {};
struct PlayerLeave {};
struct PlayerInput {
    float mouse_x = 0.f;
    float mouse_y = 0.f;
    bool right = false;
    bool left = false;
    bool forw = false;
    bool back = false;
    bool up = false;
    bool down = false;
  
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

    void ClearNonContinuous() {
    }

    void Divide(float denum) {
        mouse_x /= denum;
        mouse_y /= denum;
    }
};

struct UIInput {
    bool toggle_cursor_pressed = false;
    bool toggle_chat_pressed   = false;
    bool enter_chat_pressed    = false;
    bool disconnect_pressed    = false;

    void Detect() {
        toggle_cursor_pressed = IsKeyPressed(KEY_L);
        toggle_chat_pressed   = IsKeyPressed(KEY_TAB);
        enter_chat_pressed    = IsKeyPressed(KEY_ENTER);
        disconnect_pressed    = IsKeyPressed(KEY_MINUS);
    }

    void ClearNonContinuous() {
        toggle_cursor_pressed = false;
        toggle_chat_pressed   = false;
        enter_chat_pressed    = false;
        disconnect_pressed    = false;
    }

    void Divide(float denum) {}
};

struct GameInput {
    UIInput ui_input{};
    PlayerInput player_input{};
    void Detect() {
        ui_input.Detect();
        player_input.Detect();
    }
    void ClearNonContinuous() {
        ui_input.ClearNonContinuous();
        player_input.ClearNonContinuous();
    }

    void Divide(float denum) {
        ui_input.Divide(denum);
        player_input.Divide(denum);
    }
};

struct GameEvent {
    EventId event_id{}; // for sending over the net
    std::variant<std::monostate, PlayerJoin, PlayerLeave, PlayerInput> data{};
};

struct GameState {
    std::map<uint32_t, PlayerData> players{};
    WorldData world_data{};

#if WITH_RENDER
    void Draw(GameDrawingData &drawing_data) const;
#endif

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

struct SerializedGameState {
    uint32_t tick;
    uint32_t size;          // number of valid bytes
    uint8_t bytes[4096*2];    // max packet size
};

class GameDrawingData;

class Game : public GameBase<GameState, GameEvent, SerializedGameState> {
protected:
    GameMetadata m_game_metadata;

public:
    void InitNewPlayer(GameState& state, uint32_t id);

    virtual void ApplyEvent(GameState& state, const GameEvent& event, uint32_t id);

    virtual void Draw(const GameState& state, GameDrawingData& data);

    virtual void UpdateGameLogic(GameState& state) {
        state.world_data.Update(dt, m_game_metadata);
    }

    virtual GameState Lerp(const GameState& state1, const GameState& state2, float alpha, const void* data);

    virtual SerializedGameState Serialize(const GameState& state);
    GameState Deserialize(SerializedGameState data);

    void InitGame(GameState& state);
};

Camera GetCameraFromPos(Vector3 pos, Vector3 target);
Camera GetCameraFromActor(const ActorData& actor_data);
