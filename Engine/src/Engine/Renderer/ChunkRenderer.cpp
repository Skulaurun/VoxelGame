#include <Engine/Renderer/ChunkRenderer.hpp>

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <Engine/World/Chunk.hpp>

#include <tracy/Tracy.hpp>

void ChunkRenderer::render() {
    ZoneScoped;

    for (const auto& renderable : renderables) {
        if (renderable.enabled) {
            Renderer::draw(*renderable.vertexArray, renderable.indexCount);
        }
    }
}

void ChunkRenderer::uploadMesh(const Mesh& mesh, std::size_t index) {
    ZoneScoped;

    auto vertexArray = renderables[index].vertexArray;
    vertexArray->getIndexBuffer()->upload(mesh.indices.data(), 0, mesh.indices.size() * sizeof(uint32_t));
    vertexArray->getVertexBuffers()[0]->upload(mesh.vertices.data(), 0, mesh.vertices.size() * sizeof(Vertex));
    renderables[index].indexCount = mesh.indices.size();
    renderables[index].enabled = true;
}

void ChunkRenderer::setEnabled(std::size_t index, bool value) {
    assert(index >= 0 && index < renderables.size());
    renderables[index].enabled = value;
}

void ChunkRenderer::resize(uint32_t viewRange) {
    ZoneScoped;

    this->viewRange = viewRange;
    const uint32_t side = 2 * viewRange + 1;
    renderables.resize(side * side * side);

    for (std::size_t i = 0; i < renderables.size(); i++) {
        auto indexBuffer = std::make_shared<IndexBuffer>(6 * Chunk::MAX_FACES * sizeof(uint32_t));
        auto vertexBuffer = std::make_shared<VertexBuffer>(4 * Chunk::MAX_FACES * sizeof(Vertex));
        vertexBuffer->setLayout({
            VertexAttributeType::Float3,
            VertexAttributeType::Float4,
            VertexAttributeType::Float2,
            VertexAttributeType::Float
        });
    
        auto vertexArray = std::make_shared<VertexArray>();
        vertexArray->addVertexBuffer(vertexBuffer);
        vertexArray->setIndexBuffer(indexBuffer);
        renderables[i].vertexArray = vertexArray;
    }
}
