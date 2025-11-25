#pragma once

#include <EasyNet/EasyNetClient.hpp>
#include "shared.hpp"
#include "Rendering.hpp"
#include "Chat.hpp"

#include <RaylibRetainedGUI/RaylibRetainedGUI.hpp>

class GameClient : public Game {
private:
    uint32_t m_tick = 0;
    uint32_t m_id = 0;
    std::shared_ptr<EasyNetClient> m_client;

    uint32_t m_ticks_since_last_received_game = 0;

    GameState m_last_received_game{};
    uint32_t m_last_received_game_tick = 0;

    GameState m_prev_last_received_game{};
    uint32_t m_prev_last_received_game_tick = 0;    
    
    GameState m_others_game_state{};
    GameState m_self_game_state{};

    uint32_t CalculateTickWinthPing(uint32_t tick) {
        float delta_sec = m_client->GetPeer()->roundTripTime / 2 / 1000;
        uint32_t delta_tick = delta_sec * iters_per_sec;
        return tick + delta_tick;
    }

    bool m_connected = false;

    Chat m_chat;
    std::shared_ptr<UIScreen> m_ui_screen;
    std::string m_new_chat_text;
    bool m_chat_entering = false;
    std::shared_ptr<UIElement> m_chat_ui;
    std::shared_ptr<UIStringButton> m_text_input_box;

    std::string m_name_buffer;
    std::shared_ptr<UIStringButton> m_name_input_box;

public:

    std::shared_ptr<EasyNetClient> GetNetClient() { return m_client; }

    GameClient() {
        Rendering::Init();
        InitGame(m_self_game_state);
        InitGame(m_others_game_state);

        m_client = std::make_shared<EasyNetClient>();
        m_client->CreateClient();
        m_client->SetOnReceive([this](ENetEvent event){OnReceive(event);});
        m_client->SetOnConnect([this](ENetEvent){m_connected = true;});
        m_client->SetOnDisconnect([this](ENetEvent){m_connected = false;});

        m_ui_screen = std::make_shared<UIScreen>();
        m_chat_ui = std::make_shared<UIElement>();
        
        m_text_input_box = std::make_shared<UIStringButton>(&m_new_chat_text, Rectangle{0, 0, 1, 1});
        m_name_input_box = std::make_shared<UIStringButton>(&m_name_buffer, Rectangle{0, 0, 1, 1});

        auto apply_button = std::make_shared<UIFuncButton>("Apply name");
        apply_button->BindOnReleased([this](){
            TextPacketData data(m_name_buffer.c_str());
            m_client->SendPacket(CreatePacket<TextPacketData>(MSG_NAME_CHANGE, data));
        });

        auto split_h1 = std::make_shared<UISplit>(apply_button, m_name_input_box, 0.25);
        auto split_v = std::make_shared<UISplit>(split_h1, m_text_input_box, 0.1, UI_FULL_RECT, false);

        m_chat_ui->AddChild(split_v);
    }

    void CloseChat() {
        m_chat_ui->Close();
        DisableCursor();
    }
    void OpenChat() {
        m_chat_ui->Open();
        m_ui_screen->AddChild(m_chat_ui);
        EnableCursor();
    }

    void ToggleChat() {
        if (m_chat_entering) {
            CloseChat();
        }
        else {
            OpenChat();
        }
        m_chat_entering = !m_chat_entering;
    }

