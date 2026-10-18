#include "NetCode.hpp"
#include "Server.hpp"

#include <Server/ECS/Component.hpp>

#include <print>

namespace Server {

ServerHandler::ServerHandler(Server& server) :
    m_server{server}
{}

void ServerHandler::operator()(const Common::Network::Connect& event) {
    std::println("ServerHandler: Client connected to server");
    m_server.addClient(event.connection);
}

void ServerHandler::operator()(const Common::Network::Disconnect& event) {
    std::println("ServerHandler: Client disconnected from the server");
}

void ServerHandler::operator()(Common::Network::IncomingMessage& message) {
    MESSAGE_CALLBACK_MAP(*this, message);
}

void ServerHandler::onPlayerMove(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream) {
    glm::vec3 playerPosition{};
    stream.read(playerPosition);
    
    auto* client = m_server.findClient(sender);
    if (!client) {
        return;
    }

    m_server.registry().get<Ecs::Position>(client->playerEntity).value = playerPosition;
}

void sendChunkUpdate(Common::Network::Connection& connection, ChunkLoader& chunkLoader) {
    if (chunkLoader.chunkEvents().empty()) {
        return;
    }

    Common::Network::Message message{ .id = Common::Network::MessageId::ChunkUpdate };
    Common::Network::ProtocolStream& stream = message.stream;

    uint32_t loadCount{};
    uint32_t unloadCount{};

    for (const auto& chunkEvent : chunkLoader.chunkEvents()) {
        if (chunkEvent.type == ChunkLoader::Event::Type::Load) {
            loadCount++;
        } else if (chunkEvent.type == ChunkLoader::Event::Type::Unload) {
            unloadCount++;
        }
    }

    std::size_t unloadPosition = 0;
    stream.write(unloadCount);
    unloadPosition = stream.head();

    std::size_t loadPosition = unloadCount * sizeof(glm::ivec3);
    stream.advance(loadPosition);
    stream.write(loadCount);
    loadPosition = stream.head();

    for (const auto& chunkEvent : chunkLoader.chunkEvents()) {
        if (chunkEvent.type == ChunkLoader::Event::Type::Load) {
            stream.seek(loadPosition);
            {
                Chunk& chunk = *chunkLoader.loadedChunks()[chunkEvent.id].holder->chunk;
                stream.write(chunk.position);
                stream.write(chunk.grid);
            }
            loadPosition = stream.head();
        } else if (chunkEvent.type == ChunkLoader::Event::Type::Unload) {
            stream.seek(unloadPosition);
            {
                stream.write(chunkEvent.position);
            }
            unloadPosition = stream.head();
        }
    }

    stream.seek(loadPosition);
    stream.write(chunkLoader.playerChunk());

    connection.send(message);
}

}
