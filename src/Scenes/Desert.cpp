#include "Desert.hpp"
#include "World.hpp"
#include "Game.hpp"
#include <random>

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
constexpr int trees_count = 250;
constexpr int grass_count = 500;

#define P_HIEGHTMAP_IMAGE_PATH "assets/Heightmap_01_Mountain.png"

Desert::Desert() : m_partitioner(&m_static_actors) {
    m_partitioner.GetGrid().SetHandlePairFunc(
        [](PartitionUnit* un1, PartitionUnit* un2){
            if (un1 && un2) {
                BodyData* dynamic_body = reinterpret_cast<BodyData*>(un1->user_data);
                BodyData* static_body = reinterpret_cast<BodyData*>(un2->user_data);
                SolveCollisionOneWay(*static_body, *dynamic_body, static_body->CollideWith(*dynamic_body));
            }
        }
    );
}

#if WITH_RENDER
#include "Resources.hpp"
#include "Rendering.hpp"

void Desert::LoadResources() {
    std::cout << "Loading scene resources..." << std::endl;
    Resources& r = Resources::Get();
    //ModelAliased& p = SetModelNonAnimated(R_MODEL_DEFAULT, "assets/model.glb");
    ModelAliased& player = r.SetModelAnimated(Models::Player, "assets/Solus_the_knight.gltf", 10);
    player.SetScale(1.5f);

    ModelAliased& cube = r.SetModelNonAnimated(Models::CubeExclamation, "assets/box_crate.glb");
    
    ModelAliased& football = r.SetModelNonAnimated(Models::Football, "assets/football_ball.glb");
    football.SetScale(9.1f);

    ModelAliased& tree = r.SetInstancedModel(Models::Tree, "assets/palm_tree_realistic.glb", trees_count);
    ModelAliased& grass = r.SetInstancedModel(Models::Grass, "assets/grass.glb", grass_count);

    ModelAliased& heightmap = r.SetHeightmapModel(Models::Heightmap, P_HIEGHTMAP_IMAGE_PATH, heightmap_scale);
    heightmap.SetMaterial(
        CreateMaterial(
            LoadTexture("assets/sand/GroundSand005_COL_1K.jpg"),
            LoadTexture("assets/sand/GroundSand005_NRM_1K.jpg"),
            1.3f,
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

void Desert::UnloadResources() {
    std::cout << "Unloading scene resources..." << std::endl;
    Resources& r = Resources::Get();
    for (ModelKey key = Models::None + 1; key < Models::Count; key++) {
        r.TryUnloadModel(key);
    }
    std::cout << "Successfully unloaded scene resources" << std::endl;
}

void Desert::Draw(const GameDrawingData &drawing_data) const {
    Rendering::Get().RenderModel(Models::Heightmap, m_heightmap.GetBottomCenter());
    Rendering::Get().RenderInstancedModel(Models::Tree);
    Rendering::Get().RenderInstancedModel(Models::Grass);

    for (auto& [key, static_actor] : m_static_actors) {
        static_actor.Draw(drawing_data);
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
};
#endif

void Desert::SetupStatic() {
    Image image = LoadImage(P_HIEGHTMAP_IMAGE_PATH);
    m_heightmap.Load(
        image,
        {0, 0, 0},
        heightmap_scale
    );
    UnloadImage(image);


    int grid_cells = m_heightmap.GetSamplesPerSide();
    Vector3 corner = m_heightmap.GetPosition();
    Vector3 scale = m_heightmap.GetScale();

    std::mt19937 engine(0);
    //setup trees
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    #if WITH_RENDER
    auto& model = Resources::Get().ModelFromKey(Models::Tree);
    auto data = model.GetInstancesData();
    #endif
    std::vector<Vector3> positions{};
    std::vector<Vector3> scales{};
    for (int i = 0; i < trees_count; i++) {
        float x = dist_xz(engine) / 1000.0f * scale.x + corner.x;
        float z = dist_xz(engine) / 1000.0f * scale.z + corner.z;

        float s = dist_scale(engine) / 100.0f;
        s *= 10.0f;
        scales.push_back(Vector3{s, s, s});

        positions.push_back(
            Vector3{
                x,
                m_heightmap.GetHeightAt(x, z) - s/2,
                z
            }
        );

        // create collisions for trees
        {
        BoxData box_data;
        Vector3 half_size = Vector3{0.1, 5, 0.1} * s;        
        box_data.SetHalfExtends(half_size);

        BodyData body_data;
        body_data.position = positions[i];
        body_data.shapes.push_back(CollisionShape(box_data));

        m_static_actors[i] = ActorData(body_data);
        m_static_actors[i].render_data.model_key = R_MODEL_NONE;
        }
    }
    #if WITH_RENDER
    data->SetPositions(positions);
    data->SetScales(scales);
    
    {//setup grass
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    #if WITH_RENDER
    auto& model = Resources::Get().ModelFromKey(Models::Grass);
    auto data = model.GetInstancesData();
    #endif
    std::vector<Vector3> positions{};
    std::vector<Vector3> scales{};
    for (int i = 0; i < grass_count; i++) {
        float x = dist_xz(engine) / 1000.0f * scale.x + corner.x;
        float z = dist_xz(engine) / 1000.0f * scale.z + corner.z;

        positions.push_back(
            Vector3{
                x,
                m_heightmap.GetHeightAt(x, z) - 3,
                z
            }
        );
        float s = dist_scale(engine) / 100.0f;
        s *= 5.0f;
        scales.push_back(Vector3{s, s, s});
    }

    data->SetPositions(positions);
    data->SetScales(scales);
    }
    #endif

    m_partitioner.UpdateView();

    for (auto& [key, static_actor] : m_static_actors) {
        static_actor.Update(0);
    }
}

void Desert::Setup() {
    std::cout << "Setting up scene" << std::endl;

    #if WITH_RENDER
    LoadResources();
    #endif

    SetupStatic();
    std::cout << "Successfully set up scene" << std::endl;
}

GameState Desert::PopulateState(const GameState &old_state) {
    GameState state;
    {
    BoxData box_data;
    float a = 10;
    box_data.SetHalfExtends(Vector3{a, a, a});

    BodyData body_data;
    body_data.position = Vector3{0, 20, 40};
    body_data.shapes.push_back(CollisionShape(box_data));

    ActorKey actor_key = state.world_data.AddActor(ActorData(body_data));
    state.world_data.GetActor(actor_key).render_data.model_key = Models::CubeExclamation;
    }

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

    // TODO: Iterate over players, so that when loading a new scene player actors are added right away

    return state;
}

void Desert::InitNewPlayer(GameState &state, uint32_t id) {
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

void Desert::SolveCollisionWith(BodyData &other) const {
    m_heightmap.SolveCollisionWith(other);

    PartitionUnit unit(nullptr, other.position.x, other.position.z);
    unit.user_data = reinterpret_cast<void*>(&other);
    m_partitioner.GetGrid().unit_with_grid(&unit, other.position.x, other.position.z);
}
