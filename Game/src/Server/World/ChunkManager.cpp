#include "Server/World/ChunkManager.hpp"
#include "Server/World/Chunk.hpp"
#include "Server/World/World.hpp"

#include <glm/gtx/norm.hpp>

ChunkManager::ChunkManager(World& world) : m_world{world}, m_lastPlayerChunk{INT32_MAX, INT32_MAX, INT32_MAX} {}

// FIXME: Performance degraded (why the lag spikes?)

void ChunkManager::update(const glm::vec3& playerPosition) {
    ZoneScoped;

    m_chunkEvents.clear();

    // for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
    //     if (m_loadedChunks[i].isOccupied()) {
    //         auto state = m_loadedChunks[i].chunk->state();
    //         if (state == Chunk::State::Generated) {
    //             m_chunkEvents.emplace_back(i, Event::Type::Load);
    //         }
    //     }
    // }

    // TODO: this can cause chunk to Load & Unload in same frame

    const auto playerChunk = World::toChunkPosition(playerPosition);
    if (m_lastPlayerChunk == playerChunk) {
        return;
    }
    m_lastPlayerChunk = playerChunk;

    const int32_t viewRange = m_world.viewRange();
    const int32_t squaredViewRange{ (viewRange * Chunk::SIZE) * (viewRange * Chunk::SIZE) };

    auto isInViewRange = [&](const glm::ivec3& chunkPosition) {
        constexpr int32_t CENTER_OFFSET{ Chunk::SIZE / 2 };
        return glm::distance2(playerPosition, glm::vec3{chunkPosition * Chunk::SIZE + CENTER_OFFSET}) <= squaredViewRange;
    };

    for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
        if (m_loadedChunks[i].isOccupied()) {
            if (!isInViewRange(m_loadedChunks[i].chunk->position())) {
                // Deactivate
                m_loadedChunks[i] = Slot{ 0, nullptr };
                m_chunkEvents.emplace_back(i, Event::Type::Unload);
            }
        }
    }

    glm::ivec3 position = World::toChunkPosition(playerPosition);
    for (int32_t y = (position.y - viewRange); y <= (position.y + viewRange); y++) {
        for (int32_t x = (position.x - viewRange); x <= (position.x + viewRange); x++) {
            for (int32_t z = (position.z - viewRange); z <= (position.z + viewRange); z++) {
                glm::ivec3 chunkPosition{ x, y, z };

                if (isInViewRange(chunkPosition)) {
                    Chunk* chunk = m_world.getChunkAt(chunkPosition);
                    if (chunk) {
                        if (chunk->state() == Chunk::State::Empty) {
                            continue;
                        }
                        if (!std::ranges::contains(m_loadedChunks, chunk, &Slot::chunk)) {
                            // Activate chunk
                            for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
                                if (!m_loadedChunks[i].isOccupied()) {
                                    m_loadedChunks[i] = Slot{ i, chunk };
                                    m_chunkEvents.emplace_back(i, Event::Type::Load);
                                    break;
                                }
                            }
                        }
                    } else {
                        // Generate or load chunk
                        m_chunksToGenerate.emplace_back(chunkPosition);
                    }
                }
            }
        }
    }

    if (!m_chunksToGenerate.empty()) {
        m_world.generator().enqueue([&](WorldGenerator::ChunkQueue& queue) {
            queue.insert(
                queue.end(),
                std::make_move_iterator(m_chunksToGenerate.begin()),
                std::make_move_iterator(m_chunksToGenerate.end())
            );
        });
    
        m_chunksToGenerate.clear();
    }
}
