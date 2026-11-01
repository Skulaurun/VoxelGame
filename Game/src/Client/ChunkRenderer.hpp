#pragma once

#include "Client/World/Chunk.hpp"

#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <Engine/Mesh.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <memory>
#include <deque>

class Frustum;

namespace Client {

class ChunkRenderer {

public:
    struct Renderable {
        bool empty{true};
        uint32_t indexCount{0};
        std::shared_ptr<VertexArray> vertexArray;
        AlignedBox3f boundingBox;
    };

    struct Statistics {
        uint32_t numDrawCalls;
    };

    ChunkRenderer() = default;

    // TODO: remove after Client has member that initializes OpenGL
    void init();

    void render(const Frustum& frustum);

    void loadMesh(const glm::ivec3& position, const Mesh& mesh);
    void unloadMesh(const glm::ivec3& position);

    auto statistics() const -> const auto& { return m_stats; }

private:
    auto addRenderable(std::size_t vertexCount = 0) -> std::size_t;

private:
    std::vector<Renderable> m_renderables;

    std::shared_ptr<IndexBuffer> m_vertexIndices;

    std::deque<std::size_t> m_freeIndices;
    std::unordered_map<glm::ivec3, std::size_t> m_chunkIndices;

    Statistics m_stats;

};

}
