#include "NetCode.hpp"
#include "Client/Client.hpp"
#include "Client/World/Chunk.hpp"

#include <print>

namespace Client {

ClientHandler::ClientHandler(Client& client) :
    m_client{client}
{}

void ClientHandler::operator()(const Common::Network::Connect& message) {
    std::println("ClientHandler: Connected to server");
}

void ClientHandler::operator()(const Common::Network::Disconnect& message) {
    std::println("ClientHandler: Disconnected from server");
}

void ClientHandler::operator()(Common::Network::IncomingMessage& message) {
    MESSAGE_CALLBACK_MAP(*this, message);
}

void ClientHandler::onChunkUpdate(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream) {
    uint32_t unloadCount{};
    stream.read(unloadCount);

    for (uint32_t i = 0; i < unloadCount; i++) {
        glm::ivec3 position{};
        stream.read(position);

        // TODO:
        // m_chunkManager.unload();
        if (auto* chunk = m_client.world().chunkAt(position)) {
            //if (!chunk->mesh.indices.empty()) {
                m_client.chunkRenderer().unloadMesh(position);
            //}
            m_client.world().deleteChunk(position);
        }
    }

    uint32_t loadCount{};
    stream.read(loadCount);

    for (uint32_t i = 0; i < loadCount; i++) {
        glm::ivec3 position{};
        stream.read(position);

        auto chunk = m_client.world().updateChunk(position);
        stream.read(chunk.grid);

        // TODO:
        // m_chunkManager.load();
        m_client.chunkMesher().enqueue(chunk);
    }

    glm::vec3 playerChunk{};
    stream.read(playerChunk);
}

}
