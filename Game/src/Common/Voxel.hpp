#pragma once

#include <cstdint>

enum class BlockType : uint32_t {
    Void,
    Grass,
    Stone
};

using Voxel = uint32_t;
