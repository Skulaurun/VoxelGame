#include <Engine/World/Chunk.hpp>
#include <Engine/World/World.hpp>
#include <Engine/World/Noise.hpp>

#include <tracy/Tracy.hpp>

Chunk::Chunk(World& world, glm::ivec3 position)
    : world{world}, position{position}, m_dirty{false} {}

glm::ivec3 Chunk::toWorldPosition(glm::ivec3 voxelPosition) {
    return world.toWorldPosition(position, voxelPosition);
}

void Chunk::generateMesh(glm::vec3 offset) {
    ZoneScoped;

    constexpr auto TEMP_SIZE = SIZE + 2;

    // NOTE: Static only for performance test before 
    static std::array<Voxel, TEMP_SIZE * TEMP_SIZE * TEMP_SIZE> temporary{};

    auto tempAt = [&](int x, int y, int z) -> Voxel* {
        // TODO: No check needed for temp buffer
        if (y >= TEMP_SIZE || y < 0) {
            return nullptr;
        }
        if (x >= TEMP_SIZE || x < 0) {
            return nullptr;
        }
        if (z >= TEMP_SIZE || z < 0) {
            return nullptr;
        }
        return &temporary[y * TEMP_SIZE * TEMP_SIZE + x * TEMP_SIZE + z];
    };

    {
    ZoneScopedN("QNeigh & Copy");

    temporary.fill(Voxel{});

    // TODO: Do axes matter? (switching + and - didn't do a thing?)
    std::array<Chunk*, 7> chunks = {
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
        world.getChunkAt(position + glm::ivec3(0, -1, 0))
    };

    // X Axis
    if (chunks[0]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1)] = *chunks[0]->at({ SIZE - 1, i, j });
            }
        }
    }
    if (chunks[1]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (TEMP_SIZE - 1) * TEMP_SIZE + (j + 1)] = *chunks[1]->at({ 0, i, j });
            }
        }
    }

    // Z Axis
    if (chunks[2]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1) * TEMP_SIZE + (TEMP_SIZE - 1)] = *chunks[2]->at({ j, i, 0 });
            }
        }
    }
    if (chunks[3]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1) * TEMP_SIZE] = *chunks[3]->at({ j, i, SIZE - 1 });
            }
        }
    }

    // Y Axis
    if (chunks[4]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(TEMP_SIZE - 1) * TEMP_SIZE * TEMP_SIZE + (i + 1) * TEMP_SIZE + (j + 1)] = *chunks[4]->at({ i, 0, j });
            }
        }
    }
    if (chunks[5]) {
        for (std::size_t i = 0; i < SIZE; i++) {
            for (std::size_t j = 0; j < SIZE; j++) {
                temporary[(i + 1) * TEMP_SIZE + (j + 1)] = *chunks[5]->at({ i, SIZE - 1, j });
            }
        }
    }

    for (std::size_t i = 0; i < SIZE; i++) {
        for (std::size_t j = 0; j < SIZE; j++) {
            for (std::size_t k = 0; k < SIZE; k++) {
                temporary[(i + 1) * TEMP_SIZE * TEMP_SIZE + (j + 1) * TEMP_SIZE + (k + 1)] = voxels[i * SIZE * SIZE + j * SIZE + k];
            }
        }
    }
    }

    {
    ZoneScopedN("Meshgen");

    mesh.indices.clear();
    mesh.vertices.clear();

    constexpr std::array<glm::vec4, 4> COLORS{{
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f, 1.0f }
    }};

    std::size_t quadCount = 0;
    for (int32_t y = 1; y < SIZE + 1; y++) {
        for (int32_t x = 1; x < SIZE + 1; x++) {
            for (int32_t z = 1; z < SIZE + 1; z++) {
                Voxel* voxel = tempAt(x, y, z);
                if (voxel->isTransparent()) {
                    continue;
                }

                std::array<Voxel*, 6> neighbors = {
                    tempAt(x - 1, y, z), // left
                    tempAt(x + 1, y, z), // right
                    tempAt(x, y, z + 1), // front
                    tempAt(x, y, z - 1), // back
                    tempAt(x, y + 1, z), // top
                    tempAt(x, y - 1, z)  // bottom
                };

                for (std::size_t i = 0; i < neighbors.size(); i++) {
                    if (!neighbors[i] || neighbors[i]->isTransparent()) {
                        // Face is visible
                        for (std::size_t j = 0; j < 4; j++) {
                            glm::vec3 position = Voxel::FACES[i * 4 + j];
                            position += glm::ivec3(x - 1, y - 1, z - 1);
                            position += offset;
                            
                            glm::vec3 signs = glm::sign(Voxel::FACES[i * 4 + j]);
                            glm::vec3 normal = Voxel::VERTEX_NORMALS[i * 4 + j];

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
                                !tempAt(x + corners[1].x, y + corners[1].y, z + corners[1].z)->isTransparent(),
                                !tempAt(x + corners[2].x, y + corners[2].y, z + corners[2].z)->isTransparent(),
                                !tempAt(x + corners[0].x, y + corners[0].y, z + corners[0].z)->isTransparent()
                            );

                            mesh.vertices.emplace_back(Vertex{ position, { 1.0f, 1.0f, 1.0f, 1.0f }, glm::vec2{}, 0.0f, ao });
                        }
                        quadCount++;
                    }
                }
            }
        }
    }

    generateQuadIndices(mesh, quadCount);

    }
}
