#pragma once

#include <EasyNet/EasyNetServer.hpp>
#include "Chat.hpp"
#include "shared.hpp"

constexpr uint32_t tick_period = iters_per_sec/10; // broadcast game state every 100 ms
constexpr uint32_t send_tick_period = iters_per_sec/2; // sync client's tick with server's tick
constexpr uint32_t server_lateness = iters_per_sec;
// ensuring that we're not substructing bigger uint32_t from the smaller one
constexpr uint32_t max_lateness = server_lateness+tick_period;

constexpr uint32_t broadcast_game_metadata_tick_period = iters_per_sec/2;

class GameServer : public Game{
private:
    uint32_t m_tick = 0;
    GameState m_game_state{};
    std::shared_ptr<EasyNetServer> m_server;
    Chat m_chat{};

public:
    virtual void InitGame() {
        m_scene_manager.GetScene()->Setup();
        InitGameState(m_game_state);
    };

    GameServer() {
        m_scene_manager.GetScene()->Load();
        InitGame();

        m_server = std::make_shared<EasyNetServer>();
        m_server->CreateServer(server_port);
        
        m_server->SetOnConnect([this](ENetEvent event){this->OnConnect(event);});
        m_server->SetOnDisconnect([this](ENetEvent event){this->OnDisconnect(event);});
        m_server->SetOnReceive([this](ENetEvent event){this->OnReceive(event);});
    }

    void Update() {
        if (m_tick % broadcast_game_metadata_tick_period == 0 && m_tick >= max_lateness) {
            UpdateMetadata();
            BroadcastMetadata();
            m_server->Broadcast(CreatePacket<uint32_t>(NetMsg::GAME_TICK, m_tick));
        }

        if (m_tick % tick_period == 0 && m_tick >= max_lateness) {
            uint32_t current_tick = m_tick-server_lateness;
            uint32_t prev_tick = current_tick-tick_period;

            m_game_state = ApplyEvents(m_game_state, prev_tick, current_tick);

            SerializedGameState data = Serialize(m_game_state);
            data.tick = current_tick;

            ENetPacket* packet = CreatePacket<SerializedGameState>(NetMsg::GAME_STATE, data, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
            m_server->Broadcast(packet); 
            DropEventHistory(current_tick-1);
        }
        m_server->Update();

        Scenes scene = m_scene_manager.GetScene()->CheckSceneChange(m_game_state);
        if (scene != Scenes::None) {
            ENetPacket* packet = CreatePacket<Scenes>(NetMsg::SCENE_CHANGE, scene, ENET_PACKET_FLAG_RELIABLE);
            m_server->Broadcast(packet); 
            m_scene_manager.ChangeScene(scene);
            InitGame();
        }

        m_tick++;
    }

    void OnConnect(ENetEvent event) {
        uint32_t id = enet_peer_get_id(event.peer);
        m_server->SendTo(id, CreatePacket<uint32_t>(NetMsg::GAME_TICK, m_tick));
        m_server->SendTo(id, CreatePacket<uint32_t>(NetMsg::PLAYER_ID, id));
        AddAndSyncChatMessage(server_chat_name, TextFormat("Player joined"));

        {
        ENetPacket* packet = CreatePacket<uint32_t>(NetMsg::PLAYER_JOIN, id, ENET_PACKET_FLAG_RELIABLE);
        m_server->Broadcast(packet);
        }

        {
        Scenes scene_id = m_scene_manager.GetSceneId();
        ENetPacket* packet = CreatePacket<Scenes>(NetMsg::SCENE_INITIAL, scene_id, ENET_PACKET_FLAG_RELIABLE);
        m_server->SendTo(id, packet);
        }
        AddPlayer(m_game_state, id);
        m_game_metadata.SetPlayerName(id, TextFormat("Player_%d", m_game_metadata.GetPlayers().size()));
        UpdateMetadata();
        BroadcastMetadata();
    }

    void OnDisconnect(ENetEvent event) {
        uint32_t id = enet_peer_get_id(event.peer);
        AddAndSyncChatMessage(server_chat_name, TextFormat("%s left", m_game_metadata.GetPlayerName(id)));

        {
        ENetPacket* packet = CreatePacket<uint32_t>(NetMsg::PLAYER_LEAVE, id, ENET_PACKET_FLAG_RELIABLE);
        m_server->Broadcast(packet);
        }
        RemovePlayer(m_game_state, id);
        UpdateMetadata();
        BroadcastMetadata();
    }

    void OnReceive(ENetEvent event)
    {
        MessageType msgType = ExtractMessageType(event.packet);
        switch (msgType) {
        case NetMsg::PLAYER_INPUT:
            {
                PlayerInputPacketData received = ExtractData<PlayerInputPacketData>(event.packet);
                uint32_t id = enet_peer_get_id(event.peer);

                GameEvent game_event;
                game_event.event_id = EV_PLAYER_INPUT;
                game_event.data = received.input;

                AddEvent(game_event, id, received.tick);
            }
            break;

        case NetMsg::CHAT_MESSAGE:
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
        case NetMsg::NAME_CHANGE:
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

    void UpdateMetadata() {
        // only deletion, the addition is handled on connection
        std::vector<uint32_t> player_ids_for_deletion{};
        for (auto& [id, player_metadata] : m_game_metadata.GetPlayers()) {
            if (m_game_state.players.find(id) == m_game_state.players.end()) {
                player_ids_for_deletion.push_back(id);
            }
        }
        for (auto& id : player_ids_for_deletion) {
            m_game_metadata.RemovePlayer(id);
        }
    }

    void BroadcastMetadata() {
        SerializedGameMetadata serialized = m_game_metadata.Serialize();
        ENetPacket* packet = CreatePacket(NetMsg::GAME_METADATA, serialized);
        m_server->Broadcast(packet);
    }

    void AddAndSyncChatMessage(const char* name, const char* text) {
        ChatMessage message;
        std::snprintf(message.name, max_player_name_len, "%s", name);
        std::snprintf(message.text, max_string_len, "%s", text);

        m_chat.AddMessage(message);

        ENetPacket* packet = CreatePacket(NetMsg::CHAT_MESSAGE, message);
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