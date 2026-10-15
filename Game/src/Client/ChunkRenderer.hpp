#pragma once

#include "Client/World/Chunk.hpp"

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <Engine/Mesh.hpp>

#include <vector>
#include <memory>

class Frustum;

namespace Client {

class ChunkRenderer {

public:
    struct Renderable {
        ChunkId chunkId = 0;
        uint32_t indexCount = 0;
        std::shared_ptr<VertexArray> vertexArray;
        Box3 boundingBox;
    };

    struct Statistics {
        uint32_t numDrawCalls;
    };

    ChunkRenderer() = default;

    void render(const Frustum& frustum);

    void loadMesh(ChunkId id, const Mesh& mesh);
    void unloadMesh(ChunkId id);

    void resize(uint32_t viewRange);

    auto viewRange() const { return m_viewRange; }
    auto statistics() const -> const auto& { return m_stats; }

private:
    uint32_t m_viewRange;
    std::vector<Renderable> m_renderables;

    Statistics m_stats;

};

}
