#pragma once

#include "Common/Network/ProtocolStream.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Common.hpp"

#include <deque>

namespace Client {

class Client;

struct EntitySnapshot {
    float time;
    glm::vec3 position;
    glm::quat rotation;
};

class ClientHandler {

public:
    ClientHandler(Client& client);

    void operator()(const Network::Connect& message);
    void operator()(const Network::Disconnect& message);
    void operator()(Network::IncomingMessage& message);

    auto snapshots() -> auto& { return m_entitySnaphots; }

private:
    void onChunkUpdate(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);
    void onEntityUpdate(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);
    void onSetViewDistance(std::shared_ptr<Network::Connection> sender, Network::ProtocolStream& stream);

    constexpr static auto MESSAGE_CALLBACK_MAP = []() {
        using namespace Network;

        MessageCallbackMap<ClientHandler> idToHandler{};

        idToHandler[MessageId::ChunkUpdate] = &ClientHandler::onChunkUpdate;
        idToHandler[MessageId::EntityUpdate] = &ClientHandler::onEntityUpdate;
        idToHandler[MessageId::SetViewDistance] = &ClientHandler::onSetViewDistance;

        return idToHandler;
    }();

private:
    Client& m_client;

    constexpr static auto MAX_ENTITY_SNAPSHOT_COUNT{ 8 };
    std::unordered_map<NetworkId, std::deque<EntitySnapshot>> m_entitySnaphots;

};

void sendInputCommand(Network::Connection& connection, const glm::vec2& eulerAngles, const glm::vec3& direction, bool isJump);

void sendViewDistanceRequest(Network::Connection& connection, int32_t value);

}
