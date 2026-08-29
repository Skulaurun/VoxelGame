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
                    if (!chunk) {
                        continue;
                    }

                    auto it = std::ranges::find(loadedChunks, chunk);
                    if (it == loadedChunks.end()) {
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
                    } else if (chunk->isDirty()) {
                        // Reload chunk
                        chunk->generateMesh(chunkPosition * Chunk::SIZE);
                        std::size_t i = it - loadedChunks.begin();
                        renderer.uploadMesh(chunk->getMesh(), i);
                        chunk->setDirty(false);
                    }
                }
            }
        }
    }
}

glm::ivec3 World::castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    for (float i = 0.0f; i < maxDistance; i += 0.1f) {
        glm::ivec3 position = glm::round(origin + i * direction);
        Voxel* voxel = getVoxelAt(position);
        if (voxel && !voxel->isTransparent()) {
            return position;
        }
    }

    // FIXME:
    return {};
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

Voxel* World::getVoxelAt(glm::ivec3 worldPosition) {
    Chunk* chunk = getChunkAt(World::toChunkPosition(worldPosition));
    if (chunk == nullptr) {
        return nullptr;
    }

    return chunk->at(World::toLocalPosition(worldPosition));
}

glm::ivec3 World::toWorldPosition(glm::ivec3 chunkPosition, glm::ivec3 voxelPosition) {
    return glm::ivec3{
        chunkPosition.x * Chunk::SIZE + voxelPosition.x,
        chunkPosition.y * Chunk::SIZE + voxelPosition.y,
        chunkPosition.z * Chunk::SIZE + voxelPosition.z
    };
}

glm::ivec3 World::toLocalPosition(glm::ivec3 worldPosition) {
    return ((worldPosition % Chunk::SIZE) + Chunk::SIZE) % Chunk::SIZE;
}

glm::ivec3 World::toChunkPosition(glm::ivec3 worldPosition) {
    glm::ivec3 c = worldPosition / Chunk::SIZE;
    glm::ivec3 v = worldPosition % Chunk::SIZE;
    return glm::ivec3{
        c.x - (v.x < 0),
        c.y - (v.y < 0),
        c.z - (v.z < 0)
    };
}
