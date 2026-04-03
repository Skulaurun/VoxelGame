add_library(VoxelEngine
    Application.hpp
    Application.cpp
)

target_include_directories(VoxelEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(VoxelEngine
    glm
    glfw
    stb
    Glad
    EnTT::EnTT
    ImGui_GLFW
)
