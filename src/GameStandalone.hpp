#pragma once

#include "Game.hpp"
#include "Rendering.hpp"
#include "WindowGlobal.hpp"

// placeholder for net id, for standalone there's one player
constexpr int player_id = 0;

class GameStandalone : public Game {
    
private:
    uint32_t m_tick = 0;
    GameState m_game_state{};
    
public:
    virtual void InitGame() {
        m_scene_manager.GetScene()->Setup();
        InitGameState(m_game_state);
    };

    GameStandalone() {
        Rendering::Init();
        m_scene_manager.GetScene()->Load();
        InitGame();
        
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_JOIN;
        AddEvent(game_event, player_id, m_tick+19);
    }

    void Update(GameInput input) {
        if (input.ui_input.toggle_cursor_pressed){
            WindowGlobal::Get().ToggleCursor();   
        }

        if (!input.player_input.IsEmpty()) {
            GameEvent event;
            event.event_id = EV_PLAYER_INPUT;
            event.data = input.player_input;
            AddEvent(event, player_id, m_tick);
        }
        
        m_game_state = ApplyEvents(m_game_state, m_tick, m_tick+1); 
        m_tick++;

        Scenes scene = m_scene_manager.GetScene()->CheckSceneChange(m_game_state);
        if (scene != Scenes::None) {
            m_scene_manager.ChangeScene(scene);
            InitGame();
        }
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