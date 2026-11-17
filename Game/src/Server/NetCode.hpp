#pragma once

#include "Server/World/ClientChunkView.hpp"
#include "Common/Network/Connection.hpp"

#include <entt/entt.hpp>

namespace Server {

class Server;
class World;

struct InputCommand {
    entt::entity entity;
    glm::vec2 eulerAngles;
    glm::vec2 movement; // [right, forward]
    bool isJump;
};

// TODO: ServerHandler and ClientHandler are really similar,
// the message handler dispatching should probably be at higher abstraction layer

class ServerHandler {

public:
    ServerHandler(Server& server);

    void operator()(const Network::Connect& event);
    void operator()(const Network::Disconnect& event);
    void operator()(Network::IncomingMessage& message);

    auto inputCommands() -> auto& { return m_inputCommands; }

private:
    Server& m_server;

    void onInputCommand(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);
    void onRequestViewDistance(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);

    constexpr static auto MESSAGE_CALLBACK_MAP = []() {
        using namespace Network;

        MessageCallbackMap<ServerHandler> idToHandler{};

        idToHandler[MessageId::InputCommand] = &ServerHandler::onInputCommand;
        idToHandler[MessageId::RequestViewDistance] = &ServerHandler::onRequestViewDistance;

        return idToHandler;
    }();

    std::deque<InputCommand> m_inputCommands;

};

void sendChunkUpdate(Network::Connection& connection, World& world, ClientChunkView& chunkView);

void sendEntityUpdate(
    Network::Connection& connection,
    entt::entity playerEntity,
    const entt::registry& registry,
    const std::unordered_map<entt::entity, NetworkId>& entityToNetworkId,
    uint64_t tick
);

}
