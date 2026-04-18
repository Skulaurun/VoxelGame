#include <Engine/World/Chunk.hpp>
#include <Engine/World/World.hpp>
#include <Engine/World/Noise.hpp>

#include <tracy/Tracy.hpp>

Chunk::Chunk(World& world, glm::ivec3 position)
    : world(world), position(position) {

    for (int32_t y = 0; y < SIZE; y++) {
        for (int32_t x = 0; x < SIZE; x++) {
            for (int32_t z = 0; z < SIZE; z++) {
                glm::vec3 worldPosition = toWorldPosition({x, y, z});

                const float scale = 0.05f;
                float baseHeight = 48.0f + perlinNoise(glm::vec2{worldPosition.x, worldPosition.z} * scale) * 32.0f;
                float bias = -1.0f + (worldPosition.y / baseHeight) * 2.0f;

                float density = perlinNoise(worldPosition * scale) - bias;
                if (density > 0.0f) {
                    at({x, y, z})->setType(BlockType::Stone);
                }
            }
        }
    }
}

glm::ivec3 Chunk::toWorldPosition(glm::ivec3 voxelPosition) {
    return world.toWorldPosition(position, voxelPosition);
}

void Chunk::generateMesh(glm::vec3 offset) {
    ZoneScoped;

    /*std::array<Chunk*, 7> chunks = {
        // left
        world.getChunkAt(position + glm::ivec3(-1, 0, 0)),
        // right
        world.getChunkAt(position + glm::ivec3(1, 0, 0)),
        // front
        world.getChunkAt(position + glm::ivec3(0, 0, 1)),
        // back
        world.getChunkAt(position + glm::ivec3(0, 0, -1)),
        // top
        world.getChunkAt(position + glm::ivec3(0, 1, 0)),
        // bottom
        world.getChunkAt(position + glm::ivec3(0, -1, 0)),
        // center
        this
    };*/

    mesh.indices.clear();
    mesh.vertices.clear();

    constexpr std::array<glm::vec4, 4> COLORS{{
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f, 1.0f }
    }};

    std::size_t quadCount = 0;
    for (int32_t y = 0; y < SIZE; y++) {
        for (int32_t x = 0; x < SIZE; x++) {
            for (int32_t z = 0; z < SIZE; z++) {
                Voxel* voxel = at(glm::ivec3(x, y, z));
                if (voxel->isTransparent()) {
                    continue;
                }

                std::array<Voxel*, 6> neighbors = {
                    at(glm::ivec3(x - 1, y, z)), // left
                    at(glm::ivec3(x + 1, y, z)), // right
                    at(glm::ivec3(x, y, z + 1)), // front
                    at(glm::ivec3(x, y, z - 1)), // back
                    at(glm::ivec3(x, y + 1, z)), // top
                    at(glm::ivec3(x, y - 1, z))  // bottom
                };

                for (std::size_t i = 0; i < neighbors.size(); i++) {
                    if (!neighbors[i] || neighbors[i]->isTransparent()) {
                        // Face is visible
                        for (std::size_t j = 0; j < 4; j++) {
                            glm::vec3 position = Voxel::FACES[i * 4 + j];
                            position += glm::ivec3(x, y, z);

                            const float centerOffset = (SIZE - 1) / 2.0f;
                            position += glm::vec3(-centerOffset, -centerOffset, -centerOffset);
                            position += offset;

                            mesh.vertices.emplace_back(Vertex{ position, COLORS[j], glm::vec2{}, 0.0f });
                        }
                        quadCount++;
                    }
                }
            }
        }
    }

    generateQuadIndices(mesh, quadCount);
}
