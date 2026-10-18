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
        if (renderable.indexCount != 0) {
            if (!frustum.intersectsBox(renderable.boundingBox)) {
                continue;
            }
            Renderer::draw(*renderable.vertexArray, renderable.indexCount);
            m_stats.numDrawCalls++;
        }
    }
}

void ChunkRenderer::loadMesh(const glm::ivec3& position, const Mesh& mesh) {
    ZoneScoped;

    if (mesh.indices.empty()) {
        return;
    }

    std::size_t index{};
    auto it = m_chunkIndices.find(position);
    if (it != m_chunkIndices.end()) {
        index = it->second;
    } else {
        if (m_freeIndices.empty()) {
            for (std::size_t i = 0; i < m_renderables.size(); i++) {
                if (m_renderables[i].empty) {
                    index = i;
                    break;
                }
            }
        } else [[likely]] {
            index = m_freeIndices.front();
            m_freeIndices.pop_front();
        }
    }

    m_chunkIndices[position] = index;

    auto& renderable = m_renderables[index];
    auto& vertexArray = renderable.vertexArray;
    vertexArray->getIndexBuffer()->upload(mesh.indices.data(), 0, mesh.indices.size() * sizeof(uint32_t));
    vertexArray->getVertexBuffers()[0]->upload(mesh.vertices.data(), 0, mesh.vertices.size() * sizeof(Vertex));
    renderable.indexCount = mesh.indices.size();
    renderable.boundingBox = mesh.boundingBox;
    renderable.empty = false;
}

void ChunkRenderer::unloadMesh(const glm::ivec3& position) {
    auto it = m_chunkIndices.find(position);
    if (it != m_chunkIndices.end()) {
        m_renderables[it->second].indexCount = 0;
        m_renderables[it->second].empty = true;
        m_freeIndices.emplace_back(it->second);
        m_chunkIndices.erase(position);
    }
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
