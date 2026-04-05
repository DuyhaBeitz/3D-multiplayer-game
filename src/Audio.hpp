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

constexpr int max_audio_history_tick = iters_per_sec*3;

enum SoundFlags : uint16_t {
    FLAG_SOUND_PHYISCS_DD = 1 << 0, // dynamic-dynamic collision
    FLAG_SOUND_PHYISCS_SD = 1 << 1,
    FLAG_SOUND_CONTINUOUS = 1 << 2,
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
        SoundKey _sound_key,
        float _duration_treshold = 1.0f
    ) 
    : hash(_flag, _actor_key1, _actor_key2, _tick), hit_pos(_hit_pos), rel_vel(_rel_vel), sound_key(_sound_key), duration_treshold(_duration_treshold) {
    }

    SoundEventHash hash;
    Vector3 hit_pos;
    Vector3 rel_vel;
    SoundKey sound_key;
    // for continuous, in seconds
    float duration_treshold = 1.0f;

    bool operator==(const SoundEvent& other) const {
        return  hash.flag == other.hash.flag &&
                hash.actor_key1 == other.hash.actor_key1 &&
                hash.actor_key2 == other.hash.actor_key2;
    }
};

struct SoundEventHashWithoutTick {
    size_t operator()(const SoundEvent& s) const noexcept {
        size_t h1 = std::hash<uint16_t>{}(s.hash.flag);
        size_t h2 = std::hash<uint16_t>{}(s.hash.actor_key1);
        size_t h3 = std::hash<uint16_t>{}(s.hash.actor_key2);
        // tick omitted
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

class Audio {
private:

    float m_sfx_volume = 1.0f;

    std::unordered_set<SoundEventHash> m_played{};
    std::unordered_set<SoundEvent, SoundEventHashWithoutTick> m_continuous{};
    std::unordered_set<SoundEvent, SoundEventHashWithoutTick> m_new_sounds{};

    Audio() {};
    ~Audio() {};

    void PlaySoundAtEvent(const SoundEvent& e) {
        m_new_sounds.emplace(e);
        Resources& r = Resources::Get();
        Rendering& rr = Rendering::Get();
        if (e.hash.flag & FLAG_SOUND_CONTINUOUS) {
            size_t h = SoundEventHashWithoutTick{}(e);
            int index = h % ALIASES_PER_SOUND;
            r.SoundFromKey(e.sound_key).PlayContinuous3D(index, rr.GetCamera(), e.hit_pos, 20);
            m_continuous.emplace(e);
        }
        else {
            size_t h = SoundEventHashWithoutTick{}(e);
            int index = h % ALIASES_PER_SOUND;
            // the difference is that this one doesn't get stopped
            r.SoundFromKey(e.sound_key).PlayContinuous3D(index, rr.GetCamera(), e.hit_pos, 20);
        }
    }

public:

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    static Audio& Get() {
        static Audio instance;
        return instance;
    }

    void Update(uint32_t tick) {
        Resources& r = Resources::Get();
        std::vector<SoundEvent> for_deletion;
        
        for (auto& e : m_continuous) {
            if (m_new_sounds.find(e) == m_new_sounds.end()) {
                size_t h = SoundEventHashWithoutTick{}(e);
                int index = h % ALIASES_PER_SOUND;
                r.SoundFromKey(e.sound_key).StopContinuous(index);
            }         
        }
        for (auto& e : for_deletion) {
            m_continuous.erase(e);
        }
        
        for (auto it = m_new_sounds.begin(); it != m_new_sounds.end(); ) {
            if ((tick - (*it).hash.tick) > (*it).duration_treshold*iters_per_sec) {
                it = m_new_sounds.erase(it);
            } else {
                ++it;
            }
        }
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

    float GetSFXVolume() { return m_sfx_volume; }
    void SetSFXVolume(float sfx_volume) { m_sfx_volume = sfx_volume; }
};