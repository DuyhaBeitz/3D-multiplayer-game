#pragma once
#include "shared.hpp"
#include "Resources.hpp"

// placeholder for net id, for standalone there's one player
constexpr int player_id = 0;

class GameStandalone : Game {
    
private:
    uint32_t m_tick;
    GameState m_game_state;
    
public:

    GameStandalone() {
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_JOIN;
        AddEvent(game_event, player_id, m_tick);

        Resources::Init();
    }

    void Update() {
        if (IsKeyPressed(KEY_L)){
            if (IsCursorHidden()){
                EnableCursor();
            }
            else {
                DisableCursor();
            }
        }

        PlayerInput input;
        input.Detect();

        if (!input.IsEmpty()) {
            GameEvent event;
            event.event_id = EV_PLAYER_INPUT;
            event.data = input;
            AddEvent(event, player_id, m_tick);
        }
        
        m_game_state = ApplyEvents(m_game_state, m_tick, m_tick+1); 
        m_tick++;
    }

    void DrawGame() {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        DrawingData drawing_data = {m_game_state.players.at(player_id), player_id, Resources::Get().ModelFromKey(R_MODEL_PLAYER)};
        Camera3D camera = { 0 };
        Vector3 cam_offset = {0, 5, 0};
        camera.position = drawing_data.self.position + cam_offset;
        camera.target = camera.position + drawing_data.self.VForward();
        camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
        camera.fovy = 90.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        BeginMode3D(camera);
        DrawGrid(100, 10);
        EndMode3D();
        
        EndDrawing();       
    }
};