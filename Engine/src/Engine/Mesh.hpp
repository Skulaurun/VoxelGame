#pragma once

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>
#include <Engine/Renderer/Renderer.hpp>

#include <Engine/Core/Math.hpp>

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    AlignedBox3f boundingBox;
};

inline void generateQuadIndices(std::vector<uint32_t>& indices, std::size_t quadCount) {
    indices.resize(quadCount * 6);
    for (std::size_t i = 0, j = 0; i < indices.size(); i += 6, j += 4) {
        indices[i + 0] = j + 0;
        indices[i + 1] = j + 1;
        indices[i + 2] = j + 2;

        indices[i + 3] = j + 2;
        indices[i + 4] = j + 3;
        indices[i + 5] = j + 0;
    }
}
