#pragma once

#include <raylib.h>
#include <raymath.h>
#include <cstdint>
#include <unordered_set>
#include <deque>
#include <vector>
#include <algorithm>

#include <Resources.hpp>
#include <Rendering.hpp>

constexpr int max_audio_history_tick = iters_per_sec*5;

enum SoundFlags : uint16_t {
    FLAG_SOUND_PHYISCS_DD = 0, // dynamic-dynamic collision
    FLAG_SOUND_PHYISCS_SD,
};

struct SoundEventHash {
    uint16_t flag;
    uint16_t actor_key1;
    uint16_t actor_key2;
    uint32_t tick;

    SoundEventHash(uint16_t _flag, uint16_t _actor_key1, uint16_t _actor_key2, uint32_t _tick) : 
    flag(_flag), actor_key1(std::min(_actor_key1, _actor_key2)), actor_key2(std::max(_actor_key1, _actor_key2)), tick(_tick)
    {}

    bool operator==(const SoundEventHash& other) const {
        return flag == other.flag &&
            actor_key1 == other.actor_key1 &&
            actor_key2 == other.actor_key2 &&
            tick == other.tick;
    }
};

namespace std {
    template <>
    struct hash<SoundEventHash> {
        size_t operator()(const SoundEventHash& s) const noexcept {
            size_t h1 = std::hash<uint16_t>{}(s.flag);
            size_t h2 = std::hash<uint16_t>{}(s.actor_key1);
            size_t h3 = std::hash<uint16_t>{}(s.actor_key2);
            size_t h4 = std::hash<uint32_t>{}(s.tick);

            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };
}

struct SoundEvent {
    SoundEvent(
        uint16_t _flag,
        uint16_t _actor_key1,
        uint16_t _actor_key2,
        uint32_t _tick,
        Vector3 _hit_pos,
        Vector3 _rel_vel,
        SoundKey _sound_key
    ) 
    : hash(_flag, _actor_key1, _actor_key2, _tick), hit_pos(_hit_pos), rel_vel(_rel_vel), sound_key(_sound_key) {
    }

    SoundEventHash hash;
    Vector3 hit_pos;
    Vector3 rel_vel;
    SoundKey sound_key;
};

class Audio {
private:

    std::unordered_set<SoundEventHash> m_played{};

    Audio() {};
    ~Audio() {};

    void PlaySoundAtEvent(const SoundEvent& e) {
        Resources& r = Resources::Get();
        Rendering& rr = Rendering::Get();
        r.SoundFromKey(e.sound_key).Play3D(rr.GetCamera(), e.hit_pos, 100);
    }

public:

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    static Audio& Get() {
        static Audio instance;
        return instance;
    }

    void EmitSoundEvent(SoundEvent e) {
        if (!m_played.insert(e.hash).second) return;
        PlaySoundAtEvent(e);
    }

    void Trim(uint32_t tick) {
        for (auto it = m_played.begin(); it != m_played.end(); ) {
            if ((tick - (*it).tick) > max_audio_history_tick) {
                it = m_played.erase(it);
            } else {
                ++it;
            }
        }
    }
};