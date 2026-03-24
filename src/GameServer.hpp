#pragma once

#include <EasyNet/EasyNetServer.hpp>
#include "Chat.hpp"
#include "shared.hpp"

constexpr uint32_t tick_period = iters_per_sec/20; // broadcast game state every 100 ms
constexpr uint32_t send_tick_period = tick_period; // sync client's tick with server's tick

// the server runs in the present
// but doesn't broadcast new world state right away, instead broadcasts late game state
// it waits for late inputs and applies them retroactively
// this variable is pretty much the maximum roundtrip (in ticks) that is still playable just fine
// a bit more and none of the client packets will be applied
// it's also how much players will see each other in the past
// so there needs to be some balance, between allowing big ping and providing clients with not too old state
constexpr uint32_t server_lateness = iters_per_sec/2;

// ensuring that we're not substructing bigger uint32_t from the smaller one
constexpr uint32_t max_lateness = tick_period+server_lateness;

constexpr uint32_t broadcast_game_metadata_tick_period = iters_per_sec*2;

class GameServer : public Game{
private:
    uint32_t m_tick = 0;
    GameState m_late_game_state{}; // is used to reconcile with late inputs from clients
    GameState m_game_state{};
    std::shared_ptr<EasyNetServer> m_server;
    Chat m_chat{};

public:

    GameServer() {
        InitGame(m_late_game_state);

        m_server = std::make_shared<EasyNetServer>();
        m_server->CreateServer(server_port);
        
        m_server->SetOnConnect([this](ENetEvent event){this->OnConnect(event);});
        m_server->SetOnDisconnect([this](ENetEvent event){this->OnDisconnect(event);});
        m_server->SetOnReceive([this](ENetEvent event){this->OnReceive(event);});
    }

    void Update() {
        m_server->Update();

/*
                                       'current_old_tick   'current_tick
ticks: |---|---|---|---|---|---|---|---|---|---|---|---|---|
           ^previous_old_tick  ^previous_tick
*/

        if (m_tick % tick_period == 0 && m_tick >= max_lateness) {
            uint32_t current_tick = m_tick;

            uint32_t previous_tick = current_tick - tick_period;
            uint32_t current_old_tick = current_tick - server_lateness;
            uint32_t previous_old_tick = previous_tick - server_lateness;

            m_late_game_state = ApplyEvents(m_late_game_state, previous_old_tick, current_old_tick);
            m_game_state = ApplyEvents(m_late_game_state, current_old_tick, current_tick);

            SerializedGameState data = Serialize(m_late_game_state);
            data.tick = current_old_tick;

            ENetPacket* packet = CreatePacket<SerializedGameState>(MSG_GAME_STATE, data, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
            m_server->Broadcast(packet); 
            DropEventHistory(previous_old_tick);
        }
        if (m_tick % broadcast_game_metadata_tick_period == 0 && m_tick >= max_lateness) {
            BroadcastMetadata();
            m_server->Broadcast(CreatePacket<uint32_t>(MSG_GAME_TICK, m_tick));
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
        AddAndSyncChatMessage(server_chat_name, TextFormat("Player joined"));
    }

    void OnDisconnect(ENetEvent event) {
        GameEvent game_event;
        game_event.event_id = EV_PLAYER_LEAVE;
        uint32_t id = enet_peer_get_id(event.peer);
        AddEvent(game_event, id, m_tick);
        AddAndSyncChatMessage(server_chat_name, TextFormat("%s left", m_game_metadata.GetPlayerName(id)));
    }

    void OnReceive(ENetEvent event)
    {
        MessageType msgType = ExtractMessageType(event.packet);
        switch (msgType) {
        case MSG_PLAYER_INPUT:
            {
                PlayerInputPacketData received = ExtractData<PlayerInputPacketData>(event.packet);
                uint32_t id = enet_peer_get_id(event.peer);

                GameEvent game_event;
                game_event.event_id = EV_PLAYER_INPUT;
                game_event.data = received.input;

                AddEvent(game_event, id, received.tick);
            }
            break;

        case MSG_CHAT_MESSAGE:
            {
                /*
                Server receives text,
                all clients except the one who's message that is receive full ChatMessage
                */
                const TextPacketData& received = ExtractData<TextPacketData>(event.packet);
                uint32_t id = enet_peer_get_id(event.peer);
            
                AddAndSyncChatMessage(m_game_metadata.GetPlayerName(id), received.text);
            }
            break;
        case MSG_NAME_CHANGE:
            {
                const TextPacketData& received = ExtractData<TextPacketData>(event.packet);
                uint32_t id = enet_peer_get_id(event.peer);
                m_game_metadata.SetPlayerName(id, received.text);
                BroadcastMetadata();
            }
        default:
            break;
        }
    }

    void BroadcastMetadata() {
        SerializedGameMetadata serialized = m_game_metadata.Serialize();
        ENetPacket* packet = CreatePacket(MSG_GAME_METADATA, serialized);
        m_server->Broadcast(packet);
    }

    void AddAndSyncChatMessage(const char* name, const char* text) {
        ChatMessage message;
        std::snprintf(message.name, max_player_name_len, "%s", name);
        std::snprintf(message.text, max_string_len, "%s", text);

        m_chat.AddMessage(message);

        ENetPacket* packet = CreatePacket(MSG_CHAT_MESSAGE, message);
        m_server->Broadcast(packet);
    }

#if WITH_RENDER
    void DrawGame() {
        float r = 200;
        Rendering::Get().SetCamera(
            GetCameraFromPos(Vector3{r, r, r}, Vector3{0, 0, 0})
        );

        GameDrawingData drawing_data{
            {},
            m_game_metadata
        };
        Rendering::Get().BeginRendering();
            ClearBackground(DARKGRAY);
            Draw(m_game_state, drawing_data);
        Rendering::Get().EndRendering();

        Rendering::Get().EnableCameraBasic();
        Rendering::Get().DrawPrimitives();
        Rendering::Get().DrawTexts();
        Rendering::Get().DisableCameraBasic();
        m_chat.Draw();

        DrawText(("tick: " + std::to_string(m_tick)).c_str(), 100, 128+64, 64, WHITE);
    }
#endif
};