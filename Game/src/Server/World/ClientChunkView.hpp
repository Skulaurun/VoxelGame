#pragma once

#include "Common/Common.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_set>

namespace Server {

class Server;

class ClientChunkView {

public:
    struct Event {
        enum class Type : uint8_t {
            Load, Unload
        };

        Type type;
        glm::ivec3 position;
    };

public:
    ClientChunkView(Server& server, ClientId id);

    void update(const glm::vec3& playerPosition);
    void notifyChunkDone(const glm::ivec3& position);

    void clearEvents();
    void setViewDistance(int32_t viewDistance) { m_viewDistance = viewDistance; }

    auto playerChunk() const -> const auto& { return m_lastPlayerChunk; }

    auto events() const -> const auto& { return m_chunkEvents; }

private:
    Server* m_server;
    ClientId m_clientId;

    int32_t m_viewDistance;
    glm::ivec3 m_lastPlayerChunk;
    int32_t m_lastViewDistance;

    std::unordered_set<glm::ivec3> m_loadedChunks;
    std::unordered_set<glm::ivec3> m_pendingChunks;

    // First are Unload events then Load events
    std::vector<Event> m_chunkEvents;

};

}
