#include "Server/World/World.hpp"

#include <tracy/Tracy.hpp>

namespace Server {

World::World() :
    m_generator{*this}
{}

void World::update() {
    if (!m_chunksToGenerate.empty()) {
        m_generator.enqueue([&](WorldGenerator::ChunkQueue& queue) {
            queue.insert(
                queue.end(),
                std::make_move_iterator(m_chunksToGenerate.begin()),
                std::make_move_iterator(m_chunksToGenerate.end())
            );
        });
        m_chunksToGenerate.clear();
    }
}

auto World::castRay(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) -> glm::ivec3 {
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

void World::addChunk(const glm::ivec3& position) {
    m_chunks.try_emplace(position, Chunk::Holder{Chunk::State::Empty, nullptr});
}

auto World::setChunk(std::unique_ptr<Chunk> chunk, Chunk::State state) -> Chunk& {
    auto& holder = m_chunks[chunk->position];
    holder.state = state;
    holder.chunk = std::move(chunk);
    
    return *holder.chunk.get();
}

auto World::requestChunk(const glm::ivec3& position) -> Chunk::Holder& {
    auto [it, didEmplace] = m_chunks.try_emplace(position,
        Chunk::Holder{Chunk::State::Empty, nullptr}
    );

    if (didEmplace) {
        m_chunksToGenerate.emplace_back(position);
    }

    return it->second;
}

// void World::ChunkSubscriber::request(const glm::ivec3& position) {
//     Chunk::Holder* holder = chunkHolderAt(position);
//     if (holder && holder->state == Chunk::State::Done) {
        
//     } else {
//         // Generate or load chunk
//         m_chunksToGenerate.emplace_back(position);
//     }

//     // TODO: add subscribtion
// }

}
