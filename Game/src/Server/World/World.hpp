#pragma once

#include "Server/World/Chunk.hpp"
#include "Server/World/WorldGenerator.hpp"

#include "Common/Util.hpp"

#include <tracy/Tracy.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include <memory>
#include <unordered_map>

class World {

public:
    using ChunkMap = std::unordered_map<glm::ivec3, Chunk::Holder>;

public:
    World();

    World(World&&) = delete;
    World(const World&) = delete;

    World& operator=(World&&) = delete;
    World& operator=(const World&) = delete;

    glm::ivec3 castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);

    void setViewRange(uint32_t viewRange);

    void addChunk(glm::ivec3 position);
    void setChunk(std::unique_ptr<Chunk> chunk, Chunk::State state);

    template<typename Self>
    auto chunkHolderAt(this Self&& self, glm::ivec3 position)
        -> std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Chunk::Holder*, Chunk::Holder*> {
        auto it = self.m_chunks.find(position);
        return (it != self.m_chunks.end()) ? &it->second : nullptr;
    }

    template<typename Self>
    auto chunkAt(this Self&& self, glm::ivec3 position)
        -> std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Chunk*, Chunk*> {
        auto it = self.m_chunks.find(position);
        return (it != self.m_chunks.end()) ? it->second.chunk.get() : nullptr;
    }

    template<typename Self>
    auto voxelAt(this Self&& self, glm::ivec3 worldPosition)
        -> std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, const Voxel*, Voxel*> {
        auto* chunk = self.chunkAt(toChunkPosition(worldPosition));
        return chunk ? chunk->grid.at(toLocalPosition(worldPosition)) : nullptr;
    }

    auto viewRange() const { return m_viewRange; }

    WorldGenerator& generator() { return m_generator; }

private:
    ChunkMap m_chunks;

    WorldGenerator m_generator;

    uint32_t m_viewRange;

    //mutable TracyLockable(std::mutex, m_mtxChunks);

};
