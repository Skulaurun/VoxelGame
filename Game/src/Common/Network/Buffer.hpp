#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

#define REVERSE_I16(n) int16_t(((n & 0x00ff) << 8) | ((n & 0xff00) >> 8))
#define REVERSE_I32(n)                                                                             \
    int32_t(                                                                                       \
        ((n & 0x000000ff) << 24) | ((n & 0x00ff0000) >> 8) | ((n & 0x0000ff00) << 8) |             \
        ((n & 0xff000000) >> 24)                                                                   \
    )

namespace Common {

class Buffer
{
public:
    Buffer(bool isLittleEndian = true);
    ~Buffer();

    auto readInt8(int8_t& destination) -> Buffer&;
    auto readUInt8(uint8_t& destination) -> Buffer&;

    auto readInt16(int16_t& destination) -> Buffer&;
    auto readUInt16(uint16_t& destination) -> Buffer&;

    auto readInt32(int32_t& destination) -> Buffer&;
    auto readUInt32(uint32_t& destination) -> Buffer&;

    auto read(const char* destination, std::size_t length) -> Buffer&;
    auto seek(const std::size_t offset) -> Buffer&;
    auto advance(std::size_t length) -> Buffer&;

    auto tell() -> std::size_t;
    void close();

    void setEndian(const bool isLittle) { littleEndian = isLittle; }

    auto isLittleEndian() const -> bool { return littleEndian; }

private:
    bool littleEndian;
    std::vector<uint8_t> data;
};

}
