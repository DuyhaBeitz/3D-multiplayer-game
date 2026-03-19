#include "StaticWorld.hpp"
#include "GameMetadata.hpp"
#include "Actor.hpp"
#include <random>

StaticWorld::StaticWorld() : m_partitioner(&m_static_actors) {
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

void StaticWorld::SetupWorld(const GameMetadata &game_metadata) {
    int grid_cells = m_heightmap.GetSamplesPerSide();
    Vector3 corner = m_heightmap.GetPosition();
    Vector3 scale = m_heightmap.GetScale();

    std::mt19937 engine(game_metadata.GetSeed()); 
    //setup trees
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    #ifdef WITH_RENDER
    auto data = Resources::Get().ModelFromKey(R_MODEL_TREE).GetInstancesData();
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
    #ifdef WITH_RENDER
    data->SetPositions(positions);
    data->SetScales(scales);
    
    {//setup grass
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    #ifdef WITH_RENDER
    auto data = Resources::Get().ModelFromKey(R_MODEL_GRASS).GetInstancesData();
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

void StaticWorld::Load(const GameMetadata &game_metadata) {
    std::cout << "Loading static world" << std::endl;
    Image image = LoadImage(P_HIEGHTMAP0_IMAGE_PATH);
    m_heightmap.Load(
        image,
        {0, 0, 0},
        heightmap0_scale
    );
    m_heightmap_model_key = R_MODEL_HEIGHTMAP0;
    UnloadImage(image);

    SetupWorld(game_metadata);
}

#if WITH_RENDER
void StaticWorld::Draw(const GameDrawingData &drawing_data) const {
    Rendering::Get().RenderModel(m_heightmap_model_key, m_heightmap.GetBottomCenter());
    Rendering::Get().RenderInstancedModel(R_MODEL_TREE);
    Rendering::Get().RenderInstancedModel(R_MODEL_GRASS);

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
}
#endif

void StaticWorld::SolveCollisionWith(BodyData &other) const {
    m_heightmap.SolveCollisionWith(other);

    PartitionUnit unit(nullptr, other.position.x, other.position.z);
    unit.user_data = reinterpret_cast<void*>(&other);
    m_partitioner.GetGrid().unit_with_grid(&unit, other.position.x, other.position.z);
}

// ActorData& StaticWorld::AddStaticActor(ActorKey actor_key, ActorData static_actor) {
//     m_static_actors[actor_key] = static_actor;

//     PartitionUnit unit(&m_grid, static_actor.body.position.x, static_actor.body.position.z);
//     unit.user_data = reinterpret_cast<void*>(&m_static_actors[actor_key].body);
//     m_units.push_back(unit);
//     m_grid.add(&m_units.back());

//     return m_static_actors[actor_key];
// }
