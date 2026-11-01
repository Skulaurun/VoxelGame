#include "Client/ChunkRenderer.hpp"
#include "Client/World/Chunk.hpp"

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>
#include <Engine/Renderer/Frustum.hpp>

#include <tracy/Tracy.hpp>

namespace Client {

void ChunkRenderer::init() {
    std::vector<uint32_t> indices{};
    generateQuadIndices(indices, Chunk::MAX_FACES);

    m_vertexIndices = std::make_shared<IndexBuffer>(indices.data(), indices.size() * sizeof(uint32_t));
}

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
    assert(mesh.indices.size() < Chunk::MAX_FACES * 6);

    std::size_t index{};
    auto it = m_chunkIndices.find(position);
    if (it != m_chunkIndices.end()) {
        index = it->second;
    } else {
        if (m_freeIndices.empty()) {
            index = addRenderable(mesh.vertices.size());
        } else [[likely]] {
            index = m_freeIndices.front();
            m_freeIndices.pop_front();
        }
    }

    m_chunkIndices[position] = index;

    auto& renderable = m_renderables[index];
    auto& vertexArray = renderable.vertexArray;
    vertexArray->getVertexBuffers()[0]->resize(mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
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

auto ChunkRenderer::addRenderable(std::size_t vertexCount) -> std::size_t {
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertexCount * sizeof(Vertex));
    vertexBuffer->setLayout({
        VertexAttributeType::Float3,
        VertexAttributeType::Float3,
        VertexAttributeType::Float4,
        VertexAttributeType::Float2,
        VertexAttributeType::Float,
        VertexAttributeType::Float
    });

    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->addVertexBuffer(vertexBuffer);
    vertexArray->setIndexBuffer(m_vertexIndices);

    auto index = m_renderables.size();
    auto& renderable = m_renderables.emplace_back();
    renderable.vertexArray = std::move(vertexArray);

    return index;
}

}
