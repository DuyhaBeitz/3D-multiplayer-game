include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/shared.cmake)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(server
    src/World.cpp
    src/Game.cpp
    src/server.cpp
    src/Physics.cpp
    src/GameMetadata.cpp
    src/StaticWorld.cpp
    src/SpacePartition.cpp
    src/SpaceActorPartitioner.cpp
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