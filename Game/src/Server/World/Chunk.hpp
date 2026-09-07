#pragma once

#include "Server/World/Voxel.hpp"

#include <Engine/Mesh.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Core/Math.hpp>

#include <glm/glm.hpp>

#include <array>

class World;

class Chunk {

public:
    constexpr static int32_t SIZE = 16;
    constexpr static int32_t CUBIC_SIZE = SIZE * SIZE * SIZE;
    constexpr static uint32_t MAX_FACES = 6 * ((CUBIC_SIZE + 1) / 2);

    enum class State {
        Empty,
        Generated,
        ReadyToMesh,
        Meshed
    };

public:
    Chunk(World& world, glm::ivec3 position, State state);

    void generateMesh(glm::vec3 offset);

    void setDirty(bool dirty) { m_dirty = dirty; }

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

        return &m_voxels[i];
    }

    auto box() const -> Box3 {
        return {
            toWorldPosition({0, 0, 0}),
            toWorldPosition({SIZE, SIZE, SIZE})
        };
    }

    auto toWorldPosition(glm::ivec3 voxelPosition) const -> glm::ivec3;

    void setState(State state) { m_state.store(state); }

    auto state() const { return m_state.load(); }
    auto isDirty() const { return m_dirty; }
    auto position() const { return m_position; }
    auto mesh() const -> const auto& { return m_mesh; }

private:
    World& m_world;

    std::atomic<State> m_state;

    glm::ivec3 m_position;
    std::array<Voxel, CUBIC_SIZE> m_voxels;
    Mesh m_mesh;
    bool m_dirty;

};
