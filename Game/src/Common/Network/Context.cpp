#include "Context.hpp"

namespace Common::Network {

Context::Context() {}

Context::~Context() {
    stop();
}

void Context::start() {
    thread = std::jthread([this] {
        context.run();
    });
}

void Context::stop() {
    context.stop();
}

void Context::update(bool blocking) {
    if (blocking) queueIn.wait();

    while (!queueIn.empty()) {
        Event message = queueIn.front();
        queueIn.popFront();

        if (std::holds_alternative<InternalMessage>(message)) {
            onMessage(std::get<InternalMessage>(message));
        } else {
            //onMessage(std::get<IncomingMessage>(message));
            for (auto& handler : handlers) {
                // TODO: sender
                std::get<IncomingMessage>(message).payload->dispatch(*handler);
            }
        }
    }
}

void Context::addHandler(const std::shared_ptr<Handler>& handler) {
    handlers.push_back(handler);
}

}
