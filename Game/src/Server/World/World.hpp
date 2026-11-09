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

namespace Server {

class World {

public:
    using ChunkMap = std::unordered_map<glm::ivec3, Chunk::Holder>;

public:
    World();

    World(World&&) = delete;
    World(const World&) = delete;

    World& operator=(World&&) = delete;
    World& operator=(const World&) = delete;

    void update();

    auto castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) -> glm::ivec3;

    void addChunk(const glm::ivec3& position);
    auto setChunk(std::unique_ptr<Chunk> chunk, Chunk::State state) -> Chunk&;

    auto requestChunk(const glm::ivec3& position) -> Chunk::Holder&;

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

    void setMaxViewDistance(int32_t maxViewDistance) { m_maxViewDistance = maxViewDistance; }

    auto maxViewDistance() const { return m_maxViewDistance; }
    auto generator() -> WorldGenerator& { return m_generator; }

private:
    ChunkMap m_chunks;
    WorldGenerator m_generator;

    std::vector<glm::ivec3> m_chunksToGenerate;

    // TODO: should be config variable
    int32_t m_maxViewDistance;

};

}
