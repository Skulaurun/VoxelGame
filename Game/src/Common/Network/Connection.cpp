#include "Connection.hpp"

namespace Common::Network {

Connection::Connection(TsQueue<Event>& queue, asio::io_context& context, asio::ip::tcp::socket socket)
    : owner(owner), queueIn(queue), context(context), socket(std::move(socket)) {
    //this->headerIn = { Message::Type::Unknown, 0 };
}

Connection::~Connection() {
    close();
}

void Connection::connect(const asio::ip::tcp::resolver::results_type& endpoints) {
    asio::async_connect(socket, endpoints,
        [this](std::error_code error, asio::ip::tcp::endpoint endpoint) {
            if (!error) {
                readHeader();
                pushInternalMessage(InternalMessage::Type::Connect);
            } else {
                pushInternalMessage(InternalMessage::Type::Connect, error);
            }
        }
    );
}

void Connection::close() {
    if (isConnected()) {
        socket.shutdown(asio::socket_base::shutdown_both);
        socket.close();
    }
}

void Connection::readHeader() {
    asio::async_read(socket, asio::buffer(headerIn, sizeof(headerIn)),
        [this](std::error_code error, std::size_t length) {
            if (!error) {

                MessageHeader header = {};
                const auto* it = headerIn;
                header.read(it, sizeof(headerIn));

                auto messageId = std::get<0>(header.getFields()).value();
                auto bodyLength = std::get<1>(header.getFields()).value();

                if (bodyLength > 0) {
                    readBody(messageId, bodyLength);
                } else {
                    pushIncomingMessage(Message::create(messageId));
                    readHeader();
                }
            } else {
                if (error.value() == asio::error::operation_aborted) return;
                socket.close();
                pushInternalMessage(InternalMessage::Type::Read, error);
            }
        }
    );
}

void Connection::readBody(const uint32_t id, const uint32_t length) {
    // Check if body length isnt too large
    bodyIn.resize(length);

    asio::async_read(socket, asio::buffer(bodyIn.data(), bodyIn.size()),
        [this, id](std::error_code error, std::size_t length) {
            if (!error) {

                std::shared_ptr<Message> message = Message::create(id);
                const auto* it = bodyIn.data();
                message->read(it, bodyIn.size());

                pushIncomingMessage(message);
                readHeader();
            } else {
                if (error.value() == asio::error::operation_aborted) return;
                socket.close();
                pushInternalMessage(InternalMessage::Type::Read, error);
            }
        }
    );
}

void Connection::pushIncomingMessage(const std::shared_ptr<Message> message) {
    queueIn.push(
        IncomingMessage(sharedFromThis(), message)
    );
}

void Connection::pushInternalMessage(const InternalMessage::Type type, const std::error_code error) {
    queueIn.push(
        InternalMessage(sharedFromThis(), type, error)
    );
}

void Connection::writeHeaderAndBody() {
    const auto& buffer = *queueOut.front();
    asio::async_write(socket, asio::buffer(buffer.data(), buffer.size()),
        [this](std::error_code error, std::size_t length) {
            if (!error) {
                queueOut.shift();
                if (!queueOut.empty()) {
                    writeHeaderAndBody();
                }
            } else {
                if (error.value() == asio::error::operation_aborted) return;
                socket.close();
                pushInternalMessage(InternalMessage::Type::Write, error);
            }
        }
    );
}

void Connection::send(const Message& message) {
    MessageHeader header = {};
    std::get<0>(header.getFields()).value() = static_cast<uint32_t>(message.getId());
    std::get<1>(header.getFields()).value() = static_cast<uint32_t>(message.length());

    auto bufferPtr = std::make_shared<std::vector<uint8_t>>();
    auto& buffer = *bufferPtr;
    buffer.resize(header.length() + message.length());

    uint8_t* it = buffer.data();
    header.write(it, buffer.size()).and_then([&](auto bytesWritten) {
        return message.write(it, buffer.size() - bytesWritten);
    });

    asio::post(context, [this, bufferPtr]() {
        bool writing = !queueOut.empty();
        queueOut.push(bufferPtr);
        if (!writing) {
            writeHeaderAndBody();
        }
    });
}

std::shared_ptr<Connection> Connection::sharedFromThis() {
    if (owner == Owner::Server) {
        return shared_from_this();
    } else {
        return nullptr;
    }
}

}
