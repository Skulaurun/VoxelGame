#include <Engine/Renderer/ChunkRenderer.hpp>

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>
#include <Engine/Renderer/Frustum.hpp>

#include <Engine/World/Chunk.hpp>

#include <tracy/Tracy.hpp>

void ChunkRenderer::render(const Frustum& frustum) {
    ZoneScoped;

    m_stats = {};

    for (const auto& renderable : m_renderables) {
        if (renderable.enabled && renderable.indexCount != 0) {
            if (!frustum.intersectsBox(renderable.boundingBox)) {
                continue;
            }
            Renderer::draw(*renderable.vertexArray, renderable.indexCount);
            m_stats.numDrawCalls++;
        }
    }
}

void ChunkRenderer::uploadMesh(const Mesh& mesh, std::size_t index) {
    ZoneScoped;

    if (mesh.indices.empty()) {
        return;
    }

    auto vertexArray = m_renderables[index].vertexArray;
    vertexArray->getIndexBuffer()->upload(mesh.indices.data(), 0, mesh.indices.size() * sizeof(uint32_t));
    vertexArray->getVertexBuffers()[0]->upload(mesh.vertices.data(), 0, mesh.vertices.size() * sizeof(Vertex));
    m_renderables[index].indexCount = mesh.indices.size();
    m_renderables[index].boundingBox = mesh.boundingBox;
    m_renderables[index].enabled = true;
}

void ChunkRenderer::setEnabled(std::size_t index, bool value) {
    assert(index >= 0 && index < m_renderables.size());
    m_renderables[index].enabled = value;
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
