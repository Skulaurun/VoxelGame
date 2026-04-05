include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.17.0
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog
    OVERRIDE_FIND_PACKAGE
)

FetchContent_Declare(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy
    GIT_TAG v0.13.1
    GIT_SHALLOW TRUE
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tracy
)

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
	stb
	GIT_REPOSITORY https://github.com/nothings/stb
	GIT_TAG f1c79c02822848a9bed4315b12c8c8f3761e1296
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
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
    spdlog
    tracy
	GLFW
	GLM
    stb
    EnTT
	ImGui
)

add_library(stb STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image.cpp
)
target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})

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

add_subdirectory(src/Engine ${CMAKE_CURRENT_BINARY_DIR}/bin)
