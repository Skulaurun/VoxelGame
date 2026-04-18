#pragma once

#include <Engine/World/Chunk.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include <memory>
#include <unordered_map>
#include <deque>

class ChunkRenderer;

class World {

public:
    constexpr static int32_t CHUNK_SIZE = 8;

public:
    void generate();

    void render(ChunkRenderer& renderer, const glm::vec3& playerPosition);

    void setViewRange(uint32_t viewRange);

    Chunk* getChunkAt(glm::ivec3 position);
    const Chunk* getChunkAt(glm::ivec3 position) const;

    static glm::ivec3 toWorldPosition(glm::ivec3 chunkPosition, glm::ivec3 voxelPosition);

private:
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> chunks;
    std::vector<Chunk*> loadedChunks;
    std::deque<Chunk*> chunksToUpdate;

    uint32_t viewRange;

};
