#include "WorldGenerator.hpp"
#include "Server/World/World.hpp"

#include "Common/Util.hpp"

#include <Engine/Noise.hpp>

namespace Server {

WorldGenerator::WorldGenerator(World& world, std::size_t threadCount) :
    m_world{world},
    m_running{true}
{
    for (std::size_t i = 0; i < threadCount; i++) {
        m_threads.emplace_back(&WorldGenerator::doWork, this);
    }
}

WorldGenerator::~WorldGenerator() {
    {
        // Prevents notifying after checking predicate,
        // and before worker starts to wait (resulting in missed notification)
        std::scoped_lock lock{m_mutex};
        m_running.store(false);
    }
    m_workAvailable.notify_all();
}

void WorldGenerator::enqueue(const glm::vec3& coord) {
    std::scoped_lock lock{m_mutex};

    std::size_t begin = m_queue.size();
    m_queue.emplace_back(coord);

    prepareChunks(begin, begin + 1);
    m_workAvailable.notify_one();
}

void WorldGenerator::prepareChunks(std::size_t begin, std::size_t end) {
    for (std::size_t i = begin; i < end; i++) {
        const auto& position = m_queue[i];
        m_world.addChunk(position);
    }
}

// Terrain Generation
// -------------------------------------------------

void WorldGenerator::generateTerrain(Chunk& chunk) {
    ZoneScoped;

    for (int32_t x = 0; x < Chunk::SIZE; x++) {
        for (int32_t z = 0; z < Chunk::SIZE; z++) {
            auto worldX = toWorldPosition(chunk.position.x, x);
            auto worldZ = toWorldPosition(chunk.position.z, z);

            const float scale = 0.05f;
            const float baseHeight = 48.0f + perlinNoise(glm::vec2{worldX, worldZ} * scale) * 32.0f;

            const auto density = [scale, baseHeight](const glm::vec3& position) -> float {
                float bias = -1.0f + (position.y / baseHeight) * 2.0f;
                return perlinNoise(position * scale) - bias;
            };

            auto worldY = toWorldPosition(chunk.position.y, Chunk::SIZE - 1);
            glm::ivec3 worldPosition{worldX, worldY, worldZ};

            float aboveDensity = 0.0f;
            for (int32_t y = Chunk::SIZE - 1; y >= 0; y--, worldPosition.y--) {
                const float levelDensity = density(worldPosition);

                if (levelDensity > 0.0f) {
                    if (y == Chunk::SIZE - 1) {
                        aboveDensity = density(worldPosition + glm::ivec3{0, 1, 0});
                    }

                    BlockType type = BlockType::Stone;

                    if (aboveDensity <= 0.0f) {
                        type = BlockType::Grass;
                    }

                    *chunk.grid.at({x, y, z}) = static_cast<Voxel>(type);
                }
                aboveDensity = levelDensity;
            }
        }
    }
}

// River Generation
// -------------------------------------------------

// Cave Generation
// -------------------------------------------------

// Tree Generation
// -------------------------------------------------

auto WorldGenerator::saveToWorld(Result& result) -> Chunk& {
    return m_world.setChunk(std::move(result.chunk), result.state);
}

void WorldGenerator::doWork() {
    while (m_running.load()) {
        glm::vec3 coord{};
        {
            std::unique_lock lock{m_mutex};
            m_workAvailable.wait(lock, [&]{ return !m_queue.empty() || !m_running.load(); });
            if (!m_running.load()) {
                break;
            }
            
            coord = m_queue.front();
            m_queue.pop_front();
        }
        m_workAvailable.notify_one();
    
        auto chunk = std::make_unique<Chunk>(coord);
        generateTerrain(*chunk);

        {
            std::scoped_lock lock{m_completedMutex};
            m_completedQueue.emplace_back(Chunk::State::Done, std::move(chunk));
        }
    }
}

}
