#pragma once

#include <cstdint>

namespace Network {

enum class MessageId : uint16_t {
    _Unknown_ = 0,

    // Client <- Server
    Pong,
    SetViewDistance,
    ChunkUpdate,
    EntityUpdate,

    // Client -> Server
    Ping,
    RequestViewDistance,
    InputCommand,

    // Client <-> Server

    _Count_
};

}
