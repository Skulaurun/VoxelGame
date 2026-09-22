#pragma once

#include "Common/Network/Protocol.hpp"
#include "Common/Network/ProtocolStream.hpp"

#include <variant>
#include <memory>
#include <system_error>

namespace Common::Network {

class Connection;

struct Message {
    MessageId id;
    ProtocolStream stream;
};

struct IncomingMessage {
    MessageId id;
    ProtocolStream stream;

    std::shared_ptr<Connection> sender;
};

struct Connect {
    std::shared_ptr<Connection> connection;
    std::error_code error;
};

struct Disconnect {
    std::shared_ptr<Connection> connection;
    std::error_code error;
};

using Event = std::variant<Connect, Disconnect, IncomingMessage>;

}
