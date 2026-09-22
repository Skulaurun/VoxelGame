#pragma once

#include "Server/World/ChunkLoader.hpp"
#include "Common/Network/Connection.hpp"

#include <entt/entt.hpp>

struct ClientState {
    entt::entity playerEntity;
    ChunkLoader chunkLoader;

    std::shared_ptr<Common::Network::Connection> connection;
};
