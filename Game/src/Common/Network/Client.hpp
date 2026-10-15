#pragma once

#include "Context.hpp"
#include "Connection.hpp"

#include <string>
#include <cstdint>

namespace Common::Network {

template<typename Handler>
class Client {

public:
    Client(std::shared_ptr<Handler> handler) :
        m_context{std::move(handler)}
    {}

    void start() {
        m_context.start();
    }

    void stop() {
        m_context.stop();
    }

    void update() {
        m_context.pollEvents(false);
    }

    void connect(std::string_view address, uint16_t port) {
        asio::ip::tcp::resolver resolver{m_context.context()};
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address, std::to_string(port));

        auto connection = std::make_shared<SocketConnection>(
            m_context.queueIn(), m_context.context(),
            asio::ip::tcp::socket{m_context.context()}
        );
        connection->connect(endpoints);
        m_connection = connection;
    }

    auto createConnection() -> std::shared_ptr<InternalConnection> {
        auto connection = std::make_shared<InternalConnection>(m_context.queueIn());
        m_connection = connection;
        return connection;
    }

    void send(const Message& message) {
        if (!m_connection) {
            return;
        }
        m_connection->send(message);
    }

    void disconnect() {
        if (!m_connection) {
            return;
        }
        m_connection->close();
        m_connection.reset();
    }

private:
    Context<Handler> m_context;
    std::shared_ptr<Connection> m_connection;

};

}
