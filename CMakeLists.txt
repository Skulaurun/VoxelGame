cmake_minimum_required(VERSION 3.12.0)

project(VoxelGame)

SET(CMAKE_CXX_STANDARD 26)
SET(CMAKE_C_STANDARD 23)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (WIN32)
        add_compile_options(-fansi-escape-codes)
    endif()
    
    add_compile_options(-fcolor-diagnostics)
endif()

add_subdirectory(Engine)
add_subdirectory(Game)
