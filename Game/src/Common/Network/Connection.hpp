#pragma once

#include "Message.hpp"
#include "Common/TsQueue.hpp"

#include "Asio.hpp"

namespace Common::Network {

class Connection : public std::enable_shared_from_this<Connection> {

public:
    Connection(TsQueue<Event>& queue);
    virtual ~Connection() = default;

    virtual void send(const Message& message) = 0;
    virtual void close() = 0;

protected:
    TsQueue<Event>& m_queueIn;

};

class SocketConnection : public Connection {

private:
    struct MessageHeader {
        uint32_t id;
        uint32_t bodyLength;
    };

    using Buffer = std::vector<uint8_t>;

public:
    SocketConnection(TsQueue<Event>& queue, asio::io_context& context, asio::ip::tcp::socket socket);
    virtual ~SocketConnection();

    void connect(const asio::ip::tcp::resolver::results_type& endpoints);

    void send(const Message& message) override;
    void close() override;

    void readHeader();
    void readBody(uint32_t messageId, uint32_t length);
    void writeHeaderAndBody();

    auto sharedFromThis() { return std::static_pointer_cast<SocketConnection>(shared_from_this()); }

    bool isConnected() const { return m_socket.is_open(); }

protected:
    void handleError(std::shared_ptr<SocketConnection>&& self, std::error_code error);

private:
    uint8_t m_headerIn[2 * sizeof(uint32_t)];
    Buffer m_bodyIn;

    TsQueue<Buffer> m_queueOut;

    asio::io_context& m_context;
    asio::ip::tcp::socket m_socket;

};

class InternalConnection : public Connection {

public:
    InternalConnection(TsQueue<Event>& queue);
    virtual ~InternalConnection();

    void connect(const std::shared_ptr<InternalConnection>& connection);

    void send(const Message& message) override;
    void close() override;

    auto sharedFromThis() { return std::static_pointer_cast<InternalConnection>(shared_from_this()); }

    bool isConnected() const { return m_connected; }

private:
    bool m_connected;

    std::weak_ptr<InternalConnection> m_connection;

};

}
