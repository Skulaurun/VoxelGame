#include <Common/Server.hpp>

namespace common {

    Server::Server(uint16_t port)
        : acceptor(context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

    void Server::waitForClient() {
        acceptor.async_accept([this](std::error_code error, asio::ip::tcp::socket socket) {
            if (!error) {
                std::shared_ptr<Connection> newConnection = std::make_shared<Connection>(
                    Connection::Owner::Server,
                    queueIn, context,
                    std::move(socket)
                );
                if (onClientConnect(newConnection)) {
                    newConnection->readHeader();
                    connections.push_back(std::move(newConnection));
                }
            }
            waitForClient();
        });
    }

    void Server::start() {
        waitForClient();
        NetworkContext::start();
    }

    void Server::broadcast(const Message& message, const std::shared_ptr<Connection> exclude) {
        for (const auto& connection : connections) {
            if (connection != exclude) {
                if (connection->isConnected()) {
                    connection->send(message);
                }
            }
        }
    }

    void Server::onMessage(InternalMessage& message) {
        if (!message.sender->isConnected()) {
            onClientDisconnect(message.sender);
        }
    }

}
