#include "StaticWorld.hpp"
#include "GameMetadata.hpp"
#include "Actor.hpp"
#include <random>

#if WITH_RENDER
void StaticWorld::LoadVisuals(const GameMetadata &game_metadata) {
    int grid_cells = m_heightmap.GetSamplesPerSide();
    Vector3 corner = m_heightmap.GetPosition();
    Vector3 scale = m_heightmap.GetScale();

    std::mt19937 engine(game_metadata.GetSeed()); 
    {//setup trees
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    auto data = Resources::Get().ModelFromKey(R_MODEL_TREE).GetInstancesData();
    std::vector<Vector3> positions{};
    std::vector<Vector3> scales{};
    for (int i = 0; i < data->GetCount(); i++) {
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
        Vector3 half_size = Vector3{0.1, 4.0, 0.1} * s;        
        box_data.SetHalfExtends(half_size);

        BodyData body_data;
        body_data.position = positions[i] + Vector3{0, s, 0};
        body_data.shapes.push_back(CollisionShape(box_data));

        m_static_actors[i] = ActorData(body_data);
        m_static_actors[i].render_data.model_key = R_MODEL_NONE;
        }
    }

    data->SetPositions(positions);
    data->SetScales(scales);
    
    {//setup grass
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    auto data = Resources::Get().ModelFromKey(R_MODEL_GRASS).GetInstancesData();
    std::vector<Vector3> positions{};
    std::vector<Vector3> scales{};
    for (int i = 0; i < data->GetCount(); i++) {
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

    for (auto& [key, static_actor] : m_static_actors) {
        static_actor.Update(0);
    }
    }
}
#endif

void StaticWorld::Load(const GameMetadata& game_metadata) {
    std::cout << "Loading static world" << std::endl;
    Image image = LoadImage(P_HIEGHTMAP0_IMAGE_PATH);
    m_heightmap.Load(
        image,
        {0, 0, 0},
        heightmap0_scale
    );
    m_heightmap_model_key = R_MODEL_HEIGHTMAP0;
    UnloadImage(image);

#if WITH_RENDER
    LoadVisuals(game_metadata);
#endif
}


#if WITH_RENDER
void StaticWorld::Draw(const GameDrawingData &drawing_data) const  {
    Rendering::Get().RenderModel(m_heightmap_model_key, m_heightmap.GetBottomCenter());
    Rendering::Get().RenderInstancedModel(R_MODEL_TREE);
    Rendering::Get().RenderInstancedModel(R_MODEL_GRASS);

    for (auto& [key, static_actor] : m_static_actors) {
        static_actor.Draw(drawing_data);
    }
}
#endif

void StaticWorld::SolveCollisionWith(BodyData &other) const
{
    m_heightmap.SolveCollisionWith(other);

    for (auto& [key, static_actor] : m_static_actors) {
        SolveCollisionOneWay(static_actor.body, other, static_actor.body.CollideWith(other));
    }
}