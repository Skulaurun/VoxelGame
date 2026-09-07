#pragma once

#include <tracy/Tracy.hpp>

#include <glm/vec3.hpp>

#include <thread>
#include <vector>
#include <deque>
#include <mutex>

#ifdef TRACY_ENABLE
#define TracyCvHelper(cvClass, suffix) cvClass##suffix
#define TracyCv(cvClass, variable) TracyCvHelper(cvClass, _any) variable
#else
#define TracyCv(cvClass, variable) cvClass variable
#endif

class World;
class Chunk;

class WorldGenerator {

public:
    using ChunkQueue = std::deque<glm::vec3>;

public:
    WorldGenerator(World& world, std::size_t threadCount = std::thread::hardware_concurrency());
    ~WorldGenerator();

    void enqueue(const glm::vec3& coord);

    void enqueue(std::invocable<ChunkQueue&> auto&& pusher) {
        std::scoped_lock lock{m_mutex};

        std::size_t begin = m_queue.size();
        pusher(m_queue);
        std::size_t end = m_queue.size();
        
        prepareChunks(begin, end);
        m_workAvailable.notify_one();
    }

private:
    void doWork();
    void generate(Chunk& chunk);

    void prepareChunks(std::size_t begin, std::size_t end);

private:
    World& m_world;
    std::vector<std::jthread> m_threads;
    std::atomic<bool> m_running;

    TracyLockable(std::mutex, m_mutex);
    ChunkQueue m_queue;

    TracyCv(std::condition_variable, m_workAvailable);

};
