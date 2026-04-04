#include "Green.hpp"
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

constexpr float s = 1000;
constexpr Vector3 heightmap_scale = Vector3{1500, 200, 1500};
constexpr int trees_count = 0;
constexpr int grass_count = 0;

#define P_HIEGHTMAP_IMAGE_PATH "assets/heightmap_02.png"

void Green::PostSetup() {
    float x = -150.0f;
    float z = -150.0f;
    m_door_position = Vector3{x, m_heightmap.GetHeightAt(x, z)+15, z};
}

void Green::SetupHeightmap() {
    Image image = LoadImage(P_HIEGHTMAP_IMAGE_PATH);
    m_heightmap.Load(
        image,
        {0, 0, 0},
        m_heightmap_scale
    );
    UnloadImage(image);
}

Green::Green() : SceneRegular(0, heightmap_scale, trees_count, grass_count, 10.f, 5.0f) {
}

#if WITH_RENDER

void Green::LoadResources() {
    std::cout << "Loading scene resources..." << std::endl;

    Resources& r = Resources::Get();
    //ModelAliased& p = SetModelNonAnimated(R_MODEL_DEFAULT, "assets/model.glb");
    ModelAliased& player = r.SetModelAnimated(Models::Player, "assets/Solus_the_knight.gltf", 10);
    player.SetScale(1.5f);

    ModelAliased& cube = r.SetModelNonAnimated(Models::CubeExclamation, "assets/box_crate.glb");
    
    ModelAliased& football = r.SetModelNonAnimated(Models::Football, "assets/football_ball.glb");
    football.SetScale(9.1f);

    if (trees_count > 0) {
        ModelAliased& tree = r.SetInstancedModel(Models::Tree, "assets/palm_tree_realistic.glb", trees_count);
    }
    if (grass_count > 0) {
        ModelAliased& grass = r.SetInstancedModel(Models::Grass, "assets/grass.glb", grass_count);
    }    

    ModelAliased& heightmap = r.SetHeightmapModel(Models::Heightmap, P_HIEGHTMAP_IMAGE_PATH, heightmap_scale);
    heightmap.SetMaterial(
        CreateMaterial(
            LoadTexture("assets/grass/Grass005_1K-PNG_Color.png"),
            LoadTexture("assets/grass/Grass005_1K-PNG_NormalGL.png"),
            1.3f,
            30.0f,
            ColorLerp(WHITE, GREEN, 0.1)
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

void Green::UnloadResources() {
    std::cout << "Unloading scene resources..." << std::endl;
    Resources& r = Resources::Get();
    for (ModelKey key = Models::None + 1; key < Models::Count; key++) {
        r.TryUnloadModel(key);
    }
    std::cout << "Successfully unloaded scene resources" << std::endl;
}

void Green::Draw(const GameDrawingData &drawing_data) const {
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
                for (float h = 0; h < 1000; h+=thickness) {
                    Rendering::Get().RenderPrimitiveCube({ worldX+halfCell, h, worldZ+halfCell }, { halfCell, thickness, halfCell });
                }
            }
        }
    }    
}
#endif

void Green::Load() {
    #if WITH_RENDER
    //Create directional light with shadows
    m_light = R3D_CreateLight(R3D_LIGHT_DIR);
    R3D_SetLightDirection(m_light, (Vector3){ -1, -0.5, -1 });
    R3D_SetShadowUpdateMode(m_light, R3D_SHADOW_UPDATE_INTERVAL);
    R3D_SetLightActive(m_light, true);
    R3D_SetLightRange(m_light, 500.0f);
    R3D_SetShadowSoftness(m_light, 3.2f);
    R3D_SetShadowDepthBias(m_light, 0.001f);
    R3D_EnableShadow(m_light);

    m_cubemap = R3D_LoadCubemap("assets/sunflowers_puresky.jpg", R3D_CUBEMAP_LAYOUT_AUTO_DETECT);
    R3D_ENVIRONMENT_SET(background.skyBlur, 0.0f);

    R3D_ENVIRONMENT_SET(background.energy, 0.7f);
    R3D_ENVIRONMENT_SET(background.sky, m_cubemap);

    // Setup environment ambient
    m_ambient_map = R3D_GenAmbientMap(m_cubemap, R3D_AMBIENT_ILLUMINATION);
    R3D_ENVIRONMENT_SET(ambient.map, m_ambient_map);
    R3D_ENVIRONMENT_SET(ambient.energy, 0.4f);

    // FOG
    R3D_ENVIRONMENT_SET(fog.mode, R3D_FOG_EXP2);
    R3D_ENVIRONMENT_SET(fog.color, BLUE);
    R3D_ENVIRONMENT_SET(fog.start, 3.0f);
    R3D_ENVIRONMENT_SET(fog.end, 50.0f);
    R3D_ENVIRONMENT_SET(fog.density, 0.0004f);
    R3D_ENVIRONMENT_SET(fog.skyAffect, 0.3f);

    LoadResources();
    #endif
}

void Green::Unload() {
    #if WITH_RENDER
    UnloadResources();
    R3D_DestroyLight(m_light);
    R3D_UnloadAmbientMap(m_ambient_map);
    R3D_UnloadCubemap(m_cubemap);
    #endif
};

GameState Green::PopulateState(const GameState &old_state) {
    GameState state;
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

    // Vector3 corner = m_heightmap.GetPosition();
    // Vector3 center = m_heightmap.GetBottomCenter();
    // Vector3 diagonal = (center - corner)*2.0f;

    // int N = 20;
    // for (int i = 0; i < N*N; i++) {
    //     SphereData sphere_data;
    //     sphere_data.SetRadius(10.0f);

    //     BodyData body_data;
    //     body_data.restitution = 2;
    //     body_data.position = Vector3{corner.x+diagonal.x/float(N)*(i%N), 500+10.f*(i%10), corner.z+diagonal.z/float(N)*(i/N)};
    //     body_data.shapes.push_back(CollisionShape(sphere_data));

    //     ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
    //     state.world_data.GetActor(actor_key).render_data.model_key = Models::Football;
    // }

    for (int i = 1; i < 10; i++) {
        SphereData sphere_data;
        sphere_data.SetRadius(10.0f);

        BodyData body_data;
        body_data.restitution = 2;
        body_data.position = Vector3{40.f*(i%10), 30.f*i, 40.f*(i/10)};
        body_data.shapes.push_back(CollisionShape(sphere_data));

        ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
        state.world_data.GetActor(actor_key).render_data.model_key = Models::Football;
    }

    for (const auto& [id, player_data] : old_state.players) {
        InitNewPlayer(state, id);
    }

    return state;
}

Scenes Green::CheckSceneChange(const GameState &state) {
    if (state.players.size() == 0) return Scenes::None;

    bool players_ready = true;
    for (const auto& [id, player_data] : state.players) {
        Vector3 p = state.GetActor(id).body.position;
        if (Vector3Distance(p, m_door_position) > 30) {
            players_ready = false;
            break;
        }
    }
    if (players_ready) return Scenes::Desert;
    return Scenes::None;
}


void Green::InitNewPlayer(GameState &state, uint32_t id) {
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

    actor_data.body.position = Vector3{500+10.0f*player_count, 10, 500};

    PlayerData player_data;
    player_data.actor_key = state.world_data.AddActor(actor_data);
    

    state.players[id] = player_data;
}