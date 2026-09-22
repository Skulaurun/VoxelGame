#pragma once

#include <span>
#include <vector>
#include <stdexcept>

template<typename Value>
    requires(std::is_integral_v<Value>)
auto compressRle(std::span<const Value> input) -> std::vector<Value> {
    std::vector<Value> output{};
    output.reserve(input.size());

    constexpr Value RLE_DUPLICATE{Value{0x0}};
    constexpr Value RLE_COPY{Value{0x1} << (sizeof(Value) * 8 - 1)};
    constexpr Value RLE_LENGTH{static_cast<Value>(~RLE_COPY)};

    std::size_t i = 0;
    while (i < input.size()) {
        std::size_t j = i + 1;
        std::size_t c = output.size();
        
        if (j >= input.size()) {
            output.emplace_back(RLE_COPY | 1);
            output.emplace_back(input[i]);
            break;
        }

        Value mode = input[i] != input[i + 1] ? RLE_COPY : RLE_DUPLICATE;

        output.emplace_back(mode);
        output.emplace_back(input[i]);

        if (mode == RLE_COPY) {
            while (((j + 1) < input.size() && input[j] != input[j + 1] && (j - i) < RLE_LENGTH)) {
                output.emplace_back(input[j++]);
            }
        } else {
            while ((j + 1) < input.size() && input[j] == input[j + 1] && (j - i + 1) < RLE_LENGTH) {
                j++;
            }
            j++;
        }

        output[c] |= (j - i);
        i = j;
    }

    return output;
}

template<typename Value>
    requires(std::is_integral_v<Value>)
auto decompressRle(std::span<const Value> input, std::size_t outputLength) {
    std::vector<Value> output = {};
    output.resize(outputLength);

    constexpr Value RLE_DUPLICATE{Value{0x0}};
    constexpr Value RLE_COPY{Value{0x1} << (sizeof(Value) * 8 - 1)};
    constexpr Value RLE_LENGTH{static_cast<Value>(~RLE_COPY)};

    Value count = 0;
    std::size_t length = 0;
    const Value* ip = input.data();
    const Value* ipEnd = ip + input.size();
    Value* op = output.data();
    Value* opEnd = op + output.size();
    while (ip < ipEnd) {
        if (op >= opEnd) [[unlikely]] {
            throw std::runtime_error{"Warning: RLE decompression overflow, expected less data."};
            break;
        }
        count = *ip++;
        /* Check if first bit is set */
        if ((count & RLE_COPY) != RLE_COPY) {
            /* Compression */
            length = static_cast<std::size_t>(count);
            for (std::size_t i = 0; i < length; i++) {
                *op++ = *ip;
            }
            ip++;
        } else {
            /* Raw Data */
            length = static_cast<std::size_t>(count) & RLE_LENGTH;
            for (std::size_t i = 0; i < length; i++) {
                *op++ = *ip++;
            }
        }
    }

    return output;
}
