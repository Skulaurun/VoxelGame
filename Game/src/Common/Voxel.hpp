#pragma once

enum class BlockType {
    Void,
    Grass,
    Stone
};

struct Voxel {
    BlockType type;
};
