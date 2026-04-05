#include "Forest.hpp"
#include "World.hpp"
#include "Game.hpp"
#include <random>

#if WITH_RENDER
#include "Resources.hpp"
#include "Rendering.hpp"
#endif

enum Models : ModelKey {
    None = R_MODEL_NONE,
    Player,
    CubeExclamation,
    Heightmap,
    Football,
    Tree,
    Grass,
    Test,
    Count
};

constexpr Vector3 heightmap_scale = Vector3{1000, 100, 1000};
constexpr int trees_count = 100;
constexpr int grass_count = 500;

void Forest::PostSetup() {
    float x = -150.0f;
    float z = 150.0f;
    m_door_position = Vector3{x, m_heightmap.GetHeightAt(x, z) + 15, z};
}

void Forest::SetupHeightmap() {
    Image image = LoadHeightmapImage();
    m_heightmap.Load(
        image,
        {0, 0, 0},
        m_heightmap_scale
    );
    UnloadImage(image);
}

Forest::Forest() : SceneRegular(0, heightmap_scale, trees_count, grass_count, 20.f, 5.0f) {
}

#if WITH_RENDER

void Forest::LoadResources() {
    std::cout << "Loading scene resources..." << std::endl;

    Resources& r = Resources::Get();
    //ModelAliased& p = SetModelNonAnimated(R_MODEL_DEFAULT, "assets/model.glb");
    ModelAliased& player = r.SetModelAnimated(Models::Player, "assets/Solus_the_knight.gltf", 10);
    player.SetScale(1.5f);

    ModelAliased& cube = r.SetModelNonAnimated(Models::CubeExclamation, "assets/box_crate.glb");
    
    ModelAliased& football = r.SetModelNonAnimated(Models::Football, "assets/football_ball.glb");
    football.SetScale(9.1f);

    if (trees_count > 0) {
        ModelAliased& tree = r.SetInstancedModel(Models::Tree, "assets/oak.glb", trees_count);
    }
    if (grass_count > 0) {
        ModelAliased& grass = r.SetInstancedModel(Models::Grass, "assets/grass.glb", grass_count);
    }

    Image img = LoadHeightmapImage();
    ModelAliased& heightmap = r.SetHeightmapModel(Models::Heightmap, img, heightmap_scale);
    UnloadImage(img);

    heightmap.SetMaterial(
        CreateMaterial(
            LoadTexture("assets/ground0/Ground085_1K-PNG_Color.png"),
            LoadTexture("assets/ground0/Ground085_1K-PNG_NormalGL.png"),
            1.5f,
            10.0f
        )
    );

    /*
    {
    R3D_Mesh mesh = R3D_GenMeshCube(2.0f, 2.0f, 2.0f);
    m_models[R_MODEL_TEST].FromMeshNonAnimated(mesh);
    R3D_Material mat = R3D_GetDefaultMaterial();
    Color clr = RED;
    
    mat.albedo.color = clr;
    mat.emission.color = clr;
    mat.emission.energy = 0;
    mat.orm.metalness = 0;
    mat.orm.roughness = 1.0;
    mat.orm.occlusion = 0;
    m_models[R_MODEL_TEST].SetMaterial(mat);
    }
    */
    std::cout << "Successfully loaded scene resources" << std::endl;
}

void Forest::UnloadResources() {
    std::cout << "Unloading scene resources..." << std::endl;
    Resources& r = Resources::Get();
    for (ModelKey key = Models::None + 1; key < Models::Count; key++) {
        r.TryUnloadModel(key);
    }
    std::cout << "Successfully unloaded scene resources" << std::endl;
}

void Forest::Draw(const GameDrawingData &drawing_data) const {
    Rendering::Get().RenderModel(Models::Heightmap, m_heightmap.GetBottomCenter());
    if (trees_count > 0) Rendering::Get().RenderInstancedModel(Models::Tree);
    if (grass_count > 0) Rendering::Get().RenderInstancedModel(Models::Grass);

    for (auto& [key, static_actor] : m_static_actors) {
        static_actor.Draw(drawing_data);
    }

    // draw door
    for (float i = 10; i < 13; i++) {
        Rendering::Get().RenderPrimitiveCube(m_door_position, {i, i, i});
    }


    if (WindowGlobal::Get().IsDebugRenderEnabled()) {
        int NUM_CELLS = m_partitioner.GetGrid().NUM_CELLS;
        int CELL_SIZE = m_partitioner.GetGrid().CELL_SIZE;
        
        // Precompute the range of cell indices
        const float halfCell = CELL_SIZE * 0.5f;

        // Loop over all cell indices in X and Z
        for (int ix = 0; ix < NUM_CELLS; ++ix) {
            for (int iz = 0; iz < NUM_CELLS; ++iz) {
                // World position of this cell's center (X and Z only)
                float worldX = m_partitioner.GetGrid().CellIntoCoord(ix);
                float worldZ = m_partitioner.GetGrid().CellIntoCoord(iz);

                float thickness = 10.0f;
                // Draw the cell at every requested height
                for (float h = 0; h < 100; h+=thickness) {
                    Rendering::Get().RenderPrimitiveCube({ worldX+halfCell, h, worldZ+halfCell }, { halfCell, thickness, halfCell });
                }
            }
        }
    }    
}
#endif