    void Update(GameInput input) {
        if (input.ui_input.toggle_window_pressed) {
            ToggleWindow();
        }
        if (input.ui_input.toggle_chat_pressed) {
            ToggleChat();
        }
        if (input.ui_input.enter_chat_pressed) {
            if (m_chat_entering) {
                if (m_new_chat_text.size() > 0) {
                    TextPacketData data(m_new_chat_text.c_str());
                    m_client->SendPacket(CreatePacket<TextPacketData>(MSG_CHAT_MESSAGE, data));
                    m_text_input_box->Clear();
                }
            }
        }
        if (input.ui_input.toggle_cursor_pressed){
            if (!m_chat_entering) {
                ToggleCursor();
            }
        }

        if (!input.player_input.IsEmpty() && !m_chat_entering) {
            GameEvent event;
            event.event_id = EV_PLAYER_INPUT;
            event.data = input.player_input;
            AddEvent(event, m_id, m_tick);

            PlayerInputPacketData data;
            data.input = input.player_input;
            data.tick = m_tick;
            m_client->SendPacket(CreatePacket<PlayerInputPacketData>(MSG_PLAYER_INPUT, data));
        }

        m_self_game_state = ApplyEvents(m_self_game_state, m_tick, m_tick+1);
        float alpha = float(m_ticks_since_last_received_game) / float(m_last_received_game_tick-m_prev_last_received_game_tick);
        
        std::set<ActorKey> except_keys = {};
        m_others_game_state = Lerp(m_prev_last_received_game, m_last_received_game, alpha, &except_keys);

        m_tick++;
        m_ticks_since_last_received_game++;

        m_ui_screen->Update(nullptr);
    }

    void DrawGame() {            
        if (m_self_game_state.PlayerExists(m_id)) {
            const PlayerData& player_data = m_self_game_state.GetPlayer(m_id);
            if (m_self_game_state.world_data.ActorExists(player_data.actor_key)) {
                Rendering::Get().SetCamera(
                    GetCameraFromActor(m_self_game_state.GetActor(m_id))
                );
                GameDrawingData drawing_data{
                    {player_data.actor_key},
                    m_game_metadata
                };
                
                Rendering::Get().BeginRendering();
                    std::set<ActorKey> except_keys = {};
                    for (auto& [id, player] : m_others_game_state.players) {
                        if (id != m_id) except_keys.insert(player.actor_key);
                    }

                    // everything is client-predicted, except other players - they are lerped
                    GameState smooth = Lerp(m_others_game_state, m_self_game_state, 1.f, &except_keys);
                    Draw(smooth, drawing_data);
                Rendering::Get().EndRendering();
            }     
        }
        

        Rendering::Get().EnableCameraBasic();
        //Rendering::Get().DrawPrimitives();
        Rendering::Get().DrawTexts();
        Rendering::Get().DisableCameraBasic();
        m_ui_screen->Draw();
        //DrawText(std::to_string(m_tick).c_str(), 100, 100, 64, WHITE);
        DrawText(("roundtrip: " + std::to_string(m_client->GetPeer()->roundTripTime) + "ms").c_str(), 100, GetScreenHeight()-128, 64, WHITE);
        m_chat.Draw();        
    }

    void OnReceive(ENetEvent event) {
        MessageType msgType = ExtractMessageType(event.packet);
        switch (msgType) {
        case MSG_GAME_TICK:
            m_tick = CalculateTickWinthPing(ExtractData<uint32_t>(event.packet));
            break;

        case MSG_PLAYER_ID:
            m_id = ExtractData<uint32_t>(event.packet);
            break;
            
        case MSG_GAME_STATE:
            {
            m_ticks_since_last_received_game = 0;
            m_prev_last_received_game = m_last_received_game;
            m_prev_last_received_game_tick = m_last_received_game_tick;

            SerializedGameState data = ExtractData<SerializedGameState>(event.packet);
            auto rec_state = Deserialize(data);
            m_self_game_state = ApplyEvents(rec_state, data.tick, m_tick-1);
            DropEventHistory(data.tick-max_lateness);
                        
            m_last_received_game = rec_state;
            m_last_received_game_tick = data.tick;       
            }
            break;

        case MSG_CHAT_MESSAGE:
            {
                /*
                Server receives text,
                all clients except the one who's message that is receive full ChatMessage
                */
                auto&& [message, id] = ExtractDataWithID<ChatMessage>(event.packet);
                m_chat.AddMessage(message);
            }
            break;
            
        case MSG_GAME_METADATA:
            {
                SerializedGameMetadata received = ExtractData<SerializedGameMetadata>(event.packet);
                m_game_metadata.Deserialize(received);
            }
            break;
        default:
            break;
        }
    }

    bool IsConnected() {return m_connected;}
};