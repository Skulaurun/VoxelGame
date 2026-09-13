#pragma once

#include "Message.hpp"
#include "Connection.hpp"
#include "Common/TsQueue.hpp"

#include <asio.hpp>

namespace Common::Network {

class Context {

public:
    Context();
    virtual ~Context();

    virtual void start();
    virtual void stop();

    virtual void update(bool blocking = true);

    virtual void addHandler(const std::shared_ptr<Handler>& handler);

    virtual void onMessage(InternalMessage& message) = 0;

protected:
    TsQueue<Event> queueIn;

    asio::io_context context;
    std::jthread thread;

    std::vector<std::shared_ptr<Handler>> handlers;

};

}