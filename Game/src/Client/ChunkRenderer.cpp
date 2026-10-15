#include "Client/ChunkRenderer.hpp"
#include "Client/World/Chunk.hpp"

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>
#include <Engine/Renderer/Frustum.hpp>

#include <tracy/Tracy.hpp>

namespace Client {

void ChunkRenderer::render(const Frustum& frustum) {
    ZoneScoped;

    m_stats = {};

    for (const auto& renderable : m_renderables) {
        if (renderable.chunkId != 0 && renderable.indexCount != 0) {
            if (!frustum.intersectsBox(renderable.boundingBox)) {
                continue;
            }
            Renderer::draw(*renderable.vertexArray, renderable.indexCount);
            m_stats.numDrawCalls++;
        }
    }
}

void ChunkRenderer::loadMesh(ChunkId id, const Mesh& mesh) {
    ZoneScoped;

    if (mesh.indices.empty()) {
        return;
    }

    for (std::size_t i = 0; i < m_renderables.size(); i++) {
        auto& renderable = m_renderables[i];
        if (renderable.chunkId == 0) {
            auto& vertexArray = renderable.vertexArray;
            vertexArray->getIndexBuffer()->upload(mesh.indices.data(), 0, mesh.indices.size() * sizeof(uint32_t));
            vertexArray->getVertexBuffers()[0]->upload(mesh.vertices.data(), 0, mesh.vertices.size() * sizeof(Vertex));
            renderable.indexCount = mesh.indices.size();
            renderable.boundingBox = mesh.boundingBox;
            renderable.chunkId = id;
            return;
        }
    }
    assert(false && "No empty slot for mesh");
}

void ChunkRenderer::unloadMesh(ChunkId id) {
    for (std::size_t i = 0; i < m_renderables.size(); i++) {
        if (m_renderables[i].chunkId == id) {
            m_renderables[i].chunkId = 0;
            return;
        }
    }
    assert(false && "Tried to unload mesh with unknown id");
}

void ChunkRenderer::resize(uint32_t viewRange) {
    ZoneScoped;

    m_viewRange = viewRange;
    const uint32_t side = 2 * viewRange + 1;
    m_renderables.resize(side * side * side);

    for (std::size_t i = 0; i < m_renderables.size(); i++) {
        auto indexBuffer = std::make_shared<IndexBuffer>(6 * Chunk::MAX_FACES * sizeof(uint32_t));
        auto vertexBuffer = std::make_shared<VertexBuffer>(4 * Chunk::MAX_FACES * sizeof(Vertex));
        vertexBuffer->setLayout({
            VertexAttributeType::Float3,
            VertexAttributeType::Float4,
            VertexAttributeType::Float2,
            VertexAttributeType::Float,
            VertexAttributeType::Float
        });
    
        auto vertexArray = std::make_shared<VertexArray>();
        vertexArray->addVertexBuffer(vertexBuffer);
        vertexArray->setIndexBuffer(indexBuffer);
        m_renderables[i].vertexArray = vertexArray;
    }
}

}
