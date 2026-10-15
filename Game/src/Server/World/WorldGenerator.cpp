#include "WorldGenerator.hpp"
#include "Server/World/World.hpp"

#include "Common/Util.hpp"

#include <Engine/Noise.hpp>

namespace Server {

WorldGenerator::WorldGenerator(World& world, std::size_t threadCount) :
    m_world{world},
    m_running{true}
{
    for (std::size_t i = 1; i < threadCount; i++) {
        m_threads.emplace_back(&WorldGenerator::doWork, this);
    }
}

WorldGenerator::~WorldGenerator() {
    m_running.store(false);
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

float density(const glm::vec3& worldPosition) {
    const float scale = 0.05f;
    float baseHeight = 48.0f + perlinNoise(glm::vec2{worldPosition.x, worldPosition.z} * scale) * 32.0f;
    float bias = -1.0f + (worldPosition.y / baseHeight) * 2.0f;

    return perlinNoise(worldPosition * scale) - bias;
}

void WorldGenerator::generateTerrain(Chunk& chunk) {
    for (int32_t y = 0; y < Chunk::SIZE; y++) {
        for (int32_t x = 0; x < Chunk::SIZE; x++) {
            for (int32_t z = 0; z < Chunk::SIZE; z++) {
                glm::ivec3 worldPosition = toWorldPosition(chunk.position, {x, y, z});
                if (density(worldPosition) > 0.0f) {
                    BlockType type = BlockType::Stone;

                    if (density(worldPosition + glm::ivec3{0, 1, 0}) <= 0.0f) {
                        type = BlockType::Grass;
                    }

                    *chunk.grid.at({x, y, z}) = static_cast<Voxel>(type);
                }
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

void WorldGenerator::update() {
    std::scoped_lock lock{m_completedMutex};

    for (auto& result : m_completedQueue) {
        m_world.setChunk(std::move(result.chunk), result.state);
    }

    m_completedQueue.clear();
}

void WorldGenerator::doWork() {
    while (m_running.load()) {
        //ZoneScoped;

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
