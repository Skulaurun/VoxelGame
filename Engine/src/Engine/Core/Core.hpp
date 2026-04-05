#pragma once

#include <string_view>

struct StringHasher : public std::hash<std::string_view> {
    using is_transparent = void;
};
