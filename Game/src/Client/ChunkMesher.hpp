#pragma once

#include "Client/World/Chunk.hpp"

#include <glm/glm.hpp>

#include <deque>
#include <mutex>

namespace Client {

class World;

class ChunkMesher {

public:
    constexpr static auto TEMP_SIZE = Chunk::SIZE + 2;

    struct Result {
        glm::ivec3 position;
        Mesh mesh;
    };

    struct Task {
        glm::ivec3 position;
        VoxelGrid<TEMP_SIZE> grid;
    };

public:
    ChunkMesher(World& world);

    void process(Chunk& chunk);

    void drain(std::invocable<Result&> auto drainer) {
        std::scoped_lock lock{m_completedMutex};
        while (!m_completedQueue.empty()) {
            drainer(m_completedQueue.front());
            m_completedQueue.pop_front();
        }
    }

private:
    void enqueue(const Chunk& chunk);
    void copyNeighborData(const Chunk& chunk, VoxelGrid<TEMP_SIZE>& grid);
    void mesh(const VoxelGrid<TEMP_SIZE>& grid, const glm::ivec3& chunkPosition, Mesh& mesh);
    void doWork(std::stop_token token);

private:
    World& m_world;

    std::mutex m_mutex;
    std::deque<std::unique_ptr<Task>> m_queue;

    std::mutex m_completedMutex;
    std::deque<Result> m_completedQueue;
    std::condition_variable m_workAvailable;

    std::jthread m_thread;
    
};

}
