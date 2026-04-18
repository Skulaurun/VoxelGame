#pragma once

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <Engine/Mesh.hpp>

#include <vector>
#include <memory>

class ChunkRenderer {

public:
    struct Renderable {
        uint32_t indexCount = 0;
        bool enabled = false;
        std::shared_ptr<VertexArray> vertexArray;
    };

    ChunkRenderer() = default;

    void render();
    void uploadMesh(const Mesh& mesh, std::size_t index);

    void setEnabled(std::size_t index, bool value);

    void resize(uint32_t viewRange);

    uint32_t getViewRange() const { return viewRange; }

private:
    uint32_t viewRange;
    std::vector<Renderable> renderables;

};
