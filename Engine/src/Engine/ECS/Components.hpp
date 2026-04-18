#pragma once

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

struct Velocity {
    glm::vec3 linear;
    glm::vec3 angular;
};

struct Transform {
    glm::vec3 translation;
    glm::quat rotation;
    glm::vec3 scale;
};

struct RigidBody {
    float mass;
};
