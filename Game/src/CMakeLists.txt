add_executable(VoxelGame
    "Common/Network/Connection.cpp"

    "Client/Client.cpp"
    "Client/World/World.cpp"
    "Client/NetCode.cpp"
    "Client/ChunkMesher.cpp"
    "Client/ChunkRenderer.cpp"

    "Server/Server.cpp"
    "Server/NetCode.cpp"
    "Server/World/World.cpp"
    "Server/World/ChunkLoader.cpp"
    "Server/World/WorldGenerator.cpp"

    "Main.cpp"
)

target_include_directories(VoxelGame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(VoxelGame
    VoxelEngine
)

if (asio_POPULATED)
    target_include_directories(VoxelGame
        PUBLIC ${asio_SOURCE_DIR}/include
    )
endif()
