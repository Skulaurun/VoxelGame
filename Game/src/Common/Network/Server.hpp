#pragma once

#include "Message.hpp"
#include "Connection.hpp"
#include "Context.hpp"

namespace Common::Network {

template<typename Handler>
class Server {

public:
    Server(std::shared_ptr<Handler> handler, uint16_t port) :
        m_context{std::move(handler)},
        m_acceptor{m_context.context(), asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port}}
    {}

    void start() {
        waitForClient();
        m_context.start();
    }

    void stop() {
        m_context.stop();
    }

    // TODO:
    void update() {
        m_context.pollEvents(false);
    }

    void connect(const std::shared_ptr<InternalConnection>& incomingConnection) {
        auto newConnection = std::make_shared<InternalConnection>(m_context.queueIn());
        newConnection->connect(incomingConnection);
    }

    // void broadcast(const Message& message, Connection* exclude = nullptr) {
    //     for (const auto& connection : m_connections) {
    //         if (connection.get() == exclude) {
    //             continue;
    //         }
    //         if (connection->isConnected()) {
    //             connection->send(message);
    //         }
    //     }
    // }

private:
    void waitForClient() {
        m_acceptor.async_accept([this](std::error_code error, asio::ip::tcp::socket socket) {
            if (!error) {
                std::shared_ptr<SocketConnection> newConnection = std::make_shared<SocketConnection>(
                    m_context.queueIn(), m_context.context(),
                    std::move(socket)
                );
                if (canAcceptConnection(*newConnection)) {
                    newConnection->readHeader();
                    m_context.queueIn().pushBack(std::in_place_type<Connect>, newConnection, std::error_code{});
                } else {
                    // TODO: log message
                }
            }
            waitForClient();
        });
    }

    // FIXME:
    // This function is called from Network::Context's thread
    bool canAcceptConnection(const SocketConnection& connection) { return true; }

private:
    Context<Handler> m_context;
    asio::ip::tcp::acceptor m_acceptor;

};
    
}
