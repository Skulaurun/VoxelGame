#include <Engine/World/World.hpp>
#include <Engine/Renderer/ChunkRenderer.hpp>

#include <tracy/Tracy.hpp>

#include <print>

void World::render(ChunkRenderer& renderer, const glm::vec3& playerPosition) {
    ZoneScoped;

    glm::ivec3 position = {
        playerPosition.x / Chunk::SIZE,
        playerPosition.y / Chunk::SIZE,
        playerPosition.z / Chunk::SIZE
    };

    int32_t viewRange = renderer.getViewRange();
    for (std::size_t i = 0; i < loadedChunks.size(); i++) {
        if (loadedChunks[i] && glm::distance(playerPosition, glm::vec3{loadedChunks[i]->getPosition() * Chunk::SIZE}) > viewRange * Chunk::SIZE) {
            loadedChunks[i] = nullptr;
            renderer.setEnabled(i, false);
        }
    }

    for (int32_t y = (position.y - viewRange); y <= (position.y + viewRange); y++) {
        for (int32_t x = (position.x - viewRange); x <= (position.x + viewRange); x++) {
            for (int32_t z = (position.z - viewRange); z <= (position.z + viewRange); z++) {
                glm::ivec3 chunkPosition{ x, y, z };

                if (glm::distance(playerPosition, glm::vec3{chunkPosition * Chunk::SIZE}) <= viewRange * Chunk::SIZE) {
                    Chunk* chunk = getChunkAt(chunkPosition);
                    if (chunk && !std::ranges::contains(loadedChunks, chunk)) {
                        // Load chunk
                        if (chunk->getMesh().indices.size() == 0) {
                            chunk->generateMesh(chunkPosition * Chunk::SIZE);
                        }
                        for (std::size_t i = 0; i < loadedChunks.size(); i++) {
                            if (loadedChunks[i] == nullptr) {
                                renderer.uploadMesh(chunk->getMesh(), i);
                                loadedChunks[i] = chunk;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void World::setViewRange(uint32_t viewRange) {
    this->viewRange = viewRange;
    const auto side = 2 * viewRange + 1;
    loadedChunks.resize(side * side * side, nullptr);
}

void World::setChunk(std::unique_ptr<Chunk> chunk) {
    std::scoped_lock lock{m_mtxChunks};
    chunks[chunk->getPosition()] = std::move(chunk);
}

Chunk* World::getChunkAt(glm::ivec3 position) {
    std::scoped_lock lock{m_mtxChunks};
    auto it = chunks.find(position);
    return (it != chunks.end()) ? it->second.get() : nullptr;
    return nullptr;
}
const Chunk* World::getChunkAt(glm::ivec3 position) const {
    std::scoped_lock lock{m_mtxChunks};
    auto it = chunks.find(position);
    return (it != chunks.end()) ? it->second.get() : nullptr;
    return nullptr;
}

glm::ivec3 World::toWorldPosition(glm::ivec3 chunkPosition, glm::ivec3 voxelPosition) {
    return glm::ivec3{
        chunkPosition.x * Chunk::SIZE + voxelPosition.x,
        chunkPosition.y * Chunk::SIZE + voxelPosition.y,
        chunkPosition.z * Chunk::SIZE + voxelPosition.z
    };
}
