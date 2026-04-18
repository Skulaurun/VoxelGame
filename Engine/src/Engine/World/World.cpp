#include <Engine/World/World.hpp>
#include <Engine/Renderer/ChunkRenderer.hpp>

#include <tracy/Tracy.hpp>

#include <print>

void World::generate() {
    ZoneScoped;

    for (int32_t y = -8; y <= 8; y++) {
        for (int32_t x = -8; x <= 8; x++) {
            for (int32_t z = -8; z <= 8; z++) {
                glm::ivec3 coord(x, y, z);
                chunks[coord] = std::make_unique<Chunk>(*this, coord);
            }
        }
    }
}

void World::render(ChunkRenderer& renderer, const glm::vec3& playerPosition) {
    ZoneScoped;

    glm::ivec3 position = {
        playerPosition.x / Chunk::SIZE,
        playerPosition.y / Chunk::SIZE,
        playerPosition.z / Chunk::SIZE
    };

    int32_t viewRange = renderer.getViewRange();
    for (std::size_t i = 0; i < loadedChunks.size(); i++) {
        if (loadedChunks[i] && glm::distance(playerPosition, glm::vec3(loadedChunks[i]->getPosition() * Chunk::SIZE)) > viewRange * Chunk::SIZE) {
            loadedChunks[i] = nullptr;
            renderer.setEnabled(i, false);
        }
    }

    for (int32_t y = (position.y - viewRange); y <= (position.y + viewRange); y++) {
        for (int32_t x = (position.x - viewRange); x <= (position.x + viewRange); x++) {
            for (int32_t z = (position.z - viewRange); z <= (position.z + viewRange); z++) {
                Chunk* chunk = getChunkAt(glm::ivec3{ x, y, z });
                if (chunk && glm::distance(playerPosition, glm::vec3(chunk->getPosition() * Chunk::SIZE)) <= viewRange * Chunk::SIZE) {
                    if (!std::ranges::contains(loadedChunks, chunk)) {
                        // Load chunk
                        if (chunk->getMesh().indices.size() == 0) {
                            chunk->generateMesh(chunk->getPosition() * Chunk::SIZE);
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

Chunk* World::getChunkAt(glm::ivec3 position) {
    auto it = chunks.find(position);
    return (it != chunks.end()) ? it->second.get() : nullptr;
    return nullptr;
}
const Chunk* World::getChunkAt(glm::ivec3 position) const {
    auto it = chunks.find(position);
    return (it != chunks.end()) ? it->second.get() : nullptr;
    return nullptr;
}

glm::ivec3 World::toWorldPosition(glm::ivec3 chunkPosition, glm::ivec3 voxelPosition) {
    return glm::ivec3(
        chunkPosition.x * Chunk::SIZE + voxelPosition.x,
        chunkPosition.y * Chunk::SIZE + voxelPosition.y,
        chunkPosition.z * Chunk::SIZE + voxelPosition.z
    );
}
