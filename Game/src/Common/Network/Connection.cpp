#include "Connection.hpp"

namespace Common::Network {

Connection::Connection(TsQueue<Event>& queue) :
    m_queueIn{queue}
{}

SocketConnection::SocketConnection(TsQueue<Event>& queue, asio::io_context& context, asio::ip::tcp::socket socket) :
    Connection{queue},
    m_context{context},
    m_socket{std::move(socket)}
{}

SocketConnection::~SocketConnection() {
    // If there is an outstanding read/write, it should be cancelled with asio::error::operation_aborted,
    // which will result in decreased refcount

    m_socket.shutdown(asio::socket_base::shutdown_both);
    m_socket.close();

    // FIXME: doesn't pushBack(std::in_place_type<Disconnect>...............
}

void SocketConnection::connect(const asio::ip::tcp::resolver::results_type& endpoints) {
    asio::async_connect(m_socket, endpoints,
        [this, self = sharedFromThis()](std::error_code error, asio::ip::tcp::endpoint endpoint) {
            if (!error) {
                readHeader();
            }
            m_queueIn.pushBack(std::in_place_type<Connect>, std::move(self), error);
        }
    );
}

void SocketConnection::close() {
    if (isConnected()) {
        auto self = sharedFromThis();

        m_socket.shutdown(asio::socket_base::shutdown_both);
        m_socket.close();

        m_queueIn.pushBack(std::in_place_type<Disconnect>, std::move(self), std::error_code{});
    }
}

void SocketConnection::readHeader() {
    asio::async_read(m_socket, asio::buffer(m_headerIn, sizeof(m_headerIn)),
        [this, self = sharedFromThis()](std::error_code error, std::size_t length) mutable {
            if (!error) {

                // TODO: How to handle ser/deser, need to swap bytes because of endianness...

                // MessageHeader header = {};
                // const auto* it = headerIn;
                // header.read(it, sizeof(headerIn));

                // auto messageId = std::get<0>(header.getFields()).value();
                // auto bodyLength = std::get<1>(header.getFields()).value();

                // if (bodyLength > 0) {
                //     readBody(messageId, bodyLength);
                // } else {
                //     //pushIncomingMessage(Message::create(messageId));
                //     readHeader();
                // }
            } else {
                handleError(std::move(self), error);
            }
        }
    );
}

void SocketConnection::readBody(uint32_t messageId, uint32_t length) {
    // Check if body length isnt too large
    m_bodyIn.resize(length);

    asio::async_read(m_socket, asio::buffer(m_bodyIn.data(), m_bodyIn.size()),
        [this, self = sharedFromThis(), messageId](std::error_code error, std::size_t length) mutable {
            if (!error) {

                // TODO: either make_shared<ProtocolStream>, capture in lambda by value, push ptr to queue,
                // or the way it is and move the ProtocolStream

                // std::shared_ptr<Message> message = Message::create(messageId);
                // const auto* it = bodyIn.data();
                // message->read(it, bodyIn.size());

                //pushIncomingMessage(message);
                readHeader();
            } else {
                handleError(std::move(self), error);
            }
        }
    );
}

void SocketConnection::writeHeaderAndBody() {
    const auto& buffer = *m_queueOut.front();
    asio::async_write(m_socket, asio::buffer(buffer.data(), buffer.size()),
        [this, self = sharedFromThis()](std::error_code error, std::size_t length) mutable {
            if (!error) {
                m_queueOut.popFront();
                if (!m_queueOut.empty()) {
                    writeHeaderAndBody();
                }
            } else {
                handleError(std::move(self), error);
            }
        }
    );
}

void SocketConnection::send(const Message& message) {
    if (!isConnected()) {
        return;
    }

    // MessageHeader header{};
    // header.id = ;
    // header.length = ;

    // auto bufferPtr = std::make_shared<std::vector<uint8_t>>();
    // auto& buffer = *bufferPtr;
    // buffer.resize(header.length() + message.length());

    // uint8_t* it = buffer.data();
    // header.write(it, buffer.size()).and_then([&](auto bytesWritten) {
    //     return message.write(it, buffer.size() - bytesWritten);
    // });

    // asio::post(context, [this, bufferPtr]() {
    //     bool writing = !m_queueOut.empty();
    //     m_queueOut.pushBack(bufferPtr);
    //     if (!writing) {
    //         writeHeaderAndBody();
    //     }
    // });
}

void SocketConnection::handleError(std::shared_ptr<SocketConnection>&& self, std::error_code error) {
    if (error == asio::error::operation_aborted) {
        // We caused this by either socket.cancel() or socket.close()
        return;
    }

    // Handle as a disconnect
    m_socket.close();
    m_queueIn.pushBack(
        std::in_place_type<Disconnect>,
        std::move(self),
        error
    );
}

InternalConnection::InternalConnection(TsQueue<Event>& queue)
    : Connection{queue}, m_connected{false}
{}

InternalConnection::~InternalConnection() {
    // Prevent endless execution -> m_connection's close() will check this
    m_connected = false;

    if (auto connection = m_connection.lock()) {
        connection->close();
    }

    // FIXME: doesn't pushBack(std::in_place_type<Disconnect>...............
}

void InternalConnection::connect(const std::shared_ptr<InternalConnection>& connection) {
    if (!isConnected()) {
        m_connected = true;
        m_connection = connection;

        connection->connect(sharedFromThis());
        m_queueIn.pushBack(std::in_place_type<Connect>, sharedFromThis(), std::error_code{});
    }
}

void InternalConnection::send(const Message& message) {
    if (auto connection = m_connection.lock()) {
        connection->m_queueIn.pushBack(IncomingMessage{
            message.id,
            ProtocolStream{ message.stream.data() },
            sharedFromThis()
        });
    }
}

void InternalConnection::close() {
    if (isConnected()) {
        m_connected = false;
        if (auto connection = m_connection.lock()) {
            connection->close();
        }

        m_queueIn.pushBack(std::in_place_type<Disconnect>, sharedFromThis(), std::error_code{});
    }
}

}
