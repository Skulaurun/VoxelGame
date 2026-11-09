#include "Server.hpp"

#include "NetCode.hpp"
#include "ECS/Component.hpp"

namespace Server {

constexpr int32_t PREGENERATED_RANGE{ 8 };

Server::Server() :
    m_running{false},
    m_messageHandler{std::make_shared<ServerHandler>(*this)},
    m_netServer{m_messageHandler, 5500}, // TODO: port
    m_lastClientId{0}
{
    // Server init
    m_world.setMaxViewDistance(64);

    m_world.generator().enqueue([&](WorldGenerator::ChunkQueue& queue) {
        for (int32_t y = -PREGENERATED_RANGE; y < PREGENERATED_RANGE; y++) {
            for (int32_t x = -PREGENERATED_RANGE; x < PREGENERATED_RANGE; x++) {
                for (int32_t z = -PREGENERATED_RANGE; z < PREGENERATED_RANGE; z++) {
                    queue.emplace_back(x, y, z);
                }
            }
        }
    });

    // Network init
    m_netServer.start();
}

Server::~Server() {
    stop();
}

void Server::addClient(std::shared_ptr<Common::Network::Connection> connection) {
    entt::entity playerEntity = m_registry.create();
    m_registry.emplace_or_replace<Ecs::Position>(playerEntity);

    const auto clientId = m_lastClientId++;
    const auto index = m_clients.size();
    auto& client = m_clients.emplace_back(clientId, playerEntity, ClientChunkView{*this, clientId}, connection);

    m_idToClient.emplace(clientId, index);
    m_connectionToClient.emplace(connection.get(), index);
}

void Server::removeClient(const std::shared_ptr<Common::Network::Connection>& connection) {
    if (auto clientPtr = findClient(connection)) {
        m_registry.destroy(clientPtr->playerEntity);

        m_idToClient.erase(clientPtr->id);
        m_connectionToClient.erase(connection.get());

        if (clientPtr != &m_clients.back()) {
            *clientPtr = std::move(m_clients.back());

            const auto index = static_cast<std::size_t>(clientPtr - m_clients.data());
            m_idToClient[clientPtr->id] = index;
            m_connectionToClient[clientPtr->connection.get()] = index;
        }
        m_clients.pop_back();
    } else {
        assert(false && "Cannot disconnect non-existent Client?");
    }
}

auto Server::findClient(ClientId id) -> ClientState* {
    auto it = m_idToClient.find(id);
    return (it != m_idToClient.end()) ? &m_clients[it->second] : nullptr;
}

auto Server::findClient(const std::shared_ptr<Common::Network::Connection>& connection) -> ClientState* {
    auto it = m_connectionToClient.find(connection.get());
    return (it != m_connectionToClient.end()) ? &m_clients[it->second] : nullptr;
}

auto Server::requestClientChunk(ClientId id, const glm::ivec3& position) -> bool {
    if (m_world.requestChunk(position).state == Chunk::State::Done) {
        return true;
    } else {
        m_requestedChunks[position].emplace_back(id);
        return false;
    }
}

void Server::update() {
    m_netServer.update();

    for (auto& client : m_clients) {
        const auto& position = m_registry.get<Ecs::Position>(client.playerEntity);
        client.chunkView.update(position.value);
    }

    m_world.update();
    m_world.generator().update([this](Chunk& generatedChunk) {
        auto it = m_requestedChunks.find(generatedChunk.position);
        if (it == m_requestedChunks.end()) {
            return;
        }

        // TODO: Check if holder is Ready

        auto clientsToNotify = std::move(it->second);
        m_requestedChunks.erase(it);

        for (const auto& clientId : clientsToNotify) {
            if (auto* client = findClient(clientId)) {
                client->chunkView.notifyChunkDone(generatedChunk.position);
            }
        }
    });

    for (auto& client : m_clients) {
        sendChunkUpdate(*client.connection, m_world, client.chunkView);
    }
}

void Server::run() {
    m_running.store(true);

    while (isRunning()) {
        update();
    }
}

void Server::stop() {
    m_running.store(false);
    m_netServer.stop();
}

}
