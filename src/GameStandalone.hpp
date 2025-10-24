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
        
        DrawingData drawing_data = {player_id, Resources::Get().ModelFromKey(R_MODEL_PLAYER)};
        Camera3D camera = GetCameraFromActor(m_game_state.world_data.actors.at(player_id));

        BeginMode3D(camera);
        Draw(m_game_state, &drawing_data);
        EndMode3D();
        
        EndDrawing();       
    }
};