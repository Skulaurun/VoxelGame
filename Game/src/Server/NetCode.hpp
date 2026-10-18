#pragma once

#include "Server/World/ChunkLoader.hpp"
#include "Common/Network/Connection.hpp"

namespace Server {

class Server;

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

    constexpr static auto MESSAGE_CALLBACK_MAP = []() {
        Common::Network::MessageCallbackMap<ServerHandler> idToHandler{};

        idToHandler[Common::Network::MessageId::PlayerMove] = &ServerHandler::onPlayerMove;

        return idToHandler;
    }();

};

void sendChunkUpdate(Common::Network::Connection& connection, ChunkLoader& chunkLoader);

}
