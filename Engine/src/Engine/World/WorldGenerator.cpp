#include <Engine/World/WorldGenerator.hpp>
#include <Engine/World/World.hpp>
#include <Engine/World/Noise.hpp>

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
    m_queue.emplace_back(coord);
    m_workAvailable.notify_one();
}

void WorldGenerator::doWork() {
    while (m_running.load()) {
        ZoneScoped;

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
    
        auto chunk = std::make_unique<Chunk>(m_world, coord);
        for (int32_t y = 0; y < Chunk::SIZE; y++) {
            for (int32_t x = 0; x < Chunk::SIZE; x++) {
                for (int32_t z = 0; z < Chunk::SIZE; z++) {
                    glm::vec3 worldPosition = chunk->toWorldPosition({x, y, z});
    
                    const float scale = 0.05f;
                    float baseHeight = 48.0f + perlinNoise(glm::vec2{worldPosition.x, worldPosition.z} * scale) * 32.0f;
                    float bias = -1.0f + (worldPosition.y / baseHeight) * 2.0f;
    
                    float density = perlinNoise(worldPosition * scale) - bias;
                    if (density > 0.0f) {
                        chunk->at({x, y, z})->setType(BlockType::Stone);
                    }
                }
            }
        }
    
        m_world.setChunk(std::move(chunk));
    }
}
