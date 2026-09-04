#include <Engine/World/ChunkManager.hpp>
#include <Engine/World/Chunk.hpp>
#include <Engine/World/World.hpp>

ChunkManager::ChunkManager(World& world) : m_world{world} {}

// FIXME: Performance degraded

void ChunkManager::update(const glm::vec3& playerPosition) {
    ZoneScoped;

    int32_t viewRange = m_world.viewRange();

    m_chunkEvents.clear();

    auto isInViewRange = [&](const glm::ivec3& chunkPosition) {
        const auto offset{ Chunk::SIZE / 2 };
        return glm::distance(playerPosition, glm::vec3{chunkPosition * Chunk::SIZE + offset}) <= viewRange * Chunk::SIZE;
    };

    for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
        if (m_loadedChunks[i].isOccupied()) {
            if (!isInViewRange(m_loadedChunks[i].chunk->getPosition())) {
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
                        if (!m_chunksInGeneration.contains(chunkPosition)) {
                            m_chunksInGeneration.emplace(chunkPosition);
                            
                            m_chunksToGenerate.emplace_back(chunkPosition);
                        }
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
