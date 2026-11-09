#pragma once

#include "Server/World/ClientChunkView.hpp"
#include "Common/Network/Connection.hpp"

namespace Server {

class Server;
class World;

// TODO: ServerHandler and ClientHandler are really similar,
// the message handler dispatching should probably be at higher abstraction layer

class ServerHandler {

public:
    ServerHandler(Server& server);

    void operator()(const Common::Network::Connect& event);
    void operator()(const Common::Network::Disconnect& event);
    void operator()(Common::Network::IncomingMessage& message);

private:
    Server& m_server;

    void onPlayerMove(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream);
    void onRequestViewDistance(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream);

    constexpr static auto MESSAGE_CALLBACK_MAP = []() {
        using namespace Common::Network;

        MessageCallbackMap<ServerHandler> idToHandler{};

        idToHandler[MessageId::PlayerMove] = &ServerHandler::onPlayerMove;
        idToHandler[MessageId::RequestViewDistance] = &ServerHandler::onRequestViewDistance;

        return idToHandler;
    }();

};

void sendChunkUpdate(Common::Network::Connection& connection, World& world, ClientChunkView& chunkView);

}
