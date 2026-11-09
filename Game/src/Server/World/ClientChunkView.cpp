#include "ClientChunkView.hpp"

#include "Common/Util.hpp"

#include "Server/Server.hpp"

#include <glm/gtx/norm.hpp>

namespace Server {

ClientChunkView::ClientChunkView(Server& server, ClientId id) :
    m_server{&server},
    m_clientId{id},
    m_viewDistance{0},
    m_lastPlayerChunk{0},
    m_lastViewDistance{-1}
{}

template<typename Fn>
void forEachBox(const AlignedBox3i& box, Fn&& fn) {
    for (int32_t y = box.min.y; y <= box.max.y; y++) {
        for (int32_t x = box.min.x; x <= box.max.x; x++) {
            for (int32_t z = box.min.z; z <= box.max.z; z++) {
                fn(glm::ivec3{x, y, z});
            }
        }
    }
}

template<typename Fn>
void forEachDifference(const AlignedBox3i& box, const AlignedBox3i& intersection, Fn&& fn) {
    const auto& bl = box.min;
    const auto& bh = box.max;
    const auto& il = intersection.min;
    const auto& ih = intersection.max;

    // X slabs
    forEachBox({bl, {il.x - 1, bh.y, bh.z}}, fn);
    forEachBox({{ih.x + 1, bl.y, bl.z}, bh}, fn);

    // Y slabs
    forEachBox({{il.x, bl.y, bl.z}, {ih.x, il.y - 1, bh.z}}, fn);
    forEachBox({{il.x, ih.y + 1, bl.z}, {ih.x, bh.y, bh.z}}, fn);

    // Z slabs
    forEachBox({{il.x, il.y, bl.z}, {ih.x, ih.y, il.z - 1}}, fn);
    forEachBox({{il.x, il.y, ih.z + 1}, {ih.x, ih.y, bh.z}}, fn);
}

void ClientChunkView::update(const glm::vec3& playerPosition) {
    const auto playerChunk = toChunkPosition(playerPosition);
    if (m_lastPlayerChunk != playerChunk || m_lastViewDistance != m_viewDistance) {
        const auto unloadChunk = [this](const glm::ivec3& position) {
            m_pendingChunks.erase(position);
            if (m_loadedChunks.erase(position) != 0) {
                m_chunkEvents.emplace_back(Event::Type::Unload, position);
            }
        };
        
        const auto loadChunk = [this](const glm::ivec3& position) {            
            if (m_server->requestClientChunk(m_clientId, position)) {
                if (auto [it, didEmplace] = m_loadedChunks.emplace(position); didEmplace) {
                    m_chunkEvents.emplace_back(Event::Type::Load, position);
                }
            } else {
                m_pendingChunks.emplace(position);
            }
        };

        const AlignedBox3i oldBox{m_lastPlayerChunk - m_lastViewDistance, m_lastPlayerChunk + m_lastViewDistance};
        const AlignedBox3i newBox{playerChunk - m_viewDistance, playerChunk + m_viewDistance};
        const auto intersection = oldBox.intersection(newBox);

        if (intersection.empty()) {
            forEachBox(oldBox, unloadChunk);
            forEachBox(newBox, loadChunk);
        } else {
            forEachDifference(oldBox, intersection, unloadChunk);
            forEachDifference(newBox, intersection, loadChunk);
        }
    }
    m_lastPlayerChunk = playerChunk;
    m_lastViewDistance = m_viewDistance;
}

void ClientChunkView::notifyChunkDone(const glm::ivec3& position) {
    if (m_pendingChunks.erase(position) == 0) {
        // The chunk is not needed anymore
        return;
    }
    if (auto [it, didEmplace] = m_loadedChunks.emplace(position); didEmplace) {
        m_chunkEvents.emplace_back(Event::Type::Load, position);
    }
}

void ClientChunkView::clearEvents() {
    m_chunkEvents.clear();
}

}
