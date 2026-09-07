#pragma once

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <Engine/Mesh.hpp>

#include <vector>
#include <memory>

class Frustum;

class ChunkRenderer {

public:
    struct Renderable {
        uint32_t indexCount = 0;
        bool enabled = false;
        std::shared_ptr<VertexArray> vertexArray;
        Box3 boundingBox;
    };

    struct Statistics {
        uint32_t numDrawCalls;
    };

    ChunkRenderer() = default;

    void render(const Frustum& frustum);
    void uploadMesh(const Mesh& mesh, std::size_t index);

    void setEnabled(std::size_t index, bool value);

    void resize(uint32_t viewRange);

    auto viewRange() const { return m_viewRange; }
    auto statistics() const -> const auto& { return m_stats; }

private:
    uint32_t m_viewRange;
    std::vector<Renderable> m_renderables;

    Statistics m_stats;

};
