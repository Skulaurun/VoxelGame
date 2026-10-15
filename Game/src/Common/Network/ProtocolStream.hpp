#pragma once

#include "Common/ByteStream.hpp"
#include "Common/VoxelGrid.hpp"
#include "Common/Compression.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Common::Network {

constexpr auto NETWORK_BYTE_ORDER = std::endian::big;

class ProtocolStream : public DataStream<NETWORK_BYTE_ORDER> {

public:
    using DataStream::DataStream;
    
    using DataStream::write;
    using DataStream::read;

    // TODO: use 7-bit encoded integer

    // String

    auto write(std::string_view value) -> auto& {
        auto length = static_cast<std::uint64_t>(value.size());

        write(length);
        write(std::as_bytes(std::span{value.data(), length}));
        return *this;
    }

    auto read(std::string& value) -> auto& {
        std::uint64_t length{};
        read(length);

        value.resize(length);
        read(std::as_writable_bytes(std::span{value}));
        return *this;
    }

    // GLM

    template<glm::length_t Length, typename Value, glm::qualifier Qualifier>
    auto write(const glm::vec<Length, Value, Qualifier>& value) -> auto& {
        for (glm::length_t i = 0; i < Length; i++) {
            write(value[i]);
        }
        return *this;
    }

    template<glm::length_t Length, typename Value, glm::qualifier Qualifier>
    auto read(glm::vec<Length, Value, Qualifier>& value) -> auto& {
        for (std::size_t i = 0; i < Length; i++) {
            read(value[i]);
        }
        return *this;
    }

    // VoxelGrid

    template<int32_t Size>
    auto write(const VoxelGrid<Size>& grid) -> auto& {
        auto compressed = compressRle<Voxel>(grid.voxels());
        write(static_cast<uint32_t>(compressed.size())); // FIXME: this isnt byte size
        
        for (std::size_t i = 0; i < compressed.size(); i++) {
            write(compressed[i]);
        }
        return *this;
    }

    template<int32_t Size>
    auto read(VoxelGrid<Size>& grid) -> auto& {
        uint32_t length{};
        read(length); // FIXME: this isnt byte size

        std::vector<Voxel> compressed{};
        compressed.resize(length);
        for (std::size_t i = 0; i < compressed.size(); i++) {
            read(compressed[i]);
        }

        decompressRle<Voxel>(compressed, grid.voxels());
        return *this;
    }

};

}
