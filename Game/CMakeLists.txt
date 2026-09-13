include(FetchContent)

FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio
    GIT_TAG asio-1-38-2
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/asio
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(
    asio
)

add_subdirectory(src ${CMAKE_CURRENT_BINARY_DIR}/bin)
