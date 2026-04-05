#pragma once

#include <cstdint>
#include <raylib.h>

/*
Needed because the server knows about render data, but doesn't know about rendering
*/

using ModelKey = uint16_t;
using FontKey = uint16_t;
using SoundKey = uint16_t;

constexpr ModelKey R_MODEL_NONE = 0; // doesn't correspond to model. isn't drawn at all
constexpr FontKey R_FONT_DEFAULT = 0;

constexpr FontKey R_SOUND_DEFAULT = 0;
constexpr FontKey R_SOUND_WALK = 1;

Image LoadImageFromPerlinNoise(uint32_t seed, int w, int h, Vector2 uv = {1.0f, 1.0f}, int octaves = 4);
Image LoadImageORM(const char *aoPath, const char *roughnessPath, const char *metallicPath);