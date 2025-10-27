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
        InitGame(m_game_state);
        
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_JOIN;
        AddEvent(game_event, player_id, m_tick+19);

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
        Rendering::Get().BeginRendering();
        ClearBackground(DARKGRAY);
        
        if (m_game_state.PlayerExists(player_id)) {
            Rendering::Get().SetCamera(
                GetCameraFromActor(m_game_state.GetActor(player_id))
            );

            Rendering::Get().BeginCameraMode();
            ActorKey except_key = m_game_state.GetPlayer(player_id).actor_key;
            Draw(m_game_state, &except_key);
            Rendering::Get().EndCameraMode();
        }     
        Rendering::Get().EndRendering();

    }
};