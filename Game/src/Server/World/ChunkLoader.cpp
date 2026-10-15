#include "ChunkLoader.hpp"

#include "Common/WorldConfig.hpp"
#include "Common/Util.hpp"

#include "Server/World/Chunk.hpp"
#include "Server/World/World.hpp"

#include <glm/gtx/norm.hpp>

namespace Server {

ChunkLoader::ChunkLoader(World& world) : m_world{world}, m_lastPlayerChunk{INT32_MAX, INT32_MAX, INT32_MAX} {}

void ChunkLoader::update(const glm::vec3& playerPosition) {
    ZoneScoped;

    m_chunkEvents.clear();

    const auto playerChunk = toChunkPosition(playerPosition);
    if (m_lastPlayerChunk != playerChunk) {
        const int32_t viewRange = m_world.viewRange();
        const int32_t squaredViewRange{ (viewRange * WorldConfig::CHUNK_SIZE) * (viewRange * WorldConfig::CHUNK_SIZE) };
    
        auto isInViewRange = [&](const glm::ivec3& chunkPosition) {
            constexpr int32_t CENTER_OFFSET{ WorldConfig::CHUNK_SIZE / 2 };
            return glm::distance2(playerPosition, glm::vec3{chunkPosition * WorldConfig::CHUNK_SIZE + CENTER_OFFSET}) <= squaredViewRange;
        };
    
        for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
            if (m_loadedChunks[i].isOccupied()) {
                if (!isInViewRange(m_loadedChunks[i].position)) {
                    // Deactivate
                    m_chunkEvents.emplace_back(i, Event::Type::Unload, m_loadedChunks[i].position);
                    m_loadedChunks[i] = Slot{ 0, nullptr };
                }
            }
        }
    
        glm::ivec3 position = toChunkPosition(playerPosition);
        for (int32_t y = (position.y - viewRange); y <= (position.y + viewRange); y++) {
            for (int32_t x = (position.x - viewRange); x <= (position.x + viewRange); x++) {
                for (int32_t z = (position.z - viewRange); z <= (position.z + viewRange); z++) {
                    glm::ivec3 chunkPosition{ x, y, z };
    
                    if (isInViewRange(chunkPosition)) {
                        Chunk::Holder* holder = m_world.chunkHolderAt(chunkPosition);
                        if (holder) {
                            if (!std::ranges::contains(m_loadedChunks, holder, &Slot::holder)) {
                                // Activate chunk
                                for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
                                    if (!m_loadedChunks[i].isOccupied()) {
                                        m_loadedChunks[i] = Slot{ i, holder, chunkPosition, holder->state };
                                        
                                        if (holder->state == Chunk::State::Done) {
                                            m_chunkEvents.emplace_back(i, Event::Type::Load, chunkPosition);
                                        }
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
    m_lastPlayerChunk = playerChunk;

    for (std::size_t i = 0; i < m_loadedChunks.size(); i++) {
        Slot& slot = m_loadedChunks[i];
        if (slot.hasFinishedLoading()) {
            m_chunkEvents.emplace_back(i, Event::Type::Load, slot.position);
        }
    }
}

}
