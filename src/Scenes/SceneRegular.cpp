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

struct StaticGridUserData {
    uint32_t tick;
    GameState& state;
};

ActorKey fake_key_for_heightmap = 63613;

SceneRegular::SceneRegular(uint32_t seed, Vector3 heightmap_scale, int trees_count, int grass_count, float tree_scale, float grass_scale)
 : m_partitioner(&m_static_actors), m_seed(seed), m_heightmap_scale(heightmap_scale), m_trees_count(trees_count), m_grass_count(grass_count), m_tree_scale(tree_scale), m_grass_scale(grass_scale)
  {
    m_partitioner.GetGrid().SetHandlePairFunc(
        [this](PartitionUnit* un1, PartitionUnit* un2, void* user_data){
            if (un1 && un2 && un1 != un2) {
                StaticGridUserData* data = reinterpret_cast<StaticGridUserData*>(user_data);
                ActorKey key1 = (ActorKey)reinterpret_cast<uint64_t>(un1->user_data);
                ActorKey key2 = (ActorKey)reinterpret_cast<uint64_t>(un2->user_data);
                if (user_data) {
                    BodyData* dynamic_body = &data->state.world_data.GetActor(key1).body;
                    BodyData* static_body = &m_partitioner.GetActor(key2).body;
                    if (dynamic_body && static_body) {
                        CollisionResult res = static_body->CollideWith(*dynamic_body);
                        if (res.penetration >= 0) {
                            SolveCollisionOneWay(*static_body, *dynamic_body, res);
                            #if WITH_RENDER                        
                            Audio::Get().EmitSoundEvent(
                                SoundEvent(FLAG_SOUND_PHYISCS_SD, key1, key2, data->tick,
                                    res.hit_pos, dynamic_body->velocity,
                                    R_SOUND_DEFAULT
                                )
                            );
                            #endif
                        }
                    }
                }
            }
        }
    );
}

void SceneRegular::Setup() {
    std::cout << "Setting up scene" << std::endl;

    SetupHeightmap();

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
            s *= m_tree_scale;
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
        s *= m_grass_scale;
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

void SceneRegular::UpdateActorPhysics(GameState &state, ActorKey actor_key, uint32_t tick) {
    BodyData& body = state.world_data.actors.at(actor_key).body;
    CollisionResult res = m_heightmap.CollideWith(body);
    if (res.penetration > 0) {
        m_heightmap.SolveCollisionWith(body, res);
        #if WITH_RENDER
        float speed = Vector3Length(body.velocity);
        constexpr float treshold = hor_speed/6;
        if (speed > treshold)
        {
            Audio::Get().EmitSoundEvent(
                SoundEvent(
                    FLAG_SOUND_CONTINUOUS,
                    actor_key, fake_key_for_heightmap, tick,
                    res.hit_pos, body.velocity,
                    R_SOUND_WALK,
                    0.1
                )
            );
        }
        #endif
    }
    
    PartitionUnit unit(nullptr, body.position.x, body.position.z);
    unit.user_data = reinterpret_cast<void*>(actor_key);

    StaticGridUserData user_data{tick, state};
    void* grid_user_data = reinterpret_cast<void*>(&user_data);
    m_partitioner.GetGrid().unit_with_grid(&unit, body.position.x, body.position.z, grid_user_data);
}