#pragma once

#include <Engine/Mesh.hpp>
#include <Engine/World/Voxel.hpp>

#include <Engine/Renderer/Renderer.hpp>

#include <glm/glm.hpp>

#include <array>

class World;

class Chunk {

public:
    constexpr static int32_t SIZE = 16;
    constexpr static int32_t CUBIC_SIZE = SIZE * SIZE * SIZE;
    constexpr static uint32_t MAX_FACES = 6 * ((CUBIC_SIZE + 1) / 2);

public:
    Chunk(World& world, glm::ivec3 position);

    void unload();

    Voxel* at(glm::ivec3 localPosition) {
        if (localPosition.y >= SIZE || localPosition.y < 0) {
            return nullptr;
        }
        if (localPosition.x >= SIZE || localPosition.x < 0) {
            return nullptr;
        }
        if (localPosition.z >= SIZE || localPosition.z < 0) {
            return nullptr;
        }
        int32_t i = localPosition.y * SIZE * SIZE
            + localPosition.x * SIZE
            + localPosition.z;

        return &voxels[i];
    }

    glm::ivec3 toWorldPosition(glm::ivec3 voxelPosition);

    void generateMesh(glm::vec3 offset);

    glm::ivec3 getPosition() { return position; }
    const Mesh& getMesh() const { return mesh; }

private:
    World& world;

    glm::ivec3 position;
    std::array<Voxel, CUBIC_SIZE> voxels;
    Mesh mesh;

};
