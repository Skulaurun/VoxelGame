#include "Server.hpp"

#include "NetCode.hpp"
#include "ECS/Component.hpp"

namespace Server {

constexpr int32_t VIEW_RANGE{ 8 };

Server::Server() :
    m_running{false},
    m_messageHandler{std::make_shared<ServerHandler>(*this)},
    m_netServer{m_messageHandler, 5500} // TODO: port
{
    // Server init

    m_world.setViewRange(VIEW_RANGE);
    m_world.generator().enqueue([&](WorldGenerator::ChunkQueue& queue) {
        for (int32_t y = -VIEW_RANGE; y < VIEW_RANGE; y++) {
            for (int32_t x = -VIEW_RANGE; x < VIEW_RANGE; x++) {
                for (int32_t z = -VIEW_RANGE; z < VIEW_RANGE; z++) {
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

    auto& client = m_clients.emplace_back(playerEntity, ChunkLoader{m_world}, connection);
    
    // TODO:
    const auto side = 2 * VIEW_RANGE + 1;
    client.chunkLoader.resize(side * side * side);
}

auto Server::findClient(std::shared_ptr<Common::Network::Connection> connection) -> ClientState* {
    auto it = std::ranges::find(m_clients, connection, &ClientState::connection);
    return (it != m_clients.end()) ? &*it : nullptr;
}

void Server::update() {
    m_world.generator().update();

    for (auto& client : m_clients) {
        const auto& position = m_registry.get<Ecs::Position>(client.playerEntity);
        client.chunkLoader.update(position.value);
        
        sendChunkUpdate(*client.connection, client.chunkLoader);
    }

    m_netServer.update();
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
