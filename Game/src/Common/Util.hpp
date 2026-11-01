#pragma once

#include "WorldConfig.hpp"

#include <glm/glm.hpp>

// TODO: Implement WorldPosition, ChunkPosition, VoxelPosition

constexpr glm::ivec3 toChunkPosition(glm::ivec3 worldPosition) {
    glm::ivec3 c = worldPosition / WorldConfig::CHUNK_SIZE;
    glm::ivec3 v = worldPosition % WorldConfig::CHUNK_SIZE;
    return glm::ivec3{
        c.x - (v.x < 0),
        c.y - (v.y < 0),
        c.z - (v.z < 0)
    };
}

constexpr glm::ivec3 toLocalPosition(glm::ivec3 worldPosition) {
    return ((worldPosition % WorldConfig::CHUNK_SIZE) + WorldConfig::CHUNK_SIZE) % WorldConfig::CHUNK_SIZE;
}

constexpr glm::ivec3 toWorldPosition(glm::ivec3 chunkPosition, glm::ivec3 voxelPosition) {
    return glm::ivec3{
        chunkPosition.x * WorldConfig::CHUNK_SIZE + voxelPosition.x,
        chunkPosition.y * WorldConfig::CHUNK_SIZE + voxelPosition.y,
        chunkPosition.z * WorldConfig::CHUNK_SIZE + voxelPosition.z
    };
}

constexpr int32_t toWorldPosition(int32_t chunkPosition, int32_t voxelPosition) {
    return chunkPosition * WorldConfig::CHUNK_SIZE + voxelPosition;
}
