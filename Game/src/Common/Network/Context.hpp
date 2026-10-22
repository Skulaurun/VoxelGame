#pragma once

#include "Message.hpp"
#include "Common/TsQueue.hpp"

#include "Asio.hpp"

namespace Common::Network {

template<typename Handler>
class Context {

public:
    Context(std::shared_ptr<Handler> handler) :
        m_handler{handler},
        m_workGuard{asio::make_work_guard(m_context)}
    {}

    ~Context() {
        stop();
    }

    void start() {
        m_thread = std::jthread([this] {
            m_context.run();
        });
    }

    void stop() {
        m_workGuard.reset();
        m_context.stop();
    }

    void pollEvents(bool blocking = true) {
        if (blocking) m_queueIn.wait();

        // TODO: Add budget

        while (!m_queueIn.empty()) {
            std::visit(*m_handler, m_queueIn.front());
            m_queueIn.popFront();
        }
    }

    auto context() -> asio::io_context& { return m_context; }
    auto context() const -> const asio::io_context& { return m_context; }

    auto queueIn() -> TsQueue<Event>& { return m_queueIn; }
    auto queueIn() const -> const TsQueue<Event>& { return m_queueIn; }

private:
    TsQueue<Event> m_queueIn;
    std::jthread m_thread;
    asio::io_context m_context;
    asio::executor_work_guard<asio::io_context::executor_type> m_workGuard;

    std::shared_ptr<Handler> m_handler;

};

}
