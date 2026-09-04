#include <Engine/World/World.hpp>

#include <tracy/Tracy.hpp>

World::World() : m_generator{*this}, m_chunkManager{*this} {}

glm::ivec3 World::castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    for (float i = 0.0f; i < maxDistance; i += 0.1f) {
        glm::ivec3 position = glm::floor(origin + i * direction);
        Voxel* voxel = getVoxelAt(position);
        if (voxel && !voxel->isTransparent()) {
            return position;
        }
    }

    // FIXME:
    return {};
}

void World::setViewRange(uint32_t viewRange) {
    m_viewRange = viewRange;

    const auto side = 2 * viewRange + 1;
    m_chunkManager.resize(side * side * side);
}

void World::setChunk(std::unique_ptr<Chunk> chunk) {
    std::scoped_lock lock{m_mtxChunks};
    m_chunks[chunk->getPosition()] = std::move(chunk);
}

Chunk* World::getChunkAt(glm::ivec3 position) {
    std::scoped_lock lock{m_mtxChunks};
    auto it = m_chunks.find(position);
    return (it != m_chunks.end()) ? it->second.get() : nullptr;
    return nullptr;
}
const Chunk* World::getChunkAt(glm::ivec3 position) const {
    std::scoped_lock lock{m_mtxChunks};
    auto it = m_chunks.find(position);
    return (it != m_chunks.end()) ? it->second.get() : nullptr;
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
