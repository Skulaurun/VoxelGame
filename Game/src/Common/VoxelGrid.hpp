#pragma once

#include "Voxel.hpp"

#include <Engine/Core/Math.hpp>

#include <array>

template<int32_t Size>
class VoxelGrid {

public:
    constexpr static auto CUBIC_SIZE = Size * Size * Size;

public:
    VoxelGrid() = default;

    // TODO: add non-bound checking methods

    Voxel* at(glm::ivec3 localPosition) {
        if (localPosition.x >= Size || localPosition.x < 0 ||
            localPosition.y >= Size || localPosition.y < 0 ||
            localPosition.z >= Size || localPosition.z < 0
        ) {
            return nullptr;
        }
        int32_t i = localPosition.y * Size * Size
            + localPosition.x * Size
            + localPosition.z;

        return &m_voxels[i];
    }

    Voxel& operator[](std::size_t i) {
        return m_voxels[i];
    }

private:
    std::array<Voxel, CUBIC_SIZE> m_voxels;

};
