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
    // TODO: dedup vertices (3D)

    constexpr static std::array<glm::vec3, 6 * 4> FACES{{
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

    constexpr static std::array<glm::vec3, 6 * 4> VERTEX_NORMALS{{
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

public:
    Voxel() : m_type(BlockType::Void) {}

    void setType(BlockType type) { m_type = type; }

    bool isTransparent() const { return m_type == BlockType::Void; }

    bool is(BlockType type) const { return type == m_type; }

private:
    BlockType m_type;

};
