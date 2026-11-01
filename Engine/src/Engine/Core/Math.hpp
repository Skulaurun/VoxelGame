#pragma once

#include <glm/glm.hpp>

// https://mathworld.wolfram.com/HessianNormalForm.html
struct Plane {
    glm::vec3 normal;
    float constant;
};

template<typename Point>
struct AlignedBox {
    Point min;
    Point max;

    auto intersection(const AlignedBox& other) const -> AlignedBox {
        return {
            glm::max(min, other.min),
            glm::min(max, other.max)
        };
    }

    auto empty() const -> bool {
        return glm::any(glm::greaterThan(min, max));
    }
};

using AlignedBox3f = AlignedBox<glm::vec3>;
using AlignedBox3i = AlignedBox<glm::ivec3>;

using Point3 = glm::vec3;

constexpr inline glm::vec3 toCartesian(const float theta, const float phi) {
    return {
        glm::cos(theta) * glm::cos(phi),
        glm::sin(theta),
        glm::cos(theta) * glm::sin(phi)
    };
}
