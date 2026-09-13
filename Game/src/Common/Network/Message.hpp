#pragma once

#include <variant>

namespace Common::Network {

struct Message {
    // TODO: data stream
};

struct Connect {

};

struct Disconnect {

};

using Event = std::variant<Connect, Disconnect, Message>;

}
