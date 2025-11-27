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

    void Draw() const {
        //m_heightmap.Draw();
        Rendering::Get().RenderModel(m_heightmap_model_key, m_heightmap.GetBottomCenter());
    }

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
        Image image = LoadImage(P_HIEGHTMAP0_IMAGE_PATH);
        m_heightmap.Load(
            image,
            {0, 0, 0},
            heightmap0_scale
        );
        m_heightmap_model_key = R_MODEL_HEIGHTMAP0;
        UnloadImage(image);
    }
};