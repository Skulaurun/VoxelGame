#pragma once

#include <glm/glm.hpp>

#include <array>

// TODO: dedup vertices (3D)

constexpr static std::array<glm::vec3, 6 * 4> CUBE_FACES{{
    // LEFT
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },

    // RIGHT
    { 1.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 1.0f },

    // FRONT
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f },

    // BACK
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },

    // TOP
    { 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f },

    // BOTTOM
    { 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f }
}};

constexpr static std::array<glm::vec3, 6 * 4> CUBE_VERTEX_NORMALS{{
    // LEFT
    { -1.0f, 0.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },
    { -1.0f, 0.0f, 0.0f },

    // RIGHT
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },

    // FRONT
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f },

    // BACK
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, -1.0f },
    { 0.0f, 0.0f, -1.0f },

    // TOP
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },

    // BOTTOM
    { 0.0f, -1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f },
    { 0.0f, -1.0f, 0.0f }
}};