void Forest::Load() {
    #if WITH_RENDER

    m_cubemap = R3D_LoadCubemap("assets/night.jpg", R3D_CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_ENVIRONMENT_SET(background.skyBlur, 0.0f);
    R3D_ENVIRONMENT_SET(background.energy, 0.7f);
    R3D_ENVIRONMENT_SET(background.sky, m_cubemap);

    // Setup environment ambient
    m_ambient_map = R3D_GenAmbientMap(m_cubemap, R3D_AMBIENT_ILLUMINATION);
    R3D_ENVIRONMENT_SET(ambient.map, m_ambient_map);
    R3D_ENVIRONMENT_SET(ambient.energy, 0.4f);

    // FOG
    R3D_ENVIRONMENT_SET(fog.mode, R3D_FOG_EXP2);
    R3D_ENVIRONMENT_SET(fog.color, ColorLerp(PURPLE, BLACK, 0.9));
    R3D_ENVIRONMENT_SET(fog.start, 3.0f);
    R3D_ENVIRONMENT_SET(fog.end, 50.0f);
    R3D_ENVIRONMENT_SET(fog.density, 0.0028f);
    R3D_ENVIRONMENT_SET(fog.skyAffect, 0.5f);

    LoadResources();
    #endif
}

void Forest::Unload() {
    #if WITH_RENDER
    UnloadResources();
    for (auto& [key, light] : m_lights) {
        R3D_DestroyLight(light);
    }
    m_lights.clear();
    
    R3D_UnloadAmbientMap(m_ambient_map);
    R3D_UnloadCubemap(m_cubemap);
    #endif
};

GameState Forest::PopulateState(const GameState &old_state) {
    GameState state;

    for (const auto& [id, player_data] : old_state.players) {
        InitNewPlayer(state, id);
    }

    // {
    // BoxData box_data;
    // float a = 10;
    // box_data.SetHalfExtends(Vector3{a, a, a});

    // BodyData body_data;
    // body_data.position = Vector3{0, 20, 40};
    // body_data.shapes.push_back(CollisionShape(box_data));

    // ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
    // state.world_data.GetActor(actor_key).render_data.model_key = Models::CubeExclamation;
    // }

    for (int i = 0; i < 1; i++) {
        SphereData sphere_data;
        sphere_data.SetRadius(10.0f);

        BodyData body_data;
        body_data.restitution = 2;
        body_data.position = Vector3{40.f*(i%10), 30.f*i, 40.f*(i/10)};
        body_data.shapes.push_back(CollisionShape(sphere_data));

        ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
        state.world_data.GetActor(actor_key).render_data.model_key = Models::Football;
    }
    
    return state;
}

Scenes Forest::CheckSceneChange(const GameState &state) {
    if (state.players.size() == 0) return Scenes::None;

    bool players_ready = true;    
    for (const auto& [id, player_data] : state.players) {
        Vector3 p = state.GetActor(id).body.position;
        if (Vector3Distance(p, m_door_position) > 30) {
            players_ready = false;
            break;
        }
    }
    if (players_ready) return Scenes::Green;
    return Scenes::None;
}

void Forest::InitNewPlayer(GameState &state, uint32_t id) {
    int player_count = state.players.size();

    BodyData body_data;
    float r = 13.0f / 2;
    {
    CollisionShape sphere(SphereData(r, Vector3{0.0f, -r, 0.0f}));
    body_data.shapes.push_back(sphere);
    }
    {
    CollisionShape sphere(SphereData(r, Vector3{0.0f, 0.0f, 0.0f}));
    body_data.shapes.push_back(sphere);
    }
    {
    CollisionShape sphere(SphereData(r, Vector3{0.0f, r, 0.0f}));
    body_data.shapes.push_back(sphere);
    }

    ActorData actor_data(body_data, Models::Player);
    
    actor_data.render_data.offset = {0, -12, 0};

    actor_data.body.position = Vector3{10.0f*player_count, 10, 0};

    PlayerData player_data;
    player_data.actor_key = state.world_data.AddActor(actor_data);
    

    state.players[id] = player_data;
}

void Forest::UpdateActor(GameState &state, ActorKey actor_key, uint32_t tick, void *user_data) {
    UpdateUserData* update_data = reinterpret_cast<UpdateUserData*>(user_data);

    #if WITH_RENDER

    
    //auto [is_player, id] = state.IsPlayer(actor_key);
    if (state.world_data.ActorExists(actor_key)) {
        ActorData actor_data = state.world_data.GetActor(actor_key);
        if (actor_data.render_data.model_key == Models::Player) {
            if (m_lights.find(actor_key) == m_lights.end()) {
                m_lights[actor_key] = CreateLight();
            }
            R3D_SetLightPosition(m_lights.at(actor_key), actor_data.body.position+actor_data.VForward()*10);
            R3D_SetLightDirection(m_lights.at(actor_key), actor_data.VForward());
        }
    }

    #endif
}
