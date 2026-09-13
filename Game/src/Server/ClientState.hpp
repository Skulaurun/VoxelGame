#pragma once

#include "World/ChunkLoader.hpp"

#include <entt/entt.hpp>

struct ClientState {
    entt::entity playerEntity;
    ChunkLoader chunkLoader;

    // connection
};
