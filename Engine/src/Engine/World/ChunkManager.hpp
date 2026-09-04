#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>

class World;
class Chunk;

class ChunkManager {

public:
    struct Slot {
        std::size_t index;
        Chunk* chunk;

        bool isOccupied() const { return chunk != nullptr; }
    };

    struct Event {
        enum class Type : uint8_t {
            Load, Unload
        };

        std::size_t id;
        Type type;
    };

public:
    ChunkManager(World& world);

    void update(const glm::vec3& playerPosition);

    void resize(std::size_t newSize) {
        m_loadedChunks.resize(newSize, Slot{0, nullptr});
    }

    const auto& loadedChunks() const { return m_loadedChunks; }
    const auto& chunkEvents() const { return m_chunkEvents; }

private:
    World& m_world;

    std::vector<Slot> m_loadedChunks;

    // First are Unload events then Load events
    std::vector<Event> m_chunkEvents;

    std::vector<glm::ivec3> m_chunksToGenerate;
    std::unordered_set<glm::ivec3> m_chunksInGeneration;

};
