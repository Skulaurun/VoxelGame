#pragma once

#include "ClientState.hpp"
#include "World/World.hpp"

#include "Common/Network/Server.hpp"
#include "Common/Network/Connection.hpp"

#include <entt/entt.hpp>

#include <vector>

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

    // called from Handler
    void addClient(std::shared_ptr<Common::Network::Connection> connection);
    void removeClient();

    void run();
    void update();
    void stop();

    auto netServer() -> auto& { return m_netServer; }

private:
    World m_world;
    std::atomic<bool> m_running;

    // probably part of World
    entt::registry m_registry;

    std::shared_ptr<ServerHandler> m_messageHandler;
    Common::Network::Server<ServerHandler> m_netServer;

    std::vector<ClientState> m_clients;

};
