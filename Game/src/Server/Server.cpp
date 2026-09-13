#include "Server.hpp"

#include "ECS/Component.hpp"

Server::Server() {
    // Server init

    constexpr int32_t VIEW_RANGE{ 8 };

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
}

Server::~Server() {
    stop();
}

void Server::addClient() {
    entt::entity playerEntity = m_registry.create();
    m_registry.emplace_or_replace<Ecs::Position>(playerEntity);
}

void Server::update() {
    m_world.generator().update();

    for (auto& client : m_clients) {
        const auto& position = m_registry.get<Ecs::Position>(client.playerEntity);
        client.chunkLoader.update(position.value);

        for (const auto& chunkEvent : client.chunkLoader.chunkEvents()) {
            if (chunkEvent.type == ChunkLoader::Event::Type::Load) {
                Chunk& chunk = *client.chunkLoader.loadedChunks()[chunkEvent.id].holder->chunk;
            } else if (chunkEvent.type == ChunkLoader::Event::Type::Unload) {
                // TODO: use chunk pos as id? add to event?
            }
        }

        // TODO: send chunk loads / unloads to client
    }
}

void Server::run() {
    while (isRunning()) {
        update();
    }
}

void Server::stop() {
    m_running.store(false);
}
