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
    m_server.removeClient(event.connection);
}

void ServerHandler::operator()(Common::Network::IncomingMessage& message) {
    MESSAGE_CALLBACK_MAP(*this, message);
}

void ServerHandler::onRequestViewDistance(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream) {
    int32_t requestedViewDistance{};
    stream.read(requestedViewDistance);

    auto* client = m_server.findClient(sender);
    if (!client) {
        return;
    }

    int32_t effectiveViewDistance = std::min(
        std::max(requestedViewDistance, 0),
        static_cast<int32_t>(m_server.world().maxViewDistance())
    );
    client->chunkView.setViewDistance(effectiveViewDistance);

    Common::Network::Message reply{ .id = Common::Network::MessageId::SetViewDistance };
    reply.stream.write(effectiveViewDistance);
    sender->send(reply);
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

void sendChunkUpdate(Common::Network::Connection& connection, World& world, ClientChunkView& chunkView) {
    if (chunkView.events().empty()) {
        return;
    }

    Common::Network::Message message{ .id = Common::Network::MessageId::ChunkUpdate };
    Common::Network::ProtocolStream& stream = message.stream;

    uint32_t loadCount{};
    uint32_t unloadCount{};

    for (const auto& chunkEvent : chunkView.events()) {
        if (chunkEvent.type == ClientChunkView::Event::Type::Load) {
            loadCount++;
        } else if (chunkEvent.type == ClientChunkView::Event::Type::Unload) {
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

    for (const auto& chunkEvent : chunkView.events()) {
        if (chunkEvent.type == ClientChunkView::Event::Type::Load) {
            stream.seek(loadPosition);
            {
                Chunk& chunk = *world.chunkAt(chunkEvent.position);
                stream.write(chunk.position);
                stream.write(chunk.grid);
            }
            loadPosition = stream.head();
        } else if (chunkEvent.type == ClientChunkView::Event::Type::Unload) {
            stream.seek(unloadPosition);
            {
                stream.write(chunkEvent.position);
            }
            unloadPosition = stream.head();
        }
    }

    chunkView.clearEvents();

    stream.seek(loadPosition);
    stream.write(chunkView.playerChunk());

    connection.send(message);
}

}
