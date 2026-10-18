#pragma once

#include "Common/Network/ProtocolStream.hpp"
#include "Common/Network/Message.hpp"

#include <array>

namespace Client {

class Client;

class ClientHandler {

public:
    ClientHandler(Client& client);

    void operator()(const Common::Network::Connect& message);
    void operator()(const Common::Network::Disconnect& message);
    void operator()(Common::Network::IncomingMessage& message);

private:
    Client& m_client;

    void onChunkUpdate(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream);

    constexpr static auto MESSAGE_CALLBACK_MAP = []() {
        Common::Network::MessageCallbackMap<ClientHandler> idToHandler{};

        idToHandler[Common::Network::MessageId::ChunkUpdate] = &ClientHandler::onChunkUpdate;

        return idToHandler;
    }();

};

}
