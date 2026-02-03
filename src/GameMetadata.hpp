#pragma once

#include "Serialization.hpp"
#include "Constants.hpp"
#include "Physics.hpp"

/*
Information that doesn't get synced every frame
Like player's name etc.
Isn't supposed to be used in game logic, which is replayed a lot because of reconciliation
It's for drawing, chat etc.
*/

struct PlayerMetadata {
    char name[max_player_name_len] = {};

    template <class Archive>
    void serialize(Archive& ar) {
        ar(cereal::binary_data(name, max_player_name_len));
    }
};

struct SerializedGameMetadata {
    uint32_t size = 0;          // number of valid bytes
    uint8_t bytes[4096*2];    // max packet size
};

class GameMetadata {
private:
    std::map<uint32_t, PlayerMetadata> m_players{};
    HeightmapData m_heightmap{};
    ModelKey m_heightmap_model_key{};

public:
    GameMetadata() = default;

#if WITH_RENDER
    void Draw() const {
        //m_heightmap.Draw();
        Rendering::Get().RenderModel(m_heightmap_model_key, m_heightmap.GetBottomCenter());

        Rendering::Get().RenderInstancedModel(R_MODEL_TREE);
        Rendering::Get().RenderInstancedModel(R_MODEL_GRASS);
    }
#endif

    const char* GetPlayerName(uint32_t id) {
        return m_players[id].name;
    }

    void SetPlayerName(uint32_t id, const char* name) {
        std::snprintf(m_players[id].name, max_player_name_len, "%s", name);
    }

    SerializedGameMetadata Serialize() {
        SerializedGameMetadata sgs{};
        
        // Serialize into a temporary stream
        std::ostringstream os(std::ios::binary);
        {
            cereal::BinaryOutputArchive archive(os);
            archive(*this);
        }

        // Copy into the fixed-size buffer
        std::string str = os.str();
        sgs.size = static_cast<uint32_t>(str.size());
        if (sgs.size > sizeof(sgs.bytes)) {
            throw std::runtime_error("Serialized game metadata exceeds buffer size");
        }
        std::memcpy(sgs.bytes, str.data(), sgs.size);

        return sgs;
    }

    void Deserialize(SerializedGameMetadata data) {
        // Wrap the raw buffer in a stringstream for Cereal
        std::istringstream is(std::string(reinterpret_cast<const char*>(data.bytes), data.size),
                            std::ios::binary);

        cereal::BinaryInputArchive archive(is);
        archive(*this);
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(m_players);
    }

    HeightmapData& GetHeightmap() {
        return m_heightmap;
    }

    const HeightmapData& GetHeightmap() const {
        return m_heightmap;
    }

    void Load() {
        std::cout << "Loading game metadata" << std::endl;
        Image image = LoadImage(P_HIEGHTMAP0_IMAGE_PATH);
        m_heightmap.Load(
            image,
            {0, 0, 0},
            heightmap0_scale
        );
        m_heightmap_model_key = R_MODEL_HEIGHTMAP0;
        UnloadImage(image);

#if WITH_RENDER
        int grid_cells = m_heightmap.GetSamplesPerSide();
        Vector3 corner = m_heightmap.GetPosition();
        Vector3 scale = m_heightmap.GetScale();

        {//setup trees
        auto data = Resources::Get().ModelFromKey(R_MODEL_TREE).GetInstancesData();
        std::vector<Vector3> positions{};
        std::vector<Vector3> scales{};
        float density = 0.0005f;
        for (int i = 0; i < data->GetCount(); i++) {
            float x = float(int(i / density) % grid_cells) / grid_cells * scale.x + corner.x;
            float z = float(int(i / density) / grid_cells) / grid_cells * scale.z + corner.z;

            x += GetRandomValue(-100, 100) / 10.0f;
            z += GetRandomValue(-100, 100) / 10.0f;

            positions.push_back(
                Vector3{
                    x,
                    m_heightmap.GetHeightAt(x, z) - 5,
                    z
                }
            );
            float s = GetRandomValue(50, 150) / 100.0f;
            s *= 10.0f;
            scales.push_back(Vector3{s, s, s});
        }

        data->SetPositions(positions);
        data->SetScales(scales);
        }

        {//setup grass
        auto data = Resources::Get().ModelFromKey(R_MODEL_GRASS).GetInstancesData();
        std::vector<Vector3> positions{};
        std::vector<Vector3> scales{};
        float density = 0.001f;
        for (int i = 0; i < data->GetCount(); i++) {
            float x = float(int(i / density) % grid_cells) / grid_cells * scale.x + corner.x;
            float z = float(int(i / density) / grid_cells) / grid_cells * scale.z + corner.z;

            x += GetRandomValue(-1000, 1000) / 10.0f;
            z += GetRandomValue(-1000, 1000) / 10.0f;

            positions.push_back(
                Vector3{
                    x,
                    m_heightmap.GetHeightAt(x, z) - 5,
                    z
                }
            );
            float s = GetRandomValue(50, 150) / 100.0f;
            s *= 5.0f;
            scales.push_back(Vector3{s, s, s});
        }

        data->SetPositions(positions);
        data->SetScales(scales);
        }
#endif
    }
};