#pragma once

#include "Common/Network/ProtocolStream.hpp"
#include "Common/Network/Message.hpp"

#include <array>

class ClientHandler {

public:
    using MessageHandler = void(ClientHandler::*)(std::shared_ptr<Common::Network::Connection>, Common::Network::ProtocolStream&);

public:
    void operator()(const Common::Network::Connect& message);
    void operator()(const Common::Network::Disconnect& message);
    void operator()(Common::Network::IncomingMessage& message);

    constexpr static std::array<MessageHandler, UINT16_MAX> initHandlers() {
        std::array<MessageHandler, UINT16_MAX> idToHandler{};

        idToHandler[handlerIndex(Common::Network::MessageId::ChunkUpdate)] = &ClientHandler::onChunkUpdate;

        return idToHandler;
    }

private:
    constexpr static auto handlerIndex(Common::Network::MessageId id) -> std::size_t {
        return static_cast<std::size_t>(id);
    };

    void onChunkUpdate(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream);

};

constexpr static auto MESSAGE_HANDLER_MAP = ClientHandler::initHandlers();
