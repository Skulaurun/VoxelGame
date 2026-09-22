#pragma once

#include <cstdint>

namespace Common::Network {

enum class MessageId : uint16_t {
    _Unknown_ = 0,

    // Client <- Server
    ChunkUpdate,

    // Client -> Server

    // Client <-> Server

    _Count_
};

}
