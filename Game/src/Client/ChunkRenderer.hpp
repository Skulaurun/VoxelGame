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
        Box3 boundingBox;
    };

    struct Statistics {
        uint32_t numDrawCalls;
    };

    ChunkRenderer() = default;

    void render(const Frustum& frustum);

    void loadMesh(const glm::ivec3& position, const Mesh& mesh);
    void unloadMesh(const glm::ivec3& position);

    void resize(uint32_t viewRange);

    auto viewRange() const { return m_viewRange; }
    auto statistics() const -> const auto& { return m_stats; }

private:
    uint32_t m_viewRange;
    std::vector<Renderable> m_renderables;

    std::deque<std::size_t> m_freeIndices;
    std::unordered_map<glm::ivec3, std::size_t> m_chunkIndices;

    Statistics m_stats;

};

}
