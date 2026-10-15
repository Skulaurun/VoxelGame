#pragma once

#include "Server/World/Chunk.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace Server {

class World;

class ChunkLoader {

public:
    struct Slot {
        std::size_t index;
        Chunk::Holder* holder;
        glm::ivec3 position;
        Chunk::State lastState;

        bool hasFinishedLoading() {
            if (!isOccupied()) return false;
            bool hasChanged = holder->state != lastState;
            lastState = holder->state;
            return hasChanged && holder->state == Chunk::State::Done;
        }

        bool isOccupied() const { return holder != nullptr; }
    };

    struct Event {
        enum class Type : uint8_t {
            Load, Unload
        };

        std::size_t id;
        Type type;
        glm::ivec3 position;
    };

public:
    ChunkLoader(World& world);

    void update(const glm::vec3& playerPosition);

    void resize(std::size_t newSize) {
        m_loadedChunks.resize(newSize, Slot{0, nullptr});
    }

    auto playerChunk() const -> const auto& { return m_lastPlayerChunk; }

    const auto& loadedChunks() const { return m_loadedChunks; }
    const auto& chunkEvents() const { return m_chunkEvents; }

private:
    World& m_world;

    glm::ivec3 m_lastPlayerChunk;

    std::vector<Slot> m_loadedChunks;

    // First are Unload events then Load events
    std::vector<Event> m_chunkEvents;

    std::vector<glm::ivec3> m_chunksToGenerate;

};

}
