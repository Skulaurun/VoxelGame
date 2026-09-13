#pragma once

#include "Message.hpp"
#include "Common/TsQueue.hpp"

#include <asio.hpp>

// TODO:
// the idea is something like std::variant<PhysicalConnection, LoopbackConnection>

namespace Common::Network {

class Connection : public std::enable_shared_from_this<Connection> {

public:
    Connection(TsQueue<Event>& queue, asio::io_context& context, asio::ip::tcp::socket socket);
    virtual ~Connection();

    void readHeader();
    void readBody(const uint32_t id, const uint32_t length);

    void writeHeaderAndBody();

    void send(const Message& message);
    void connect(const asio::ip::tcp::resolver::results_type& endpoints);
    void close();

    bool isConnected() const { return socket.is_open(); }

protected:
    void pushIncomingMessage(const std::shared_ptr<Message> message);
    void pushInternalMessage(const InternalMessage::Type type, const std::error_code error = std::error_code());

    std::shared_ptr<Connection> sharedFromThis();

private:
    uint8_t headerIn[2 * sizeof(uint32_t)];
    std::vector<uint8_t> bodyIn;

    TsQueue<Event>& queueIn;
    TsQueue<std::shared_ptr<std::vector<uint8_t>>> queueOut;

    asio::io_context& context;
    asio::ip::tcp::socket socket;

};

}
