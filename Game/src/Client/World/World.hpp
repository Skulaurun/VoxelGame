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

public:
    World();

    World(World&&) = delete;
    World(const World&) = delete;

    World& operator=(World&&) = delete;
    World& operator=(const World&) = delete;

    auto updateChunk(const glm::ivec3& position) -> Chunk&;
    void deleteChunk(const glm::ivec3& position);

    template<typename Self>
    auto chunkAt(this Self&& self, glm::ivec3 position) {
        auto it = self.m_chunks.find(position);
        return (it != self.m_chunks.end()) ? std::forward_like<Self&>(it->second).get() : nullptr;
    }

    template<typename Self>
    auto voxelAt(this Self&& self, glm::ivec3 worldPosition) {
        auto* chunk = self.chunkAt(toChunkPosition(worldPosition));
        return chunk ? std::forward_like<Self&>(chunk->grid.at(toLocalPosition(worldPosition))) : nullptr;
    }

private:
    ChunkMap m_chunks;

};

}
