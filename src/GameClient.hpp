#pragma once

#include <EasyNet/EasyNetClient.hpp>
#include "shared.hpp"

class GameClient : public Game {
private:
    uint32_t m_tick = 0;
    uint32_t m_id = 0;
    std::shared_ptr<EasyNetClient> m_client;

    uint32_t m_ticks_since_last_recieved_game = 0;

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

    Model m_model;

public:

    std::shared_ptr<EasyNetClient> GetNetClient() { return m_client; }

    GameClient() {
        m_model = LoadModel("assets/model.glb");

        m_client = std::make_shared<EasyNetClient>();
        m_client->CreateClient();
        m_client->SetOnReceive([this](ENetEvent event){OnReceive(event);});
        m_client->SetOnConnect([this](ENetEvent){m_connected = true;});
        m_client->SetOnDisconnect([this](ENetEvent){m_connected = false;});
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
            
            if (!IsCursorHidden()){
                input.mouse_x = 0;
                input.mouse_y = 0;    
            }
            else {
                input.mouse_x = GetMouseDelta().x / 450;
                input.mouse_y = -GetMouseDelta().y / 450;
            }
            input.right = IsKeyDown(KEY_D);
            input.left = IsKeyDown(KEY_A);
            input.forw = IsKeyDown(KEY_W);
            input.back = IsKeyDown(KEY_S);
            input.up = IsKeyDown(KEY_SPACE);
            input.down = IsKeyDown(KEY_LEFT_CONTROL);

            if (!input.IsEmpty()) {
                GameEvent event;
                event.event_id = EV_PLAYER_INPUT;
                event.data = input;
                AddEvent(event, m_id, m_tick);

                PlayerInputPacketData data;
                data.input = input;
                data.tick = m_tick;
                m_client->SendPacket(CreatePacket<PlayerInputPacketData>(MSG_PLAYER_INPUT, data, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT));
            }
            
            m_self_game_state = ApplyEvents(m_self_game_state, m_tick, m_tick+1);
            float alpha = float(m_ticks_since_last_recieved_game) / float(m_last_received_game_tick-m_prev_last_received_game_tick);
            m_others_game_state = Lerp(m_prev_last_received_game, m_last_received_game, alpha, &m_id);

            // if (GetTime() > 4) {
            //     game_manager.OutputHistory();

            //     GameState state = {};
            //     char buffer[max_string_len];
            //     SerializeGameState(game_manager.ApplyEvents(first_received_game, first_received_game_tick, last_received_tick), buffer, max_string_len);
            //     std::cout << last_received_tick << std::endl;
            //     std::cout << std::endl << buffer << std::endl;
            //     CloseWindow();
            // }


            m_tick++;
            m_ticks_since_last_recieved_game++;
    }

    void DrawGame() {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawText(std::to_string(m_tick).c_str(), 100, 100, 64, WHITE);
        DrawText(("roundtrip: " + std::to_string(m_client->GetPeer()->roundTripTime) + "ms").c_str(), 100, 200, 64, WHITE);
        
        if (m_self_game_state.players.find(m_id) != m_self_game_state.players.end()) {
            DrawingData drawing_data = {m_self_game_state.players.at(m_id), m_id, m_model};
            Draw(m_others_game_state, &drawing_data);
        }

        
        EndDrawing();
    }

    void OnReceive(ENetEvent event) {
        MessageType msgType = static_cast<MessageType>(event.packet->data[0]);
        switch (msgType) {
        case MSG_GAME_TICK:
            m_tick = CalculateTickWinthPing(ExtractData<uint32_t>(event.packet));
            break;

        case MSG_PLAYER_ID:
            m_id = ExtractData<uint32_t>(event.packet);
            break;
            
        case MSG_GAME_STATE:
            {
            m_ticks_since_last_recieved_game = 0;
            m_prev_last_received_game = m_last_received_game;
            m_prev_last_received_game_tick = m_last_received_game_tick;

            SerializedGameState data = ExtractData<SerializedGameState>(event.packet);
            auto rec_state = Deserialize(data);
            m_self_game_state = ApplyEvents(rec_state, data.tick, m_tick-1);
            DropEventHistory(data.tick);
            
            m_last_received_game = rec_state;
            m_last_received_game_tick = data.tick;       
            }
            break;

        default:
            break;
        }
    }

    bool IsConnected() {return m_connected;}
};