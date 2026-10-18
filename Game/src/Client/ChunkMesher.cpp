#include "ChunkMesher.hpp"

#include "Client/World/Cube.hpp"
#include "Client/World/World.hpp"

#include "Common/VoxelGrid.hpp"

#include <tracy/Tracy.hpp>

namespace Client {

ChunkMesher::ChunkMesher(World& world) :
    m_world{world},
    m_thread{std::bind_front(&ChunkMesher::doWork, this)}
{}

constexpr auto FULL_NEIGHBOR_OFFSETS = std::to_array<glm::ivec3>({
    {-1, 0, 0}, {0, -1, 0}, {0, 0, -1},
    {1, 0, 0}, {0, 1, 0}, {0, 0, 1},
    {-1, 1, 0}, {-1, 0, 1}, {-1, -1, 0},
    {-1, 0, -1}, {-1, 1, -1}, {-1, -1, 1},
    {-1, 1, 1}, {1, -1, 0}, {0, -1, 1},
    {0, -1, -1}, {1, -1, -1}, {1, -1, 1},
    {1, 1, -1}, {1, 0, -1}, {0, 1, -1},
    {-1, -1, -1}, {1, 1, 1}, {1, 1, 0},
    {1, 0, 1}, {0, 1, 1}
});

constexpr uint32_t REQUIRED_NEIGHBOR_COUNT{FULL_NEIGHBOR_OFFSETS.size()};

template<std::invocable<Chunk&> Fn>
static void forEachNeighbor(World& world, const glm::ivec3& position, Fn fn) {
    for (std::size_t i = 0; i < FULL_NEIGHBOR_OFFSETS.size(); i++) {
        auto* chunk = world.chunkAt(position + FULL_NEIGHBOR_OFFSETS[i]);
        if (chunk) {
            fn(*chunk);
        }
    }
}

void ChunkMesher::process(Chunk& chunk) {
    chunk.neighborCount = 0;
    forEachNeighbor(m_world, chunk.position, [this, &chunk](Chunk& neighbor) {
        chunk.neighborCount = std::min(chunk.neighborCount + 1u, REQUIRED_NEIGHBOR_COUNT);
        neighbor.neighborCount = std::min(neighbor.neighborCount + 1u, REQUIRED_NEIGHBOR_COUNT);

        if (neighbor.neighborCount == REQUIRED_NEIGHBOR_COUNT) {
            enqueue(neighbor);
        }
    });

    if (chunk.neighborCount == REQUIRED_NEIGHBOR_COUNT) {
        enqueue(chunk);
    }
}

void ChunkMesher::enqueue(const Chunk& chunk) {
    std::scoped_lock lock{m_mutex};

    auto task = std::make_unique<Task>(chunk.position);
    copyNeighborData(chunk, task->grid);
    m_queue.emplace_back(std::move(task));
    m_workAvailable.notify_one();
}

void ChunkMesher::doWork(std::stop_token token) {
    while (!token.stop_requested()) {
        std::unique_ptr<Task> task{};
        {
            std::unique_lock lock{m_mutex};
            m_workAvailable.wait(lock, [&]{ return !m_queue.empty() || token.stop_requested(); });
            if (token.stop_requested()) {
                return;
            }

            task = std::move(m_queue.front());
            m_queue.pop_front();
        }

        Mesh chunkMesh{};
        mesh(task->grid, task->position, chunkMesh);

        {
            std::scoped_lock lock{m_completedMutex};
            m_completedQueue.emplace_back(task->position, std::move(chunkMesh));
        }
    }
}

// // X
// int32_t x{}; // 0, temp_size - 1
// int32_t y{}; // i
// int32_t z{}; // j

// // Y
// int32_t x{}; // i
// int32_t y{}; // 0, temp_size - 1
// int32_t z{}; // j

// // Z
// int32_t x{}; // j
// int32_t y{}; // i
// int32_t z{}; // 0, temp_size - 1

template<int32_t Size>
constexpr auto ELEMENT_STRIDE = std::array{
    1, 1, Size,
    1, 1, Size
};

template<int32_t Size>
constexpr auto ROW_STRIDE = std::array{
    Size * Size, Size, Size * Size,
    Size * Size, Size, Size * Size
};

// FIXME: doesnt work (probably)
void ChunkMesher::copyNeighborData(const Chunk& chunk, VoxelGrid<TEMP_SIZE>& grid) {
    ZoneScoped;

    // FIXME: missing corners for ao
    constexpr auto NEIGHBOR_OFFSETS = std::to_array<glm::ivec3>({
        {-1, 0, 0}, // left
        {0, -1, 0}, // bottom
        {0, 0, -1}, // back
        {1, 0, 0},  // right
        {0, 1, 0},  // top
        {0, 0, 1}   // front
    });

    // This needs to be consistent with NEIGHBOR_OFFSETS
    constexpr auto FACE_OFFSETS = std::array{
        grid.indexOf({0, 1, 1}),
        grid.indexOf({1, 0, 1}),
        grid.indexOf({1, 1, 0}),

        grid.indexOf({TEMP_SIZE - 1, 1, 1}),
        grid.indexOf({1, TEMP_SIZE - 1, 1}),
        grid.indexOf({1, 1, TEMP_SIZE - 1})
    };

    for (int32_t n = 0; n < FACE_OFFSETS.size(); n++) {
        Chunk* neighbor = m_world.chunkAt(chunk.position + NEIGHBOR_OFFSETS[n]);
        if (!neighbor) continue;

        int32_t i1 = FACE_OFFSETS[n];
        int32_t i2 = 0;

        for (int32_t i = 0; i < Chunk::SIZE; i++) {
            for (int32_t j = 0; j < Chunk::SIZE; j++) {
                grid[i1] = neighbor->grid[i2];

                i1 += ELEMENT_STRIDE<TEMP_SIZE>[n];
                i2 += ELEMENT_STRIDE<Chunk::SIZE>[n];
            }
            i1 -= ELEMENT_STRIDE<TEMP_SIZE>[n] * Chunk::SIZE;
            i1 += ROW_STRIDE<TEMP_SIZE>[n];

            i2 -= ELEMENT_STRIDE<Chunk::SIZE>[n] * Chunk::SIZE;
            i2 += ROW_STRIDE<Chunk::SIZE>[n];
        }
    }

    for (int32_t y = 0; y < Chunk::SIZE; y++) {
        for (int32_t x = 0; x < Chunk::SIZE; x++) {
            for (int32_t z = 0; z < Chunk::SIZE; z++) {
                grid[grid.indexOf({x + 1, y + 1, z + 1})] = chunk.grid[chunk.grid.indexOf({x, y, z})];
            }
        }
    }
}

void ChunkMesher::mesh(const VoxelGrid<TEMP_SIZE>& grid, const glm::ivec3& chunkPosition, Mesh& mesh) {
    ZoneScoped;

    mesh.indices.clear();
    mesh.vertices.clear();

    auto isAir = [](const Voxel* voxel) {
        return static_cast<BlockType>(*voxel) == BlockType::Void;
    };

    std::size_t quadCount = 0;
    for (int32_t y = 1; y < Chunk::SIZE + 1; y++) {
        for (int32_t x = 1; x < Chunk::SIZE + 1; x++) {
            for (int32_t z = 1; z < Chunk::SIZE + 1; z++) {
                const Voxel* voxel = grid.at({x, y, z});
                if (isAir(voxel)) {
                    continue;
                }

                auto neighbors = std::array{
                    grid.at({x - 1, y, z}), // left
                    grid.at({x + 1, y, z}), // right
                    grid.at({x, y, z + 1}), // front
                    grid.at({x, y, z - 1}), // back
                    grid.at({x, y + 1, z}), // top
                    grid.at({x, y - 1, z})  // bottom
                };

                for (std::size_t i = 0; i < neighbors.size(); i++) {
                    if (!neighbors[i] || isAir(neighbors[i])) {
                        // Face is visible
                        for (std::size_t j = 0; j < 4; j++) {
                            glm::vec3 position = CUBE_FACES[i * 4 + j];
                            position += glm::ivec3(x - 1, y - 1, z - 1);
                            position += chunkPosition * Chunk::SIZE;
                            
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
                                !isAir(grid.at({x + corners[1].x, y + corners[1].y, z + corners[1].z})),
                                !isAir(grid.at({x + corners[2].x, y + corners[2].y, z + corners[2].z})),
                                !isAir(grid.at({x + corners[0].x, y + corners[0].y, z + corners[0].z}))
                            );

                            glm::vec4 color{ 0.5f, 0.5f, 0.5f, 1.0f };
                            if (static_cast<BlockType>(*voxel) == BlockType::Grass) {
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
    mesh.boundingBox = Chunk::box(chunkPosition);
}

}
