#include "NetCode.hpp"
#include "Client/World/Chunk.hpp"

#include <print>

void ClientHandler::operator()(const Common::Network::Connect& message) {
    std::println("ClientHandler: Connected to server");
}

void ClientHandler::operator()(const Common::Network::Disconnect& message) {
    std::println("ClientHandler: Disconnected from server");
}

void ClientHandler::onChunkUpdate(std::shared_ptr<Common::Network::Connection> sender, Common::Network::ProtocolStream& stream) {
    uint32_t chunkCount{};
    stream.read(chunkCount);

    for (uint32_t i = 0; i < chunkCount; i++) {
        auto chunk = std::make_shared<Chunk>();
        stream.read(chunk->position);
        stream.read(chunk->grid);
    }

    std::println("{}", chunkCount);
}

void ClientHandler::operator()(Common::Network::IncomingMessage& message) {
    if (message.id > Common::Network::MessageId::_Unknown_ && message.id < Common::Network::MessageId::_Count_) {
        (this->*MESSAGE_HANDLER_MAP[handlerIndex(message.id)])(message.sender, message.stream);
    }
}
