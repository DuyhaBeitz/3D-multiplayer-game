#include "Game.hpp"

#include "GameDrawingData.hpp"

void GameState::Draw(GameDrawingData &drawing_data) const {
    world_data.Draw(drawing_data);

    for (const auto& [id, player_data] : players) {
        if (drawing_data.actors_except.find(player_data.actor_key) == drawing_data.actors_except.end()) {

            if (world_data.ActorExists(player_data.actor_key)) {
                const ActorData actor_data = world_data.GetActor(player_data.actor_key);

                constexpr float font_size = 16;
                const Vector3 draw_pos = {
                    actor_data.body.position.x,
                    actor_data.body.Max().y+font_size,
                    actor_data.body.position.z
                };
                const char* name = drawing_data.game_metadata.GetPlayerName(id);

                if (name) Rendering::Get().RenderText(name, draw_pos, -actor_data.yaw*180/PI+90, font_size);
            }
        }
    }
}

void Game::InitNewPlayer(GameState &state, uint32_t id) {
    BodyData body_data;
    CollisionShape sphere(SphereData{13});
    body_data.shapes.push_back(sphere);
    ActorData actor_data(body_data, R_MODEL_PLAYER);
    
    actor_data.render_data.offset = {0, -12, 0};

    actor_data.body.position = Vector3{0, 10, 0};

    PlayerData player_data;
    player_data.actor_key = state.world_data.AddActor(actor_data);
    
    m_game_metadata.SetPlayerName(id, TextFormat("player_%d", state.players.size()));

    state.players[id] = player_data;
}

void Game::ApplyEvent(GameState &state, const GameEvent &event, uint32_t id) {
    switch (event.event_id) {
    case EV_PLAYER_JOIN:
        InitNewPlayer(state, id);
        break;

    case EV_PLAYER_LEAVE:
        state.world_data.RemoveActor(state.players.at(id).actor_key);
        break;

    case EV_PLAYER_INPUT:
        if (std::holds_alternative<PlayerInput>(event.data)) {
            const PlayerInput& input = std::get<PlayerInput>(event.data);
            if (state.world_data.ActorExists(state.players[id].actor_key)) {
                state.ApplyInput(input, id);
            }
        }
        break;
    
    default:
        break;
    }
}

void Game::Draw(const GameState &state, GameDrawingData &drawing_data) {
    state.Draw(drawing_data);
    drawing_data.game_metadata.Draw();
}

GameState Game::Lerp(const GameState &state1, const GameState &state2, float alpha, const void *data) {
    alpha = fmin(1, fmax(0, alpha));
    GameState lerped = state2;

    const std::set<ActorKey>* except_keys = static_cast<const std::set<ActorKey>*>(data);

    for (auto& [actor_key, actor] : state2.world_data.actors) {
        if (state1.world_data.ActorExists(actor_key)) {
            ActorData& actor_data = lerped.world_data.GetActor(actor_key);

            if (except_keys->find(actor_key) == except_keys->end()) {
                actor_data.body.position = Vector3Lerp(
                    state1.world_data.GetActor(actor_key).body.position,
                    state2.world_data.GetActor(actor_key).body.position,
                    alpha
                );
            }
            else {
                actor_data = state1.world_data.GetActor(actor_key);
            }
            actor_data.body.UpdateShapePositions();
        }
    }
    return lerped;
}
SerializedGameState Game::Serialize(const GameState &state) {
    SerializedGameState sgs{};
    
    // Serialize into a temporary stream
    std::ostringstream os(std::ios::binary);
    {
        cereal::BinaryOutputArchive archive(os);
        archive(state);
    }

    // Copy into the fixed-size buffer
    std::string str = os.str();
    sgs.size = static_cast<uint32_t>(str.size());
    if (sgs.size > sizeof(sgs.bytes)) {
        throw std::runtime_error("Serialized state exceeds buffer size");
    }
    std::memcpy(sgs.bytes, str.data(), sgs.size);

    return sgs;
}

GameState Game::Deserialize(SerializedGameState data) {
    // Wrap the raw buffer in a stringstream for Cereal
    std::istringstream is(std::string(reinterpret_cast<const char*>(data.bytes), data.size),
                        std::ios::binary);

    cereal::BinaryInputArchive archive(is);
    GameState gs;
    archive(gs);
    return gs;
}

void Game::InitGame(GameState &state) {
    m_game_metadata.Load();

    // { // floor
    // BoxData box_data;
    // box_data.half_extents = Vector3{1000, 100, 1000};

    // BodyData body_data;
    // body_data.position = Vector3{0, -100, 0};
    // body_data.velocity = Vector3{0, 0, 0};
    // body_data.inverse_mass = 0;
    // body_data.shapes.push_back(CollisionShape(box_data));

    // state.world_data.AddActor(ActorData(body_data));
    // }

    // {
    // BoxData box_data;
    // box_data.half_extents = Vector3{30, 5, 30};

    // BodyData body_data;
    // body_data.position = Vector3{40, 20, 0};
    // body_data.shapes.push_back(CollisionShape(box_data));

    // state.world_data.AddActor(ActorData(body_data));
    // }
    
    {
    BoxData box_data;
    float a = 10;
    box_data.half_extents = Vector3{a, a, a};

    BodyData body_data;
    body_data.position = Vector3{0, 20, 40};
    body_data.shapes.push_back(CollisionShape(box_data));

    ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
    state.world_data.GetActor(actor_key).render_data.model_key = R_MODEL_CUBE_EXCLAMATION;
    }

    for (int i = 0; i < 1; i++) {
        SphereData sphere_data;
        sphere_data.radius = 10;

        BodyData body_data;
        body_data.restitution = 2;
        body_data.position = Vector3{40, 30.f*i, 40.f};
        body_data.shapes.push_back(CollisionShape(sphere_data));

        ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
        state.world_data.GetActor(actor_key).render_data.model_key = R_MODEL_FOOTBALL;
    }
}

Camera GetCameraFromPos(Vector3 pos, Vector3 target) {
    Camera3D camera = { 0 };
    Vector3 cam_offset = {0, 5, 0};
    camera.position = pos;
    camera.target = target;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 90.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;    
}

Camera GetCameraFromActor(const ActorData &actor_data) {
    Vector3 cam_offset = {0, 10, 0};
    Vector3 position = actor_data.body.position + cam_offset;
    Vector3 target =  position + actor_data.VForward();

    // Vector3 target =  actor_data.body.position + cam_offset;
    // Vector3 position = target - actor_data.VForward()*130;
    
    return GetCameraFromPos(position, target);
}

void ToggleWindow() {
    bool cursor_was_hidden = IsCursorHidden();

    EnableCursor();
    ToggleBorderlessWindowed();
    BeginDrawing();
    EndDrawing();
    if (cursor_was_hidden) DisableCursor();
    BeginDrawing();
    EndDrawing();  
}

void ToggleCursor() {
    if (IsCursorHidden()){
        EnableCursor();
    }
    else {
        DisableCursor();
    }
}