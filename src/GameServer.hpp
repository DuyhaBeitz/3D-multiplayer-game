#pragma once

#include <EasyNet/EasyNetServer.hpp>
#include "shared.hpp"

class GameServer : public Game{
private:
    uint32_t m_tick;
    GameState m_late_game_state;
    GameState m_game_state;
    std::shared_ptr<EasyNetServer> m_server;

public:

    GameServer() {
        m_server = std::make_shared<EasyNetServer>();
        m_server->CreateServer(server_port);
        
        m_server->SetOnConnect([this](ENetEvent event){this->OnConnect(event);});
        m_server->SetOnDisconnect([this](ENetEvent event){this->OnDisconnect(event);});
        m_server->SetOnReceive([this](ENetEvent event){this->OnRecieve(event);});

        InitGame(m_late_game_state);
    }

    void Update() {
        m_server->Update();

        if (m_tick % tick_period == 0 && m_tick >= max_lateness) {
            uint32_t current_tick = m_tick-server_lateness;

            uint32_t previous_tick = current_tick - tick_period;
            uint32_t current_old_tick = current_tick - receive_tick_period;
            uint32_t previous_old_tick = previous_tick - receive_tick_period;

            m_late_game_state = ApplyEvents(m_late_game_state, previous_old_tick, current_old_tick);
            m_game_state = ApplyEvents(m_late_game_state, current_old_tick, current_tick);

            char buffer[max_string_len];
            SerializedGameState data = Serialize(m_game_state);
            data.tick = current_tick;

            ENetPacket* packet = CreatePacket<SerializedGameState>(MSG_GAME_STATE, data);
            m_server->Broadcast(packet); 
            DropEventHistory(previous_old_tick);
        }
        m_tick++;
    }

    void OnConnect(ENetEvent event) {
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_JOIN;
        uint32_t id = enet_peer_get_id(event.peer);
        AddEvent(game_event, id, m_tick);
        m_server->SendTo(id, CreatePacket<uint32_t>(MSG_GAME_TICK, m_tick));
        m_server->SendTo(id, CreatePacket<uint32_t>(MSG_PLAYER_ID, id));
    }

    void OnDisconnect(ENetEvent event) {
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_LEAVE;
        uint32_t id = enet_peer_get_id(event.peer);
        AddEvent(game_event, id, m_tick);
    }

    void OnRecieve(ENetEvent event)
    {
        MessageType msgType = ExtractMessageType(event.packet);
        switch (msgType) {
        case MSG_PLAYER_INPUT:
            {
            PlayerInputPacketData recieved = ExtractData<PlayerInputPacketData>(event.packet);
            uint32_t id = enet_peer_get_id(event.peer);

            GameEvent game_event;
            game_event.event_id = EV_PLAYER_INPUT;
            game_event.data = recieved.input;

            AddEvent(game_event, id, recieved.tick);
            }
            break;

        default:
            break;
        }
    }

    void DrawGame() {
        Rendering::Get().BeginRendering();
        ClearBackground(DARKGRAY);
        
        float r = 200;
        Rendering::Get().SetCamera(
            GetCameraFromPos(Vector3{r, r, r}, Vector3{0, 0, 0})
        );

        const ActorKey except_key = 100;
        Rendering::Get().BeginCameraMode();
            Draw(m_game_state, &except_key);
        Rendering::Get().EndCameraMode();
        
        Rendering::Get().EndRendering();
    }
};