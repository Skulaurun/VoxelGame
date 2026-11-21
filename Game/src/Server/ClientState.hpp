#pragma once

#include "Server/World/ClientChunkView.hpp"
#include "Common/Common.hpp"
#include "Common/Network/Connection.hpp"

#include <entt/entt.hpp>

namespace Server {

struct InputCommand {
    glm::vec2 eulerAngles;
    glm::vec3 movement; // [right, up, forward]
    bool isJump;
};

struct ClientState {
    ClientId id;
    entt::entity playerEntity;
    ClientChunkView chunkView;
    std::deque<InputCommand> inputCommands;

    std::shared_ptr<Network::Connection> connection;
};

}
