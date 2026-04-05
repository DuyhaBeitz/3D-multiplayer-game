#pragma once
#include <raylib.h>
#include <raymath.h>

void SetSoundPosition(const Camera& listener, Sound sound, const Vector3& position, float maxDist, float volume_multiplier = 1.0f);

#define ALIASES_PER_SOUND 32

struct SoundPro {
    Sound aliases[ALIASES_PER_SOUND];
    float volume_multiplier = 1.0f;

    void Load(const char* filename) {
        aliases[0] = LoadSound(filename);
        for (int i = 1; i < ALIASES_PER_SOUND; i++){
            aliases[i] = LoadSoundAlias(aliases[0]);
        }
    }

    void Unload() {
        for (int i = 1; i < ALIASES_PER_SOUND; i++)
            UnloadSoundAlias(aliases[i]);
        UnloadSound(aliases[0]);
    }

    void Play();
    void Play3D(const Camera& listener, const Vector3& position, float maxDist);
    void PlayContinuous3D(int alias_index, const Camera& listener, const Vector3& position, float maxDist);

    void StopContinuous(int alias_index) {
        if (alias_index < 0 || alias_index >= ALIASES_PER_SOUND) return;
        StopSound(aliases[alias_index]);
    }
};