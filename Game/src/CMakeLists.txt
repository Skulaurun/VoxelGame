add_library(VoxelCommon STATIC
    "Common/Network/Connection.cpp"
)

if (asio_POPULATED)
    target_include_directories(VoxelCommon
        PUBLIC ${asio_SOURCE_DIR}/include
    )
endif()

target_link_libraries(VoxelCommon PUBLIC VoxelEngine)
target_include_directories(VoxelCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(VoxelServer STATIC
    "Server/Server.cpp"
    "Server/NetCode.cpp"
    "Server/World/World.cpp"
    "Server/World/ClientChunkView.cpp"
    "Server/World/WorldGenerator.cpp"
)
target_link_libraries(VoxelServer PUBLIC VoxelCommon)

add_executable(VoxelGame
    "Client/Client.cpp"
    "Client/World/World.cpp"
    "Client/NetCode.cpp"
    "Client/ChunkMesher.cpp"
    "Client/ChunkRenderer.cpp"

    "Main.cpp"
)
target_link_libraries(VoxelGame PUBLIC VoxelServer)

add_executable(VoxelGameDedicatedServer
    "Server/Main.cpp"
)
target_link_libraries(VoxelGameDedicatedServer PUBLIC VoxelServer)
