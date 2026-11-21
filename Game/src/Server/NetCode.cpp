#include "NetCode.hpp"
#include "Server.hpp"

#include "Common/ECS/Component.hpp"

#include <print>

namespace Server {

ServerHandler::ServerHandler(Server& server) :
    m_server{server}
{}

void ServerHandler::operator()(const Network::Connect& event) {
    std::println("ServerHandler: Client connected to server");
    m_server.addClient(event.connection);
}

void ServerHandler::operator()(const Network::Disconnect& event) {
    std::println("ServerHandler: Client disconnected from the server");
    m_server.removeClient(event.connection);
}

void ServerHandler::operator()(Network::IncomingMessage& message) {
    MESSAGE_CALLBACK_MAP(*this, message);
}

/*************************************************************/
/*  Receiving: Server <- Client                              */
/*************************************************************/

void ServerHandler::onRequestViewDistance(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream) {
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

    Network::Message reply{ .id = Network::MessageId::SetViewDistance };
    reply.stream.write(effectiveViewDistance);
    sender->send(reply);
}

void ServerHandler::onInputCommand(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream) {    
    auto* client = m_server.findClient(sender);
    if (!client) {
        return;
    }

    auto& command = client->inputCommands.emplace_back();
    stream.read(command.eulerAngles);
    stream.read(command.movement);
    stream.read(command.isJump);
}

/*************************************************************/
/*  Sending: Server -> Client                                */
/*************************************************************/

void sendEntityUpdate(
    Network::Connection& connection,
    entt::entity playerEntity,
    const entt::registry& registry,
    const std::unordered_map<entt::entity, NetworkId>& entityToNetworkId,
    uint64_t tick
) {
    Network::Message message{ .id = Network::MessageId::EntityUpdate };
    Network::ProtocolStream& stream = message.stream;

    stream.write(tick);
    stream.write(entityToNetworkId.at(playerEntity));

    uint32_t entityCount{};
    auto lengthOffset = stream.head();
    stream.advance(sizeof(entityCount));

    for (auto [entity, position, velocity, rotation] : registry.view<Ecs::Position, Ecs::Velocity, Ecs::Rotation>().each()) {
        auto it = entityToNetworkId.find(entity);
        if (it == entityToNetworkId.end()) {
            continue;
        }

        stream.write(it->second);
        stream.write(position.value);
        stream.write(velocity.value);
        stream.write(rotation.value);
        entityCount++;
    }

    stream.seek(lengthOffset);
    stream.write(entityCount);

    connection.send(message);
}

void sendChunkUpdate(Network::Connection& connection, World& world, ClientChunkView& chunkView) {
    if (chunkView.events().empty()) {
        return;
    }

    Network::Message message{ .id = Network::MessageId::ChunkUpdate };
    Network::ProtocolStream& stream = message.stream;

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
