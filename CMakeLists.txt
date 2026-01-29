cmake_minimum_required(VERSION 3.20)

project(Multiplayer LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build all dependencies as static libraries" FORCE)
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")
set(FETCHCONTENT_QUIET OFF)
#set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(CMAKE_BUILD_TYPE Release)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        12.1.0
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    EasyNet
    GIT_REPOSITORY https://github.com/DuyhaBeitz/EasyNet.git
    GIT_TAG main
    UPDATE_COMMAND ""
)
FetchContent_MakeAvailable(EasyNet)

set(R3D_ASSIMP_VENDORED ON CACHE BOOL "" FORCE)
set(R3D_RAYLIB_VENDORED ON CACHE BOOL "" FORCE)
add_subdirectory(external/r3d)


FetchContent_Declare(
    RaylibRetainedGUI
    GIT_REPOSITORY https://github.com/DuyhaBeitz/RaylibRetainedGUI.git
    GIT_TAG master
    UPDATE_COMMAND ""
    GIT_TAG 0.0.1
)
FetchContent_MakeAvailable(RaylibRetainedGUI)

include_directories(
    src
    "${CMAKE_SOURCE_DIR}/external/r3d/include"
)


add_executable(server
    src/World.cpp
    src/Game.cpp
    src/server.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
)
target_link_libraries(server PUBLIC
    EasyNet
    fmt::fmt
    raylib
    r3d
)

add_executable(client 
    src/World.cpp
    src/Game.cpp
    src/client.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
)
target_link_libraries(client PUBLIC
    EasyNet
    fmt::fmt
    raylib
    RaylibRetainedGUI
    r3d
)

add_executable(standalone 
    src/World.cpp
    src/Game.cpp
    src/standalone.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
)
target_link_libraries(standalone PUBLIC
    raylib
    RaylibRetainedGUI
    r3d
)

add_executable(test
    src/World.cpp
    src/Game.cpp
    src/test.cpp
    src/Physics.cpp
    src/Resources.cpp
    src/Rendering.cpp
)
target_link_libraries(test PUBLIC
    raylib
    RaylibRetainedGUI
    r3d
)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32)
    target_link_libraries(client PRIVATE ws2_32)
    target_link_libraries(standalone PRIVATE ws2_32)
    target_link_libraries(test PRIVATE ws2_32)
endif()