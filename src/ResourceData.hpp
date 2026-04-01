#pragma once

#include <cstdint>

/*
Needed because the server knows about render data, but doesn't know about rendering
*/

using ModelKey = uint16_t;
using FontKey = uint16_t;

constexpr ModelKey R_MODEL_NONE = 0; // doesn't correspond to model. isn't drawn at all
constexpr FontKey R_FONT_DEFAULT = 0;