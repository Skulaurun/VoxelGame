#include <Common/Client.hpp>

namespace common {

    void Client::connect(const std::string& address, const uint16_t port) {
        asio::ip::tcp::resolver resolver(context);
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address, std::to_string(port));

        connection = std::make_unique<Connection>(
            Connection::Owner::Client,
            queueIn, context,
            asio::ip::tcp::socket(context)
        );

        connection->connect(endpoints);
    }

    void Client::disconnect() {
        if (connection) {
            connection->close();
            connection.reset();
            onClientDisconnect();
        }
    }

    void Client::send(const Message& message) {
        if (connection && connection->isConnected()) {
            connection->send(message);
        }
    }

    void Client::onMessage(InternalMessage& message) {
        if (!message.isError()) {
            if (connection->isConnected()) {
                onClientConnect();
            }
        } else {
            if (!connection->isConnected()) {
                onClientDisconnect();
            }
        }
    }

}
