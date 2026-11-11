#pragma once

#include "Common/Network/ProtocolStream.hpp"
#include "Common/Network/Message.hpp"

#include <array>

namespace Client {

class Client;

class ClientHandler {

public:
    ClientHandler(Client& client);

    void operator()(const Network::Connect& message);
    void operator()(const Network::Disconnect& message);
    void operator()(Network::IncomingMessage& message);

private:
    Client& m_client;

    void onChunkUpdate(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);
    void onSetViewDistance(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);

    constexpr static auto MESSAGE_CALLBACK_MAP = []() {
        using namespace Network;

        MessageCallbackMap<ClientHandler> idToHandler{};

        idToHandler[MessageId::ChunkUpdate] = &ClientHandler::onChunkUpdate;
        idToHandler[MessageId::SetViewDistance] = &ClientHandler::onSetViewDistance;

        return idToHandler;
    }();

};

}
