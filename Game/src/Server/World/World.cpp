#include "Server/World/World.hpp"

#include <tracy/Tracy.hpp>

namespace Server {

World::World() :
    m_generator{*this}
{}

glm::ivec3 World::castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) {
    for (float i = 0.0f; i < maxDistance; i += 0.1f) {
        glm::ivec3 position = glm::floor(origin + i * direction);
        Voxel* voxel = voxelAt(position);
        if (voxel && static_cast<BlockType>(*voxel) != BlockType::Void) {
            return position;
        }
    }

    // FIXME:
    return {};
}

void World::setViewRange(uint32_t viewRange) {
    m_viewRange = viewRange;
}

void World::addChunk(glm::ivec3 position) {
    m_chunks[position] = Chunk::Holder{Chunk::State::Empty, nullptr};
}

void World::setChunk(std::unique_ptr<Chunk> chunk, Chunk::State state) {
    auto& holder = m_chunks[chunk->position];
    holder.state = state;
    holder.chunk = std::move(chunk);
}

}
