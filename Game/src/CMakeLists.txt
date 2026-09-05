add_executable(VoxelGame
    "Client/Client.cpp"

    "Main.cpp"
)

target_link_libraries(VoxelGame
    VoxelEngine
)
