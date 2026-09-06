#pragma once

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>
#include <Engine/Renderer/Renderer.hpp>

#include <Engine/Core/Math.hpp>

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    Box3 boundingBox;
};

inline void generateQuadIndices(Mesh& mesh, std::size_t quadCount) {
    mesh.indices.resize(quadCount * 6);
    for (std::size_t i = 0, j = 0; i < mesh.indices.size(); i += 6, j += 4) {
        mesh.indices[i + 0] = j + 0;
        mesh.indices[i + 1] = j + 1;
        mesh.indices[i + 2] = j + 2;

        mesh.indices[i + 3] = j + 2;
        mesh.indices[i + 4] = j + 3;
        mesh.indices[i + 5] = j + 0;
    }
}
