#include "Buffer.hpp"

#include <type_traits>

#define READ_PRIMITIVE(s, p) s.read((char*)&p, sizeof(p))

#define READ_NUMBER16(s, n, e)                                                                     \
    {                                                                                              \
        READ_PRIMITIVE(s, n);                                                                      \
        n = e ? REVERSE_I16(n) : n;                                                                \
    }

#define READ_NUMBER32(s, n, e)                                                                     \
    {                                                                                              \
        READ_PRIMITIVE(s, n);                                                                      \
        n = e ? REVERSE_I32(n) : n;                                                                \
    }

namespace Common {

Buffer::Buffer(const std::filesystem::path& path, const bool isLittleEndian) :
    littleEndian{isLittleEndian}
{
    stream = std::ifstream(path, std::ios::binary);
}

Buffer::~Buffer() { stream.close(); }

auto Buffer::readInt8(int8_t& destination) -> Buffer&
{
    READ_PRIMITIVE(stream, destination);
    return *this;
}

auto Buffer::readUInt8(uint8_t& destination) -> Buffer&
{
    READ_PRIMITIVE(stream, destination);
    return *this;
}

auto Buffer::readInt16(int16_t& destination) -> Buffer&
{
    READ_NUMBER16(stream, destination, littleEndian);
    return *this;
}

auto Buffer::readInt32(int32_t& destination) -> Buffer&
{
    READ_NUMBER32(stream, destination, littleEndian);
    return *this;
}

auto Buffer::readUInt16(uint16_t& destination) -> Buffer&
{
    READ_NUMBER16(stream, destination, littleEndian);
    return *this;
}

auto Buffer::readUInt32(uint32_t& destination) -> Buffer&
{
    READ_NUMBER32(stream, destination, littleEndian);
    return *this;
}

auto Buffer::read(const char* destination, std::size_t length) -> Buffer&
{
    stream.read((char*)destination, length);
    return *this;
}

auto Buffer::seek(const std::streampos offset) -> Buffer&
{
    stream.seekg(offset);
    return *this;
}

auto Buffer::advance(std::streamsize length) -> Buffer&
{
    stream.ignore(length);
    return *this;
}

auto Buffer::tell() -> std::size_t { return stream.tellg(); }

void Buffer::close() { stream.close(); }

}
