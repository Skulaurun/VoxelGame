#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include <memory>

template<typename Chunk>
class ChunkStorage {

public:
    ChunkStorage() = default;

    Chunk* at(const glm::ivec3& position) {
        auto it = m_chunks.find(position);
        return (it != m_chunks.end()) ? it->second.get() : nullptr;
    }

private:
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> m_chunks;

};
