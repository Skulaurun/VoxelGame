add_executable(VoxelGame
    "Client/Client.cpp"
    "Client/ChunkMesher.cpp"
    "Client/ChunkRenderer.cpp"

    "Server/World/Voxel.cpp"
    "Server/World/Chunk.cpp"
    "Server/World/World.cpp"
    "Server/World/ChunkManager.cpp"
    "Server/World/WorldGenerator.cpp"

    "Main.cpp"
)

target_include_directories(VoxelGame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(VoxelGame
    VoxelEngine
)
