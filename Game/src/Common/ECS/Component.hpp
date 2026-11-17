#pragma once

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Ecs {

// Physics

struct Position {
    glm::vec3 value;
};

struct Velocity {
    glm::vec3 value;
};

struct Rotation {
    glm::quat value;
};

}
