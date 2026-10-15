#include "NetCode.hpp"
#include "Server.hpp"

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

// TODO:
void ServerHandler::operator()(const Common::Network::IncomingMessage& event) {}

void sendChunkUpdate(Common::Network::Connection& connection, ChunkLoader& chunkLoader) {
    if (chunkLoader.chunkEvents().empty()) {
        return;
    }

    Common::Network::Message message{ .id = Common::Network::MessageId::ChunkUpdate };
    Common::Network::ProtocolStream& stream = message.stream;

    /*
    ChunkUpdate {
        uint32_t toLoadCount;
        Chunk[] {
            glm::ivec3 position;
            VoxelGrid grid;
        }
        uint32_t toUnloadCount;
        glm::ivec3[] chunkPositions;
    }
    */

    uint32_t chunkCount{};
    stream.advance(sizeof(chunkCount));

    stream.write(chunkLoader.playerChunk());

    std::vector<glm::ivec3> toUnload{};

    for (const auto& chunkEvent : chunkLoader.chunkEvents()) {
        if (chunkEvent.type == ChunkLoader::Event::Type::Load) {
            Chunk& chunk = *chunkLoader.loadedChunks()[chunkEvent.id].holder->chunk;
            stream.write(chunk.position);
            stream.write(chunk.grid);
            chunkCount++;
        } else if (chunkEvent.type == ChunkLoader::Event::Type::Unload) {
            toUnload.emplace_back(chunkEvent.position);
        }
    }

    stream.write(static_cast<uint32_t>(toUnload.size()));
    for (const auto& chunkPosition : toUnload) {
        stream.write(chunkPosition);
    }

    stream.seek(0);
    stream.write(chunkCount);

    connection.send(message);
}

}
