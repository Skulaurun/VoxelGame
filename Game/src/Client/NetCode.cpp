#include "NetCode.hpp"
#include "Client/Client.hpp"
#include "Client/World/Chunk.hpp"

#include "Client/ECS/Component.hpp"

#include <tracy/Tracy.hpp>

#include <print>

namespace Client {

ClientHandler::ClientHandler(Client& client) :
    m_client{client}
{}

void ClientHandler::operator()(const Network::Connect& message) {
    std::println("ClientHandler: Connected to server");
    sendViewDistanceRequest(*message.connection, m_client.options().renderDistance);
}

void ClientHandler::operator()(const Network::Disconnect& message) {
    std::println("ClientHandler: Disconnected from server");
}

void ClientHandler::operator()(Network::IncomingMessage& message) {
    MESSAGE_CALLBACK_MAP(*this, message);
}

void ClientHandler::onSetViewDistance(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream) {
    int32_t effectiveViewDistance{};
    stream.read(effectiveViewDistance);

    m_client.setViewDistance(effectiveViewDistance);
}

void ClientHandler::onChunkUpdate(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream) {
    ZoneScoped;

    uint32_t unloadCount{};
    stream.read(unloadCount);

    for (uint32_t i = 0; i < unloadCount; i++) {
        glm::ivec3 position{};
        stream.read(position);

        // TODO:
        // m_chunkManager.unload();
        if (auto* chunk = m_client.world().chunkAt(position)) {
            m_client.chunkMesher().removeChunk(position);

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

        auto [chunk, isNew] = m_client.world().getOrEmplaceChunk(position);
        stream.read(chunk.grid);

        // TODO:
        // m_chunkManager.load();
        m_client.chunkMesher().addChunk(chunk, isNew);
    }

    glm::vec3 playerChunk{};
    stream.read(playerChunk);

    TracyPlot("Chunk Loads", int64_t{loadCount});
    TracyPlot("Chunk Unloads", int64_t{unloadCount});
}

void ClientHandler::onEntityUpdate(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream) {
    uint64_t serverTick{};
    stream.read(serverTick);

    m_client.updateServerTime(serverTick * SIMULATION_TICK);

    NetworkId playerNetworkId{};
    stream.read(playerNetworkId);

    uint32_t entityCount{};
    stream.read(entityCount);

    for (std::size_t i = 0; i < entityCount; i++) {
        NetworkId id{};
        stream.read(id);

        if (playerNetworkId == id) {
            // TODO: temporary solution
            glm::vec3 position{};
            stream.read(position);
            stream.advance(sizeof(glm::vec3));
            stream.advance(sizeof(EntitySnapshot::rotation));
            m_client.camera().setPosition(position);
            continue;
        }

        auto [it, didEmplace] = m_entitySnaphots.try_emplace(id);
        if (it->second.size() == MAX_ENTITY_SNAPSHOT_COUNT) {
            it->second.pop_front();
        }

        auto& snapshot = it->second.emplace_back(serverTick * SIMULATION_TICK);
        stream.read(snapshot.position);
        stream.advance(sizeof(glm::vec3));
        stream.read(snapshot.rotation);

        if (didEmplace) {
            const auto spawnEntity = [](entt::registry& registry, const EntitySnapshot& snapshot) -> entt::entity {
                auto entity = registry.create();
                registry.emplace_or_replace<Ecs::Position>(entity, snapshot.position);
                registry.emplace_or_replace<Ecs::Rotation>(entity, snapshot.rotation);
                registry.emplace_or_replace<Ecs::NetworkInterpolated>(entity);
                return entity;
            };
            auto entity = spawnEntity(m_client.registry(), snapshot);
            std::get<0>(m_client.biEntityToNetworkId()).emplace(entity, id);
            std::get<1>(m_client.biEntityToNetworkId()).emplace(id, entity);
        }
    }
}

void sendInputCommand(Network::Connection& connection, const glm::vec2& eulerAngles, const glm::vec3& direction, bool isJump) {
    Network::Message message{ .id = Network::MessageId::InputCommand };
    Network::ProtocolStream& stream = message.stream;

    stream.write(eulerAngles);
    stream.write(direction);
    stream.write(isJump);

    connection.send(message);
}

void sendViewDistanceRequest(Network::Connection& connection, int32_t value) {
    Network::Message message{ .id = Network::MessageId::RequestViewDistance };
    Network::ProtocolStream& stream = message.stream;

    stream.write(value);
    connection.send(message);
}

}
