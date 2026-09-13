#pragma once

#include <deque>
#include <mutex>

namespace Common {

template<typename Value>
class TsQueue {

public:
    TsQueue() = default;

    template <typename... Arguments>
    void pushBack(Arguments&&... arguments) {
        {
            std::scoped_lock lock{m_mutexQueue};
            m_queue.emplace_back(std::forward<Arguments>(arguments)...);
        }
        m_waitCondition.notify_one();
    }

    template <typename... Arguments>
    void pushFront(Arguments&&... arguments) {
        {
            std::scoped_lock lock{m_mutexQueue};
            m_queue.emplace_front(std::forward<Arguments>(arguments)...);
        }
        m_waitCondition.notify_one();
    }

    void popFront() {
        std::scoped_lock lock{m_mutexQueue};
        m_queue.pop_front();
    }

    void popBack() {
        std::scoped_lock lock{m_mutexQueue};
        m_queue.pop_back();
    }

    auto empty() const -> bool {
        std::scoped_lock lock{m_mutexQueue};
        return m_queue.empty();
    }

    auto back() -> Value& {
        std::scoped_lock lock{m_mutexQueue};
        return m_queue.back();
    }

    auto back() const -> const Value& {
        std::scoped_lock lock{m_mutexQueue};
        return m_queue.back();
    }
    
    auto front() -> Value& {
        std::scoped_lock lock{m_mutexQueue};
        return m_queue.front();
    }

    auto front() const -> const Value& {
        std::scoped_lock lock{m_mutexQueue};
        return m_queue.front();
    }

    void wait() {
        std::unique_lock lock{m_mutexWait};
        m_waitCondition.wait(lock, [this] {
            return !empty();
        });
    }

private:
    std::mutex m_mutexWait;
    std::condition_variable m_waitCondition;

    std::mutex m_mutexQueue;
    std::deque<Value> m_queue;

};

}
