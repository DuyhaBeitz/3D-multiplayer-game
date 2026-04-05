#pragma once
#include <raylib.h>
#include <raymath.h>

static void SetSoundPosition(const Camera& listener, Sound sound, const Vector3& position, float maxDist) {
    // Calculate direction vector and distance between listener and sound source
    Vector3 direction = Vector3Subtract(position, listener.position);
    float distance = Vector3Length(direction);
    
    // Apply logarithmic distance attenuation and clamp between 0-1
    float attenuation = 1.0f / (1.0f + (distance / maxDist));
    attenuation = Clamp(attenuation, 0.0f, 1.0f);
    
    // Calculate normalized vectors for spatial positioning
    Vector3 normalizedDirection = Vector3Normalize(direction);
    Vector3 forward = Vector3Normalize(Vector3Subtract(listener.target, listener.position));
    Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, listener.up));
    
    // Reduce volume for sounds behind the listener
    float dotProduct = Vector3DotProduct(forward, normalizedDirection);
    if (dotProduct < 0.0f) {
        attenuation *= (1.0f + dotProduct * 0.5f);
    }
    
    // Set stereo panning based on sound position relative to listener
    float pan = 0.5f + 0.5f * Vector3DotProduct(normalizedDirection, right);
    
    // Apply final sound properties
    SetSoundVolume(sound, attenuation);
    SetSoundPan(sound, pan);
}

#define ALIASES_PER_SOUND 10

struct SoundPro {
    Sound aliases[ALIASES_PER_SOUND];

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

    void Play() {
        for (int i = 0; i < ALIASES_PER_SOUND; i++){
            if (!IsSoundPlaying(aliases[i])) {
                PlaySound(aliases[i]);
                break;
            }
        }
    }

    void Play3D(const Camera& listener, const Vector3& position, float maxDist) {
        for (int i = 0; i < ALIASES_PER_SOUND; i++){
            if (!IsSoundPlaying(aliases[i])) {
                SetSoundPosition(listener, aliases[i], position, maxDist);
                PlaySound(aliases[i]);
                break;
            }
        }
    }
};