#include "GameMetadata.hpp"
#include <random>

#if WITH_RENDER
void GameMetadata::LoadVisuals() {
    int grid_cells = m_heightmap.GetSamplesPerSide();
    Vector3 corner = m_heightmap.GetPosition();
    Vector3 scale = m_heightmap.GetScale();

    std::mt19937 engine(m_seed); 
    {//setup trees
    std::uniform_int_distribution<int> dist_xz(0, 1000);
    std::uniform_int_distribution<int> dist_scale(50, 150);

    auto data = Resources::Get().ModelFromKey(R_MODEL_TREE).GetInstancesData();
    std::vector<Vector3> positions{};
    std::vector<Vector3> scales{};
    for (int i = 0; i < data->GetCount(); i++) {
        float x = dist_xz(engine) / 1000.0f * scale.x + corner.x;
        float z = dist_xz(engine) / 1000.0f * scale.z + corner.z;

        positions.push_back(
            Vector3{
                x,
                m_heightmap.GetHeightAt(x, z) - 5,
                z
            }
        );
        float s = dist_scale(engine) / 100.0f;
        s *= 10.0f;
        scales.push_back(Vector3{s, s, s});
    }

    data->SetPositions(positions);
    data->SetScales(scales);
    }

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
}
#endif