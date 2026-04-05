#include "SoundPro.hpp"
#include "Audio.hpp"

void SetSoundPosition(const Camera& listener, Sound sound, const Vector3& position, float maxDist, float volume_multiplier) {
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
    float pan = 0.5f - 0.5f * Vector3DotProduct(normalizedDirection, right);
    
    // Apply final sound properties
    SetSoundVolume(sound, attenuation*volume_multiplier);
    SetSoundPan(sound, pan);
};

void SoundPro::Play() {
    for (int i = 0; i < ALIASES_PER_SOUND; i++){
        if (!IsSoundPlaying(aliases[i])) {
            SetSoundVolume(aliases[i], volume_multiplier * Audio::Get().GetSFXVolume());
            PlaySound(aliases[i]);
            break;
        }
    }
}

void SoundPro::Play3D(const Camera &listener, const Vector3 &position, float maxDist) {
    for (int i = 0; i < ALIASES_PER_SOUND; i++){
        if (!IsSoundPlaying(aliases[i])) {
            SetSoundPosition(listener, aliases[i], position, maxDist, volume_multiplier * Audio::Get().GetSFXVolume());
            PlaySound(aliases[i]);
            break;
        }
    }
}

void SoundPro::PlayContinuous3D(int alias_index, const Camera &listener, const Vector3 &position, float maxDist) {
    if (alias_index < 0 || alias_index >= ALIASES_PER_SOUND) return;
    if (!IsSoundPlaying(aliases[alias_index])) {
        SetSoundPosition(listener, aliases[alias_index], position, maxDist, volume_multiplier * Audio::Get().GetSFXVolume());
        PlaySound(aliases[alias_index]);
    }
}