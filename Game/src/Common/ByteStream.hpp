#pragma once

#include "Common/Serialization.hpp"

#include <cstddef>
#include <bit>
#include <span>
#include <vector>
#include <stdexcept>

namespace Common {

// TODO: consider splitting into ReadStream and WriteStream

template<std::endian Endian>
class DataStream
{
public:
    DataStream() : m_head{} {}

    DataStream(std::span<const std::byte> data) :
        m_head{},
        m_data{std::from_range, data}
    {}

    template<typename Value>
        requires(std::is_integral_v<Value> || std::is_floating_point_v<Value>)
    auto write(Value value) -> auto& {
        if constexpr (std::endian::native != Endian) {
            value = byteswap(value);
        }

        write(std::span{ reinterpret_cast<const std::byte*>(&value), sizeof(Value) });
        return *this;
    }

    auto write(std::span<const std::byte> value) -> auto& {
        if (value.size() > remaining()) {
            m_data.resize(m_data.size() + value.size());
        }
        std::copy_n(value.data(), value.size(), m_data.begin() + m_head);
        advance(value.size());
        return *this;
    }

    template<typename Value>
        requires(std::is_integral_v<Value> || std::is_floating_point_v<Value>)
    auto read(Value& value) -> auto& {
        read(std::span{ reinterpret_cast<std::byte*>(&value), sizeof(Value) });

        if constexpr (std::endian::native != Endian) {
            value = byteswap(value);
        }
        return *this;
    }

    auto read(std::span<std::byte> value) -> auto& {
        if (value.size() > remaining()) {
            throw std::runtime_error{ "Tried to read past the end of ByteStream" };
        }
        std::copy_n(m_data.begin() + m_head, value.size(), value.data());
        advance(value.size());
        return *this;
    }

    auto seek(std::size_t offset) -> auto& {
        m_head = offset;
        return *this;
    }

    auto advance(std::size_t length) -> auto& {
        m_head += length;
        if (m_head > m_data.size()) {
            m_data.resize(m_head);
        }
        return *this;
    }

    auto remaining() const { return m_data.size() - m_head; };

    auto head() const { return m_head; }

    auto size() const { return m_data.size(); }

    auto data() -> auto& { return m_data; }
    auto data() const -> const auto& { return m_data; }

private:
    std::size_t m_head;
    std::vector<std::byte> m_data;
};

}
