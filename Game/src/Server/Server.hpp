#pragma once

#include "ClientState.hpp"
#include "World/World.hpp"

#include "Common/Common.hpp"
#include "Common/Network/Server.hpp"
#include "Common/Network/Connection.hpp"

#include <entt/entt.hpp>

#include <vector>

namespace Server {

class ServerHandler;

class Server {

public:
    Server();
    ~Server();

    Server(Server&&) = delete;
    Server(const Server&) = delete;

    Server& operator=(Server&&) = delete;
    Server& operator=(const Server&) = delete;

    bool isRunning() const { return m_running.load(); }

    void addClient(std::shared_ptr<Common::Network::Connection> connection);
    void removeClient(const std::shared_ptr<Common::Network::Connection>& connection);

    auto findClient(ClientId id) -> ClientState*;
    auto findClient(const std::shared_ptr<Common::Network::Connection>& connection) -> ClientState*;

    auto requestClientChunk(ClientId id, const glm::ivec3& position) -> bool;

    void run();
    void update();
    void stop();

    auto registry() -> auto& { return m_registry; }
    auto world() -> auto& { return m_world; }

    auto netServer() -> auto& { return m_netServer; }

private:
    World m_world;
    std::atomic<bool> m_running;

    // probably part of World
    entt::registry m_registry;

    std::shared_ptr<ServerHandler> m_messageHandler;
    Common::Network::Server<ServerHandler> m_netServer;

    std::vector<ClientState> m_clients;

    std::unordered_map<
        Common::Network::Connection*,
        std::size_t
    > m_connectionToClient;

    ClientId m_lastClientId;
    std::unordered_map<ClientId, std::size_t> m_idToClient;

    std::unordered_map<glm::ivec3, std::vector<ClientId>> m_requestedChunks;

};

}
