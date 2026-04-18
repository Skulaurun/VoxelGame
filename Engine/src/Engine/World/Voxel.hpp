#pragma once

#include <glm/glm.hpp>

#include <array>

enum class BlockType {
    Void,
    Grass,
    Stone
};

class Voxel {

public:
    constexpr static std::array<glm::vec3, 6 * 4> FACES{{
        // LEFT
        { -0.5f, -0.5f, 0.5f },
        { -0.5f, -0.5f, -0.5f },
        { -0.5f, 0.5f, -0.5f },
        { -0.5f, 0.5f, 0.5f },

        // RIGHT
        { 0.5f, -0.5f, 0.5f },
        { 0.5f, -0.5f, -0.5f },
        { 0.5f, 0.5f, -0.5f },
        { 0.5f, 0.5f, 0.5f },

        // FRONT
        { -0.5f, -0.5f, 0.5f },
        { 0.5f, -0.5f, 0.5f },
        { 0.5f, 0.5f, 0.5f },
        { -0.5f, 0.5f, 0.5f },

        // BACK
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f },
        { 0.5f, 0.5f, -0.5f },
        { -0.5f, 0.5f, -0.5f },

        // TOP
        { 0.5f, 0.5f, 0.5f },
        { -0.5f, 0.5f, 0.5f },
        { -0.5f, 0.5f, -0.5f },
        { 0.5f, 0.5f, -0.5f },

        // BOTTOM
        { 0.5f, -0.5f, 0.5f },
        { -0.5f, -0.5f, 0.5f },
        { -0.5f, -0.5f, -0.5f },
        { 0.5f, -0.5f, -0.5f }
    }};

public:
    Voxel() : type(BlockType::Void) {}

    void setType(BlockType type) { this->type = type; }

    bool isTransparent() const { return type == BlockType::Void; }

private:
    BlockType type;

};
