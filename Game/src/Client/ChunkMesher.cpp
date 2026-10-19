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

constexpr auto NEIGHBOR_OFFSETS = std::to_array<glm::ivec3>({
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

constexpr uint32_t REQUIRED_NEIGHBOR_COUNT{NEIGHBOR_OFFSETS.size()};

struct CopyPattern {
    glm::ivec3 counts;
    int32_t srcOffset;
    int32_t dstOffset;
};

template<int32_t SrcChunkSize, int32_t DstChunkSize, std::size_t NeighborCount>
    requires(DstChunkSize - SrcChunkSize == 2)
constexpr auto createCopyPatterns(const std::array<glm::ivec3, NeighborCount>& offsets) {
    auto src = [](int32_t offset) constexpr {
        return offset < 0 ? (SrcChunkSize - 1) : 0;
    };
    auto dst = [](int32_t offset) constexpr {
        constexpr auto PADDING{1};
        return (offset < 0) ? 0 : (offset > 0 ? DstChunkSize - 1 : PADDING);
    };

    std::array<CopyPattern, NeighborCount> patterns{};
    for (std::size_t i = 0; i < NeighborCount; i++) {
        const auto& offset = offsets[i];
        patterns[i] = {
            {
                offset.x == 0 ? SrcChunkSize : 1,
                offset.y == 0 ? SrcChunkSize : 1,
                offset.z == 0 ? SrcChunkSize : 1
            },
            VoxelGrid<SrcChunkSize>::indexOf({
                src(offset.x), src(offset.y), src(offset.z)
            }),
            VoxelGrid<DstChunkSize>::indexOf({
                dst(offset.x), dst(offset.y), dst(offset.z)
            })
        };
    }
    return patterns;
}

template<std::invocable<Chunk&> Fn>
static void forEachNeighbor(World& world, const glm::ivec3& position, Fn fn) {
    for (std::size_t i = 0; i < NEIGHBOR_OFFSETS.size(); i++) {
        auto* chunk = world.chunkAt(position + NEIGHBOR_OFFSETS[i]);
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

void ChunkMesher::copyNeighborData(const Chunk& chunk, VoxelGrid<TEMP_SIZE>& grid) {
    ZoneScoped;

    // TODO: ao (26 neighborhood might not be needed)
    constexpr auto NEIGHBORHOOD_6 = std::to_array<glm::ivec3>({
        {-1, 0, 0},
        {1, 0, 0},
        {0, 0, 1},
        {0, 0, -1},
        {0, 1, 0},
        {0, -1, 0}
    });

    constexpr auto COPY_PATTERNS = createCopyPatterns<Chunk::SIZE, TEMP_SIZE>(NEIGHBORHOOD_6);

    for (int32_t i = 0; i < NEIGHBORHOOD_6.size(); i++) {
        Chunk* neighbor = m_world.chunkAt(chunk.position + NEIGHBORHOOD_6[i]);
        if (!neighbor) continue;

        for (int32_t y = 0; y < COPY_PATTERNS[i].counts.y; y++) {
            for (int32_t x = 0; x < COPY_PATTERNS[i].counts.x; x++) {
                for (int32_t z = 0; z < COPY_PATTERNS[i].counts.z; z++) {
                    grid[COPY_PATTERNS[i].dstOffset + grid.indexOf({x, y, z})]
                        = neighbor->grid[COPY_PATTERNS[i].srcOffset + neighbor->grid.indexOf({x, y, z})];
                }
            }
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
