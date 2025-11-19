include(FetchContent)

FetchContent_Declare(
	GLM
	GIT_REPOSITORY https://github.com/g-truc/glm
	GIT_TAG 1.0.2
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
)

FetchContent_Declare(
	GLFW
	GIT_REPOSITORY https://github.com/glfw/glfw
	GIT_TAG 3.4
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw
)

FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG v3.15.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.92.4-docking
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(
	GLFW
	GLM
    EnTT
	ImGui
)

add_library(ImGui_GLFW STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
target_include_directories(ImGui_GLFW PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(ImGui_GLFW
    glfw
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad)

add_subdirectory(src ${CMAKE_CURRENT_BINARY_DIR}/bin)
