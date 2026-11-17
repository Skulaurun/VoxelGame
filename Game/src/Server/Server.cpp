#include "Server.hpp"

#include "NetCode.hpp"
#include "Common/ECS/System.hpp"
#include "Common/ECS/Component.hpp"

#include <Engine/Core/Core.hpp>
#include <Engine/Core/Log.hpp>

namespace Server {

constexpr int32_t PREGENERATED_RANGE{ 8 };

Server::Server() :
    m_running{false},
    m_tick{0},
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

auto spawnPlayer(entt::registry& registry, World& world) -> entt::entity {
    auto playerEntity = registry.create();
    registry.emplace_or_replace<Ecs::Position>(playerEntity, glm::vec3{0.0f, 100.0f, 0.0f});
    registry.emplace_or_replace<Ecs::Velocity>(playerEntity, glm::vec3{});
    registry.emplace_or_replace<Ecs::Rotation>(playerEntity, glm::identity<glm::quat>());

    return playerEntity;
}

void Server::addClient(std::shared_ptr<Network::Connection> connection) {
    const auto playerEntity = spawnPlayer(m_registry, m_world);
    const auto clientId = m_lastClientId++;
    const auto index = m_clients.size();
    auto& client = m_clients.emplace_back(clientId, playerEntity, ClientChunkView{*this, clientId}, connection);

    const auto networkId = m_lastNetworkId++;
    m_entityToNetworkId.emplace(playerEntity, networkId);
    m_networkIdToEntity.emplace(networkId, playerEntity);

    m_idToClient.emplace(clientId, index);
    m_connectionToClient.emplace(connection.get(), index);
}

void Server::removeClient(const std::shared_ptr<Network::Connection>& connection) {
    if (auto clientPtr = findClient(connection)) {
        if (const auto it = m_entityToNetworkId.find(clientPtr->playerEntity); it != m_entityToNetworkId.end()) {
            m_networkIdToEntity.erase(it->second);
            m_entityToNetworkId.erase(it);
        }

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

auto Server::findClient(const std::shared_ptr<Network::Connection>& connection) -> ClientState* {
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

void Server::beginUpdate() {
    m_netServer.update();
}

void Server::update(float dt) {
    // FIXME: server lags and client jumps forward a lot
    for (const auto& command : m_messageHandler->inputCommands()) {
        // TODO: unordered_map<entity, queue> would probably be more efficient
        if (auto* position = m_registry.try_get<Ecs::Position>(command.entity)) {
            constexpr auto MOVEMENT_SPEED{60.0f};

            const glm::vec2 forward{
                glm::cos(glm::radians(command.eulerAngles.y)),
                glm::sin(glm::radians(command.eulerAngles.y))
            };
            const glm::vec2 right{-forward.y, forward.x};

            const auto speed = MOVEMENT_SPEED * dt;
            const auto result = forward * command.movement.y + right * command.movement.x;
            
            position->value.x += speed * result.x;
            position->value.z += speed * result.y;
        }
    }

    Ecs::simulate(m_registry, dt);

    for (auto& client : m_clients) {
        sendEntityUpdate(*client.connection, client.playerEntity, m_registry, m_entityToNetworkId, m_tick);
    }

    m_tick++;
}

void Server::endUpdate() {
    m_messageHandler->inputCommands().clear();

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
    using Clock = std::chrono::steady_clock;

    // TODO: what value to choose?
    constexpr auto MAX_SIMULATION_TIME_PER_FRAME{ 0.5f };

    float accumulator{0.0f};
    auto lastFrame{Clock::now()};

    m_running.store(true);
    while (isRunning()) {
        const auto currentFrame = Clock::now();
        const auto dt = std::chrono::duration<float>{
            currentFrame - lastFrame
        }.count();
        accumulator += dt;
        lastFrame = currentFrame;
        // TODO: figure how to manage time drift and if this is the correct approach
        accumulator = glm::min(accumulator, MAX_SIMULATION_TIME_PER_FRAME);

        if (accumulator >= SIMULATION_TICK) {
            beginUpdate();
            while (accumulator >= SIMULATION_TICK) {
                update(SIMULATION_TICK);
                accumulator -= SIMULATION_TICK;
            }
            endUpdate();
        }

        const auto blockTime = std::chrono::duration<float>{
            SIMULATION_TICK - accumulator
        };
        const auto waitUntil = Clock::now() + blockTime;
        while (Clock::now() < waitUntil) {
            // wait
        }
    }
}

void Server::stop() {
    m_running.store(false);
    m_netServer.stop();
}

}
