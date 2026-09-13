#pragma once

#include "Client/World/Chunk.hpp"

#include <glm/glm.hpp>

class ChunkMesher {

public:
    ChunkMesher() = default;

    void generate(Chunk& chunk, glm::vec3 offset);

private:
    //World& m_world;
    
};
