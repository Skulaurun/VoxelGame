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
    m_socket.shutdown(asio::socket_base::shutdown_both);
    m_socket.close();
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
                MessageHeader header{};
                Common::ReadIterator it = m_headerIn;
                readNumber<NETWORK_BYTE_ORDER>(header.id, it);
                readNumber<NETWORK_BYTE_ORDER>(header.bodyLength, it);

                if (header.bodyLength > 0) {
                    readBody(header.id, header.bodyLength);
                } else {
                    m_queueIn.pushBack(
                        std::in_place_type<IncomingMessage>,
                        static_cast<MessageId>(header.id),
                        ProtocolStream{},
                        std::move(self)
                    );
                    readHeader();
                }
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
                m_queueIn.pushBack(
                    std::in_place_type<IncomingMessage>,
                    static_cast<MessageId>(messageId),
                    ProtocolStream{ std::as_bytes(std::span{m_bodyIn}) },
                    std::move(self)
                );
                readHeader();
            } else {
                handleError(std::move(self), error);
            }
        }
    );
}

void SocketConnection::writeHeaderAndBody() {
    const auto& buffer = m_queueOut.front();
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

    MessageHeader header{};
    header.id = std::to_underlying(message.id);
    header.bodyLength = static_cast<uint32_t>(message.stream.size());

    Buffer buffer{};
    buffer.resize(sizeof(MessageHeader) + header.bodyLength);

    Common::WriteIterator it = buffer.data();
    writeNumber<NETWORK_BYTE_ORDER>(header.id, it);
    writeNumber<NETWORK_BYTE_ORDER>(header.bodyLength, it);

    // TODO: Prettify
    std::memcpy(it, message.stream.data().data(), message.stream.size());

    asio::post(m_context, [this, buffer = std::move(buffer)]() mutable {
        bool writing = !m_queueOut.empty();
        m_queueOut.pushBack(std::move(buffer));
        if (!writing) {
            writeHeaderAndBody();
        }
    });
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
            connection
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
