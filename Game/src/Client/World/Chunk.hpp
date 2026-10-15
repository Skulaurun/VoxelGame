#pragma once

#include "Common/WorldConfig.hpp"
#include "Common/VoxelGrid.hpp"
#include "Common/Util.hpp"

#include <Engine/Mesh.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Core/Math.hpp>

#include <glm/glm.hpp>

namespace Client {

using ChunkId = uint64_t;

struct Chunk {
    constexpr static auto SIZE = WorldConfig::CHUNK_SIZE;

    using VoxelGrid = VoxelGrid<SIZE>;
    
    constexpr static uint32_t MAX_FACES = 6 * ((VoxelGrid::CUBIC_SIZE + 1) / 2);
    
    glm::ivec3 position;
    VoxelGrid grid;

    Mesh mesh;
    bool dirty;

    constexpr static auto box(const glm::ivec3& position) -> Box3 {
        return {
            toWorldPosition(position, {0, 0, 0}),
            toWorldPosition(position, {SIZE, SIZE, SIZE})
        };
    }
};

}
