#pragma once

#include "Util.hpp"
#include "VoxelGrid.hpp"

#include <glm/glm.hpp>

namespace Common {

// struct Chunk {
//     constexpr static auto SIZE = WorldConfig::CHUNK_SIZE;
//     using VoxelGrid = VoxelGrid<SIZE>;
    
//     glm::ivec3 position;
//     VoxelGrid grid;

//     constexpr auto box() const -> Box3 {
//         return {
//             toWorldPosition(position, {0, 0, 0}),
//             toWorldPosition(position, {SIZE, SIZE, SIZE})
//         };
//     }
// };

}
