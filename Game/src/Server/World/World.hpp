#pragma once

#include "Server/World/Chunk.hpp"
#include "Server/World/ChunkManager.hpp"
#include "Server/World/WorldGenerator.hpp"

#include <tracy/Tracy.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include <memory>
#include <unordered_map>

class ChunkRenderer;

class World {

public:
    constexpr static int32_t CHUNK_SIZE = 8;

public:
    World();

    World(World&&) = delete;
    World(const World&) = delete;

    World& operator=(World&&) = delete;
    World& operator=(const World&) = delete;

    glm::ivec3 castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance);

    void setViewRange(uint32_t viewRange);

    void setChunk(std::unique_ptr<Chunk> chunk);

    Chunk* getChunkAt(glm::ivec3 position);
    const Chunk* getChunkAt(glm::ivec3 position) const;

    Voxel* getVoxelAt(glm::ivec3 worldPosition);

    auto viewRange() const { return m_viewRange; }

    ChunkManager& chunkManager() { return m_chunkManager; }
    WorldGenerator& generator() { return m_generator; }

    static glm::ivec3 toWorldPosition(glm::ivec3 chunkPosition, glm::ivec3 voxelPosition);
    static glm::ivec3 toChunkPosition(glm::ivec3 worldPosition);
    static glm::ivec3 toLocalPosition(glm::ivec3 worldPosition);

private:
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> m_chunks;

    ChunkManager m_chunkManager;
    WorldGenerator m_generator;

    uint32_t m_viewRange;

    mutable TracyLockable(std::mutex, m_mtxChunks);

};
