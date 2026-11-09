#pragma once

#include "Server/World/ClientChunkView.hpp"
#include "Common/Common.hpp"
#include "Common/Network/Connection.hpp"

#include <entt/entt.hpp>

namespace Server {

struct ClientState {
    ClientId id;
    entt::entity playerEntity;
    ClientChunkView chunkView;

    std::shared_ptr<Common::Network::Connection> connection;
};

}
