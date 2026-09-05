#pragma once

#include <glm/glm.hpp>

#include <string_view>

struct StringHasher : public std::hash<std::string_view> {
    using is_transparent = void;
};

template<std::size_t Size>
constexpr auto embedView(const char (&data)[Size]) -> std::string_view {
    return { data, Size };
}

constexpr glm::vec3 WORLD_UP{0.0f, 1.0f, 0.0f};

using WorldPosition = glm::vec3;
