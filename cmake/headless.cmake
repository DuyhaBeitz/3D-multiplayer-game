include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/shared.cmake)

add_executable(server
    src/World.cpp
    src/Game.cpp
    src/server.cpp
    src/Physics.cpp
)
target_link_libraries(server PUBLIC
    EasyNet
    fmt::fmt
    raylib
)

if(NOT raylib_SOURCE_DIR)
    FetchContent_GetProperties(raylib)
endif()

target_include_directories(server
    PRIVATE
        src
        ${r3d_SOURCE_DIR}/include
        ${raylib_SOURCE_DIR}/include
)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
endif()