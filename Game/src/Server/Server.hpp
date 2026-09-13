#pragma once

#include "ClientState.hpp"
#include "World/World.hpp"

#include <entt/entt.hpp>

#include <vector>

class Server {

public:
    Server();
    ~Server();

    Server(Server&&) = delete;
    Server(const Server&) = delete;

    Server& operator=(Server&&) = delete;
    Server& operator=(const Server&) = delete;

    bool isRunning() const { return m_running.load(); }

    void addClient(/* connection */);
    void removeClient();

    void run();
    void update();
    void stop();

private:
    World m_world;
    std::atomic<bool> m_running;

    // probably part of World
    entt::registry m_registry;

    std::vector<ClientState> m_clients;

};
