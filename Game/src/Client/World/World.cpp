#include "Client/World/World.hpp"

namespace Client {

World::World() {}

auto World::updateChunk(const glm::ivec3& position) -> Chunk& {
    auto& ptr = m_chunks[position];
    if (!ptr) {
        ptr = std::make_unique<Chunk>(position);
    }
    ptr->dirty = true;
    return *ptr;
}

}
