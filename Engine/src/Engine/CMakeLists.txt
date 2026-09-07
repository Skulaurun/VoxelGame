add_library(VoxelEngine
    "Core/Log.cpp"
    "Core/Window.cpp"

    "Renderer/OpenGL/OpenGL.cpp"
    "Renderer/OpenGL/Shader.cpp"
    "Renderer/OpenGL/Texture.cpp"
    "Renderer/OpenGL/VertexBuffer.cpp"
    "Renderer/OpenGL/IndexBuffer.cpp"
    "Renderer/OpenGL/VertexArray.cpp"
    "Renderer/PerspectiveCamera.cpp"
    "Renderer/Frustum.cpp"
    "Renderer/Renderer.cpp"
)

target_include_directories(VoxelEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(VoxelEngine
    Tracy::TracyClient
    spdlog::spdlog
    glm
    glfw
    stb
    Glad
    EnTT::EnTT
    ImGui_GLFW
)
