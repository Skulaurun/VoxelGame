#pragma once

#include "Client/World/Chunk.hpp"

#include <Engine/Core/Core.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

namespace Client {

class World {

public:
    using ChunkMap = std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>>;
    using NeighborList = std::array<Chunk*, 6>;

    constexpr static auto CHUNK_NEIGHBOR_OFFSETS = std::to_array<glm::ivec3>({
        {-1, 0, 0}, // left
        {0, -1, 0}, // bottom
        {0, 0, -1}, // back
        {1, 0, 0},  // right
        {0, 1, 0},  // top
        {0, 0, 1}   // front
    });

public:
    World();

    World(World&&) = delete;
    World(const World&) = delete;

    World& operator=(World&&) = delete;
    World& operator=(const World&) = delete;

    auto updateChunk(const glm::ivec3& position) -> Chunk&;

    template<typename Self>
    auto chunkAt(this Self&& self, glm::ivec3 position)
        -> std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Chunk*, Chunk*> {
        auto it = self.m_chunks.find(position);
        return (it != self.m_chunks.end()) ? it->second.get() : nullptr;
    }

    template<typename Self>
    auto voxelAt(this Self&& self, glm::ivec3 worldPosition)
        -> std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Voxel*, Voxel*> {
        auto* chunk = self.chunkAt(toChunkPosition(worldPosition));
        return chunk ? chunk->grid.at(toLocalPosition(worldPosition)) : nullptr;
    }

    auto setPlayerPosition(const WorldPosition& position) { m_playerPosition = position; }

    auto playerPosition() const { return m_playerPosition; }

private:
    ChunkMap m_chunks;

    // TODO: temporary solution
    WorldPosition m_playerPosition;

};

}
