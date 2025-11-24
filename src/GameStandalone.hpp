#pragma once
#include "Game.hpp"
#include "Resources.hpp"

// placeholder for net id, for standalone there's one player
constexpr int player_id = 0;

class GameStandalone : public Game {
    
private:
    uint32_t m_tick;
    GameState m_game_state;
    
public:

    GameStandalone() {
        Resources::Init();
        InitGame(m_game_state);
        
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_JOIN;
        AddEvent(game_event, player_id, m_tick+19);
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
        if (m_game_state.PlayerExists(player_id)) {
            Rendering::Get().SetCamera(
                GetCameraFromActor(m_game_state.GetActor(player_id))
            );

            GameDrawingData drawing_data{
                {m_game_state.GetPlayer(player_id).actor_key},
                m_game_metadata
            };

            Rendering::Get().BeginRendering();
                ClearBackground(DARKGRAY);
                Draw(m_game_state, drawing_data);
            Rendering::Get().EndRendering();
        } 
        Rendering::Get().EnableCameraBasic();
        Rendering::Get().DrawPrimitives();
        Rendering::Get().DrawTexts();
        Rendering::Get().DisableCameraBasic();
        DrawFPS(100, 100);
    }
};