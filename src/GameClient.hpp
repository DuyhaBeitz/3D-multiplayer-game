#pragma once

#include <EasyNet/EasyNetClient.hpp>
#include "shared.hpp"
#include "Rendering.hpp"

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

public:

    std::shared_ptr<EasyNetClient> GetNetClient() { return m_client; }

    GameClient() {
        m_client = std::make_shared<EasyNetClient>();
        m_client->CreateClient();
        m_client->SetOnReceive([this](ENetEvent event){OnReceive(event);});
        m_client->SetOnConnect([this](ENetEvent){m_connected = true;});
        m_client->SetOnDisconnect([this](ENetEvent){m_connected = false;});

        Rendering::Init();
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
            AddEvent(event, m_id, m_tick);

            PlayerInputPacketData data;
            data.input = input;
            data.tick = m_tick;
            m_client->SendPacket(CreatePacket<PlayerInputPacketData>(MSG_PLAYER_INPUT, data, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT));
        }
        
        m_self_game_state = ApplyEvents(m_self_game_state, m_tick, m_tick+1);
        float alpha = float(m_ticks_since_last_recieved_game) / float(m_last_received_game_tick-m_prev_last_received_game_tick);
        m_others_game_state = Lerp(m_prev_last_received_game, m_last_received_game, alpha, &m_id);

        m_tick++;
        m_ticks_since_last_recieved_game++;
    }

    void DrawGame() {            
        BeginDrawing();
        if (m_self_game_state.PlayerExists(m_id)) {
            const PlayerData& player_data = m_self_game_state.GetPlayer(m_id);
            if (m_self_game_state.world_data.ActorExists(player_data.actor_key)) {
                Rendering::Get().SetCamera(
                    GetCameraFromActor(m_self_game_state.GetActor(m_id))
                );
                const ActorKey except_key = player_data.actor_key + 100;
                
                Rendering::Get().BeginRendering();
                    //ClearBackground(DARKGRAY);
                    // DrawText(std::to_string(m_tick).c_str(), 100, 100, 64, WHITE);
                    // DrawText(("roundtrip: " + std::to_string(m_client->GetPeer()->roundTripTime) + "ms").c_str(), 100, 200, 64, WHITE);
                    Draw(m_others_game_state, &except_key);
                Rendering::Get().EndRendering();
            }     
        }
        EndDrawing();
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
            m_ticks_since_last_recieved_game = 0;
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

        default:
            break;
        }
    }

    bool IsConnected() {return m_connected;}
};