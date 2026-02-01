#pragma once

#include <cstdint>

/*
Needed because the server knows about render data, but doesn't know about rendering
*/

using ModelKey = uint16_t;
using FontKey = uint16_t;

constexpr ModelKey R_MODEL_DEFAULT = 0;
constexpr ModelKey R_MODEL_PLAYER = 1;
constexpr ModelKey R_MODEL_CUBE_EXCLAMATION = 2;
constexpr ModelKey R_MODEL_HEIGHTMAP0 = 3;
constexpr ModelKey R_MODEL_FOOTBALL = 4;
constexpr ModelKey R_MODEL_TREE = 5;

constexpr FontKey R_FONT_DEFAULT = 0;

#define P_HIEGHTMAP0_IMAGE_PATH "assets/Heightmap_01_Mountain.png"