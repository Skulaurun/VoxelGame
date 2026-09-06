#pragma once

#include <glm/glm.hpp>

// https://mathworld.wolfram.com/HessianNormalForm.html
struct Plane {
    glm::vec3 normal;
    float constant;
};

// axis-aligned bounding box
struct Box3 {
    glm::vec3 min;
    glm::vec3 max;
};

using Point3 = glm::vec3;

constexpr inline glm::vec3 toCartesian(const float theta, const float phi) {
    return {
        glm::cos(theta) * glm::cos(phi),
        glm::sin(theta),
        glm::cos(theta) * glm::sin(phi)
    };
}
