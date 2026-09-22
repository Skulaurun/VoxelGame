#pragma once

#include "Server/World/ChunkLoader.hpp"
#include "Common/Network/Connection.hpp"

class Server;

// TODO: ServerHandler and ClientHandler are really similar,
// the message handler dispatching should probably be at higher abstraction layer

class ServerHandler {

public:
    ServerHandler(Server& server);

    void operator()(const Common::Network::Connect& event);
    void operator()(const Common::Network::Disconnect& event);
    void operator()(const Common::Network::IncomingMessage& event);

private:
    Server& m_server;

};

void sendChunkUpdate(Common::Network::Connection& connection, ChunkLoader& chunkLoader);
