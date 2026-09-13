#include "ChunkMesher.hpp"

#include "World/Cube.hpp"

#include "Common/WorldConfig.hpp"
#include "Common/VoxelGrid.hpp"

#include <tracy/Tracy.hpp>

// ChunkCache:
//     addChunks(): mesher -> check all unmeshed chunks if can mesh

void ChunkMesher::generate(Chunk& chunk, glm::vec3 offset) {
    ZoneScoped;

    constexpr auto SIZE = WorldConfig::CHUNK_SIZE;
    constexpr auto TEMP_SIZE = SIZE + 2;

    // NOTE: Static only for performance test before
    static VoxelGrid<TEMP_SIZE> temporary{};

    {
    ZoneScopedN("QNeigh & Copy");

    // TODO: Do axes matter? (switching + and - didn't do a thing?)
    std::array<Chunk*, 7> chunks = {
        // left
        m_world.getChunkAt(chunk.position + glm::ivec3(-1, 0, 0)),
        // right
        m_world.getChunkAt(chunk.position + glm::ivec3(1, 0, 0)),
        // front
        m_world.getChunkAt(chunk.position + glm::ivec3(0, 0, 1)),
        // back
        m_world.getChunkAt(chunk.position + glm::ivec3(0, 0, -1)),
        // top
        m_world.getChunkAt(chunk.position + glm::ivec3(0, 1, 0)),
        // bottom
        m_world.getChunkAt(chunk.position + glm::ivec3(0, -1, 0))
    };

    // X Axis
    if (chunks[0]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1)] = *chunks[0]->grid.at({ SIZE - 1, i, j });
            }
        }
    }
    if (chunks[1]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (TEMP_SIZE - 1) * TEMP_SIZE + (j + 1)] = *chunks[1]->grid.at({ 0, i, j });
            }
        }
    }

    // Z Axis
    if (chunks[2]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1) * TEMP_SIZE + (TEMP_SIZE - 1)] = *chunks[2]->grid.at({ j, i, 0 });
            }
        }
    }
    if (chunks[3]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1) * TEMP_SIZE] = *chunks[3]->grid.at({ j, i, SIZE - 1 });
            }
        }
    }

    // Y Axis
    if (chunks[4]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(TEMP_SIZE - 1) * TEMP_SIZE * TEMP_SIZE + (i + 1) * TEMP_SIZE + (j + 1)] = *chunks[4]->grid.at({ i, 0, j });
            }
        }
    }
    if (chunks[5]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE + (j + 1)] = *chunks[5]->grid.at({ i, SIZE - 1, j });
            }
        }
    }

    for (std::size_t i = 0; i < SIZE; i++) {
        for (std::size_t j = 0; j < SIZE; j++) {
            for (std::size_t k = 0; k < SIZE; k++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1) * TEMP_SIZE + (k + 1)] = chunk.grid[i * SIZE * SIZE + j * SIZE + k];
            }
        }
    }
    }

    {
    ZoneScopedN("Meshgen");

    Mesh& mesh = chunk.mesh;

    mesh.indices.clear();
    mesh.vertices.clear();

    auto isAir = [](Voxel* voxel) {
        return voxel->type == BlockType::Void;
    };

    std::size_t quadCount = 0;
    for (int32_t y = 1; y < SIZE + 1; y++) {
        for (int32_t x = 1; x < SIZE + 1; x++) {
            for (int32_t z = 1; z < SIZE + 1; z++) {
                Voxel* voxel = temporary.at({x, y, z});
                if (isAir(voxel)) {
                    continue;
                }

                std::array<Voxel*, 6> neighbors = {
                    temporary.at({x - 1, y, z}), // left
                    temporary.at({x + 1, y, z}), // right
                    temporary.at({x, y, z + 1}), // front
                    temporary.at({x, y, z - 1}), // back
                    temporary.at({x, y + 1, z}), // top
                    temporary.at({x, y - 1, z})  // bottom
                };

                for (std::size_t i = 0; i < neighbors.size(); i++) {
                    if (!neighbors[i] || isAir(neighbors[i])) {
                        // Face is visible
                        for (std::size_t j = 0; j < 4; j++) {
                            glm::vec3 position = CUBE_FACES[i * 4 + j];
                            position += glm::ivec3(x - 1, y - 1, z - 1);
                            position += offset;
                            
                            glm::vec3 signs = glm::sign(CUBE_FACES[i * 4 + j] - 0.5f);
                            glm::vec3 normal = CUBE_VERTEX_NORMALS[i * 4 + j];

                            auto corners = std::to_array<glm::vec3>({
                                {signs.x, signs.y, signs.z},
                                
                                {0.0f, signs.y, signs.z},
                                {signs.x, 0.0f, signs.z},
                                {signs.x, signs.y, 0.0f}
                            });

                            for (std::size_t i = 1; i <= 2; i++) {
                                if (normal * corners[i] == glm::vec3{0.0f, 0.0f, 0.0f}) {
                                    corners[i] = corners.back();
                                    break;
                                }
                            }

                            auto computeAo = [](int32_t side1, int32_t side2, int32_t corner) -> float {
                                if (side1 && side2) {
                                    return 0;
                                }

                                return static_cast<float>(3 - (side1 + side2 + corner)) / 3.0f;
                            };

                            // TODO: Assert
                            float ao = computeAo(
                                !isAir(temporary.at({x + corners[1].x, y + corners[1].y, z + corners[1].z})),
                                !isAir(temporary.at({x + corners[2].x, y + corners[2].y, z + corners[2].z})),
                                !isAir(temporary.at({x + corners[0].x, y + corners[0].y, z + corners[0].z}))
                            );

                            glm::vec4 color{ 0.5f, 0.5f, 0.5f, 1.0f };
                            if (voxel->type == BlockType::Grass) {
                                color = { 0.2f, 0.85f, 0.25f, 1.0f };
                            }

                            mesh.vertices.emplace_back(Vertex{ position, color, glm::vec2{}, 0.0f, ao });
                        }
                        quadCount++;
                    }
                }
            }
        }
    }

    generateQuadIndices(mesh, quadCount);

    mesh.boundingBox = chunk.box();

    }
}

// {
// ZoneScopedN("Process loaded chunks");

// const auto& loadedChunks = chunkManager.loadedChunks();

// for (const auto& chunkEvent : chunkManager.chunkEvents()) {
//     if (chunkEvent.type == ChunkManager::Event::Type::Load) {
//         Chunk* chunk = loadedChunks[chunkEvent.id].chunk;
//         if (chunk->mesh().indices.size() == 0) {
//             chunk->generateMesh(chunk->position() * Chunk::SIZE);
//         }
//         m_chunkRenderer.uploadMesh(chunk->mesh(), chunkEvent.id);
//     }
//     if (chunkEvent.type == ChunkManager::Event::Type::Unload) {
//         m_chunkRenderer.setEnabled(chunkEvent.id, false);
//     }
// }

// for (const auto& chunkRef : loadedChunks) {
//     if (chunkRef.isOccupied() && chunkRef.chunk->isDirty()) {
//         // Update chunk
//         Chunk* chunk = chunkRef.chunk;
//         chunk->generateMesh(chunk->position() * Chunk::SIZE);
//         chunk->setDirty(false);

//         m_chunkRenderer.uploadMesh(chunk->mesh(), chunkRef.index);
//     }
// }
// }
