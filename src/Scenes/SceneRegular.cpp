#include "SceneRegular.hpp"
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

SceneRegular::SceneRegular(std::string heightmap_image_path, uint32_t seed, Vector3 heightmap_scale, int trees_count, int grass_count)
 : m_partitioner(&m_static_actors), m_heightmap_image_path(heightmap_image_path), m_seed(seed), m_heightmap_scale(heightmap_scale), m_trees_count(trees_count), m_grass_count(grass_count)
  {
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
void SceneRegular::Setup() {
    std::cout << "Setting up scene" << std::endl;

    Image image = LoadImage(m_heightmap_image_path.c_str());
    m_heightmap.Load(
        image,
        {0, 0, 0},
        m_heightmap_scale
    );
    UnloadImage(image);


    int grid_cells = m_heightmap.GetSamplesPerSide();
    Vector3 corner = m_heightmap.GetPosition();
    Vector3 scale = m_heightmap.GetScale();

    std::mt19937 engine(m_seed);
    //setup trees
    if (m_trees_count > 0) {
        std::uniform_int_distribution<int> dist_xz(0, 1000);
        std::uniform_int_distribution<int> dist_scale(50, 150);

        #if WITH_RENDER
        auto& model = Resources::Get().ModelFromKey(Models::Tree);
        auto data = model.GetInstancesData();
        #endif
        std::vector<Vector3> positions{};
        std::vector<Vector3> scales{};
        for (int i = 0; i < m_trees_count; i++) {
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
        #endif
    }
    
    if (m_grass_count > 0) {//setup grass
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    #if WITH_RENDER
    auto& model = Resources::Get().ModelFromKey(Models::Grass);
    auto data = model.GetInstancesData();
    #endif
    std::vector<Vector3> positions{};
    std::vector<Vector3> scales{};
    for (int i = 0; i < m_grass_count; i++) {
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

    #if WITH_RENDER
    data->SetPositions(positions);
    data->SetScales(scales);
    #endif
    }    

    m_partitioner.UpdateView();

    for (auto& [key, static_actor] : m_static_actors) {
        static_actor.Update(0);
    }

    PostSetup();

    std::cout << "Successfully set up scene" << std::endl;
}

void SceneRegular::SolveCollisionWith(BodyData &other) const {
    m_heightmap.SolveCollisionWith(other);

    PartitionUnit unit(nullptr, other.position.x, other.position.z);
    unit.user_data = reinterpret_cast<void*>(&other);
    m_partitioner.GetGrid().unit_with_grid(&unit, other.position.x, other.position.z);
}
