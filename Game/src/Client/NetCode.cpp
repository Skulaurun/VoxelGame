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

void ClientHandler::onChunkUpdate(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream) {
    uint32_t loadCount{};
    stream.read(loadCount);

    glm::vec3 playerChunk{};
    stream.read(playerChunk);

    for (uint32_t i = 0; i < loadCount; i++) {
        glm::ivec3 position{};
        stream.read(position);

        auto chunk = m_client.world().updateChunk(position);
        stream.read(chunk.grid);

        // TODO: debug
        m_client.chunkMesher().enqueue(chunk);

        // m_chunkManager.load();
    }

    uint32_t unloadCount{};
    stream.read(unloadCount);

    for (uint32_t i = 0; i < unloadCount; i++) {
        glm::ivec3 position{};
        stream.read(position);

        m_client.chunkRenderer().unloadMesh(static_cast<ChunkId>(std::hash<glm::ivec3>{}(position)));

        // m_chunkManager.unload();
    }
}

void ClientHandler::operator()(Common::Network::IncomingMessage& message) {
    if (message.id > Common::Network::MessageId::_Unknown_ && message.id < Common::Network::MessageId::_Count_) {
        (this->*MESSAGE_HANDLER_MAP[handlerIndex(message.id)])(message.sender, message.stream);
    }
}

}
