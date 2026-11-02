#include "Client/World/World.hpp"

namespace Client {

World::World() {}

auto World::getOrEmplaceChunk(const glm::ivec3& position) -> std::tuple<Chunk&, bool> {
    auto& chunk = m_chunks[position];
    if (!chunk) {
        chunk = std::make_unique<Chunk>(position);
        return {*chunk, true};
    }
    return {*chunk, false};
}

void World::deleteChunk(const glm::ivec3& position) {
    m_chunks.erase(position);
}

}
