#pragma once

#include <bit>
#include <cstring>
#include <concepts>

namespace Common {

template<typename Value>
    requires(std::is_integral_v<Value> || std::is_floating_point_v<Value>)
constexpr auto byteswap(Value value) {
    if constexpr (std::same_as<Value, float>) {
        return std::bit_cast<float>(std::byteswap(std::bit_cast<std::uint32_t>(value)));
    } else if constexpr (std::same_as<Value, double>) {
        return std::bit_cast<double>(std::byteswap(std::bit_cast<std::uint64_t>(value)));
    } else {
        return std::byteswap(value);
    }
}

using WriteIterator = uint8_t*;
using ReadIterator = const uint8_t*;

template<std::endian Endian, typename Value>
    requires(std::integral<Value> || std::floating_point<Value>)
void readNumber(Value& value, ReadIterator& iterator) {
    constexpr auto Size = sizeof(Value);
    
    std::memcpy(&value, iterator, sizeof(Value));
    iterator += Size;

    if constexpr (std::endian::native != Endian) {
        value = byteswap(value);
    }
}

template<std::endian Endian, typename Value>
    requires(std::integral<Value> || std::floating_point<Value>)
void writeNumber(Value value, WriteIterator& iterator) {
    constexpr auto Size = sizeof(Value);

    if constexpr (std::endian::native != Endian) {
        value = byteswap(value);
    }

    std::memcpy(iterator, &value, sizeof(Value));
    iterator += Size;
}

}
