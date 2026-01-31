#pragma once

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/cereal.hpp>
#include <fstream>

#include <raylib.h>

namespace cereal {
    template <class Archive>
    void serialize(Archive& ar, ::Vector3& v) {
        ar(v.x, v.y, v.z);
    }
}