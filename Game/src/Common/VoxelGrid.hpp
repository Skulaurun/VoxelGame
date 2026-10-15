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
        if (isOutsideBounds(localPosition)) {
            return nullptr;
        }
        return &m_voxels[indexOf(localPosition)];
    }

    const Voxel* at(glm::ivec3 localPosition) const {
        if (isOutsideBounds(localPosition)) {
            return nullptr;
        }
        return &m_voxels[indexOf(localPosition)];
    }

    Voxel& operator[](std::size_t i) {
        return m_voxels[i];
    }

    const Voxel& operator[](std::size_t i) const {
        return m_voxels[i];
    }

    constexpr static auto isOutsideBounds(const glm::ivec3& localPosition) -> bool {
        return localPosition.x >= Size || localPosition.x < 0 ||
            localPosition.y >= Size || localPosition.y < 0 ||
            localPosition.z >= Size || localPosition.z < 0;
    }

    constexpr static auto indexOf(const glm::ivec3& localPosition) -> int32_t {
        return localPosition.y * Size * Size
            + localPosition.x * Size
            + localPosition.z;
    }

    auto voxels() -> auto& { return m_voxels; }
    auto voxels() const -> const auto& { return m_voxels; }

private:
    std::array<Voxel, CUBIC_SIZE> m_voxels;

};
