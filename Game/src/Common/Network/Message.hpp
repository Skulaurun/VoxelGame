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

template<typename Handler>
using MessageCallback = void(Handler::*)(std::shared_ptr<Connection>, ProtocolStream&);

template<typename Handler>
class MessageCallbackMap {

public:
    void operator()(Handler& handler, IncomingMessage& message) const {
        if (message.id > MessageId::_Unknown_ && message.id < MessageId::_Count_) {
            (handler.*data[handlerIndex(message.id)])(message.sender, message.stream);
        }
    }

    constexpr auto operator[](MessageId id) -> auto& {
        return data[handlerIndex(id)];
    }

    constexpr auto operator[](MessageId id) const -> const auto& {
        return data[handlerIndex(id)];
    }

    constexpr static auto handlerIndex(MessageId id) -> std::size_t {
        return static_cast<std::size_t>(id);
    };

private:
    std::array<MessageCallback<Handler>, 1u << 16> data;
};

}
