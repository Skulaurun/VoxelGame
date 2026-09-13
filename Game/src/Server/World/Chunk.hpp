#pragma once

#include "Common/WorldConfig.hpp"
#include "Common/VoxelGrid.hpp"

#include <Engine/Mesh.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Core/Math.hpp>

#include <glm/glm.hpp>

struct Chunk {
    constexpr static auto SIZE = WorldConfig::CHUNK_SIZE;

    using VoxelGrid = VoxelGrid<WorldConfig::CHUNK_SIZE>;

    enum class State : uint8_t {
        Empty,

        // Generation stages
        Terrain,
        Vegetation,

        Done
    };

    struct Holder {
        State state;
        std::unique_ptr<Chunk> chunk;
    };

    glm::ivec3 position;
    VoxelGrid grid;
};
