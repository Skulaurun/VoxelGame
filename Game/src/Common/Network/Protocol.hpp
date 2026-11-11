#pragma once

#include <cstdint>

namespace Network {

enum class MessageId : uint16_t {
    _Unknown_ = 0,

    // Client <- Server
    SetViewDistance,
    ChunkUpdate,

    // Client -> Server
    RequestViewDistance,
    PlayerMove,

    // Client <-> Server

    _Count_
};

}
